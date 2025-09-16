#include "numc.h"

#include <stddef.h>
#include <structmember.h>

PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject *mat = args;
     if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    Matrix61c* mat61c = (Matrix61c*)mat;

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* result = NULL;
    int flag = allocate_matrix(&result, self->mat->rows, self->mat->cols);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }

    flag = add_matrix(result, self->mat, mat61c->mat);
    if (flag) {
        PyErr_SetString(PyExc_ValueError, "Operands do not have the same dimensions!");
        return NULL;
    }
    rv->mat = result;
    rv->shape = self->shape;
    return (PyObject *) rv;

}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject *mat = args;

    if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    Matrix61c* mat61c = (Matrix61c*)mat;

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* result = NULL;
    int flag = allocate_matrix(&result, self->mat->rows, self->mat->cols);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }

    flag = sub_matrix(result, self->mat, mat61c->mat);
    if (flag) {
        PyErr_SetString(PyExc_ValueError, "Operands do not have the same dimensions!");
        return NULL;
    }
    rv->mat = result;
    rv->shape = self->shape;
    return (PyObject *) rv;


}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    /* TODO: YOUR CODE HERE */
    PyObject *mat = args;

    if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    Matrix61c* mat61c = (Matrix61c*)mat;

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* result = NULL;
    int flag = allocate_matrix(&result, self->mat->rows, mat61c->mat->cols);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }

    flag = mul_matrix(result, self->mat, mat61c->mat);
    if (flag) {
        PyErr_SetString(PyExc_ValueError, "Dimension Mismatch!");
        return NULL;
    }
    rv->mat = result;
    rv->shape = get_shape(self->mat->rows, mat61c->mat->cols);
    return (PyObject *) rv;

}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    /* TODO: YOUR CODE HERE */
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* result = NULL;
    int flag = allocate_matrix(&result, self->mat->rows, self->mat->cols);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }

    flag = neg_matrix(result, self->mat);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Operands do not have the same dimensions!");
        return NULL;
    }
    rv->mat = result;
    rv->shape = self->shape;
    return (PyObject *) rv;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    /* TODO: YOUR CODE HERE */
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* result = NULL;
    int flag = allocate_matrix(&result, self->mat->rows, self->mat->cols);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }

    flag = abs_matrix(result, self->mat);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Operands do not have the same dimensions!");
        return NULL;
    }
    rv->mat = result;
    rv->shape = self->shape;
    return (PyObject *) rv;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    /* TODO: YOUR CODE HERE */
    if (!PyLong_Check(pow)) {
        PyErr_SetString(PyExc_TypeError, "Invalid value for argument pow");
        return NULL;
    }
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* result = NULL;
    int flag = allocate_matrix(&result, self->mat->rows, self->mat->cols);
    if (flag) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }

    int pow_num = (int) PyLong_AsLong(pow);

    flag = pow_matrix(result, self->mat, pow_num);
    if (flag) {
        if (flag == -3) {
            PyErr_SetString(PyExc_ValueError, "Pow is negative!");
            return NULL;
        }
        if (flag == -2) {
            PyErr_SetString(PyExc_ValueError, "Not a square matrix!");
            return NULL;
        }

    }
    rv->mat = result;
    rv->shape = self->shape;
    return (PyObject *) rv;
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    /* TODO: YOUR CODE HERE */
    (binaryfunc) Matrix61c_add,
    (binaryfunc) Matrix61c_sub,
    (binaryfunc) Matrix61c_multiply,
    NULL, NULL,
    (ternaryfunc) Matrix61c_pow,
    (unaryfunc) Matrix61c_neg,
    NULL,
    (unaryfunc) Matrix61c_abs,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject *row = NULL;
    PyObject *col = NULL;
    PyObject *val = NULL;
    if (PyArg_UnpackTuple(args, "args", 3, 3, &row, &col, &val)) {
        if (!PyLong_Check(row) || !PyLong_Check(col) || (!PyLong_Check(val) && !PyFloat_Check(val))) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument!");
            return NULL;
        }
        int i = (int) PyLong_AsLong(row);
        int j = (int) PyLong_AsLong(col);
        double val_c = (double) PyFloat_AsDouble(val);
        if (i < 0 || i >= self->mat->rows || j < 0 || j >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "Index out of range!");
            return NULL;
        }

        set(self->mat, i, j, val_c);
        return Py_None;

    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }

}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject *row = NULL;
    PyObject *col = NULL;
    if (PyArg_UnpackTuple(args, "args", 2, 2, &row, &col)) {
        if (!PyLong_Check(row) || !PyLong_Check(col)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument!");
            return NULL;
        }
        int i = (int) PyLong_AsLong(row);
        int j = (int) PyLong_AsLong(col);
        if (i < 0 || i >= self->mat->rows || j < 0 || j >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "Index out of range!");
            return NULL;
        }
        return PyFloat_FromDouble(get(self->mat, i, j));

    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }

}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    /* TODO: YOUR CODE HERE */
    {"set", (PyCFunction) Matrix61c_set_value, METH_VARARGS, "set value at the `row`th row and `col`th column"},
    {"get", (PyCFunction) Matrix61c_get_value, METH_VARARGS, "get value at the `row`th row and `col`th column"},
    {NULL, NULL, 0, NULL} // sentinel indicating the end of the method table
};

/* INDEXING */

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    /* TODO: YOUR CODE HERE */
    if (self->mat->is_1d) { // 1D matrix
        if (!PyLong_Check(key) && !PySlice_Check(key)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument!");
            return NULL;
        }

        int total_size = self->mat->rows * self->mat->cols;
        if (PyLong_Check(key)) { // if key is a int
            int index = (int) PyLong_AsLong(key);
            if (index < 0 || index >= total_size) {
                PyErr_SetString(PyExc_IndexError, "Index out of bounds!");
                return NULL;
            }
            int row = index / self->mat->cols;
            int col = index % self->mat->cols;
            return PyFloat_FromDouble(self->mat->data[row][col]);
        }

        // if key is a slice
        Py_ssize_t start = 0, stop = 0, step = 0, slicelength = 0;
        if (getFromSlice(key, total_size, &start, &stop,
            &step, &slicelength)) {
            return NULL;
        }

        if (slicelength == 1) { // 1 by 1 slice
            if (self->mat->rows == 1) {
                return PyFloat_FromDouble(self->mat->data[0][start]);
            }
            return PyFloat_FromDouble(self->mat->data[start][0]);
        }

        if (self->mat->rows == 1) {
            return creatMatrixFromSlice(self, 0, start, 1, slicelength);
        }
        return creatMatrixFromSlice(self, start, 0, slicelength, 1);
    }
    //2-D matrix
    if (!PyLong_Check(key) && !PySlice_Check(key)
        && !(PyTuple_Check(key) && PyTuple_Size(key) == 2)) {
        PyErr_SetString(PyExc_TypeError, "Invalid value for argument!");
        return NULL;
    }

    if (PyLong_Check(key)) { // Integer index for row
        int row = (int) PyLong_AsLong(key);
        if (row < 0 || row >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of bounds!");
            return NULL;
        }
        return creatMatrixFromSlice(self, row, 0, 1, self->mat->cols);
    }

    if (PySlice_Check(key)) { // Slice index for rows
        Py_ssize_t start = 0, stop = 0, step = 0, slicelength = 0;
        if (getFromSlice(key, self->mat->rows, &start, &stop,
                &step, &slicelength)) {
            return NULL;
        }
        return creatMatrixFromSlice(self, start, 0, slicelength, self->mat->cols);
    }

    // Tuple index
    PyObject *row_key = PyTuple_GetItem(key, 0);
    PyObject *col_key = PyTuple_GetItem(key, 1);
    if (!(PyLong_Check(row_key) || PySlice_Check(row_key)) ||
        !(PyLong_Check(col_key) || PySlice_Check(col_key))) {
        PyErr_SetString(PyExc_TypeError, "Tuple elements must be integers or slices");
        return NULL;
    }

    // Handle row index
    Py_ssize_t row_start = 0, row_stop = 0, row_step = 0, row_slicelength = 0;
    int row_is_int = 0;
    int row_index = 0;
    if (PyLong_Check(row_key)) {
        row_is_int = 1;
        row_index = (int) PyLong_AsLong(row_key);
        if (row_index < 0 || row_index >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Row index out of bounds");
            return NULL;
        }
    } else {
        if (getFromSlice(row_key, self->mat->rows, &row_start, &row_stop,
            &row_step, &row_slicelength)) {
            return NULL;
        }
    }
    // Handle col index
    Py_ssize_t col_start = 0, col_stop = 0, col_step = 0, col_slicelength = 0;
    int col_is_int = 0;
    int col_index = 0;
    if (PyLong_Check(col_key)) {
        col_is_int = 1;
        col_index = (int) PyLong_AsLong(col_key);
        if (col_index < 0 || col_index >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "Row index out of bounds");
            return NULL;
        }
    } else {
        if (getFromSlice(col_key, self->mat->cols, &col_start, &col_stop,
            &col_step, &col_slicelength)) {
            return NULL;
        }
    }

    if (row_is_int == 1 && col_is_int == 1) { // the slice is 1 by 1
        return PyFloat_FromDouble(self->mat->data[row_index][col_index]);
    }
    if (row_slicelength == 1 && col_slicelength == 1) {
        return PyFloat_FromDouble(self->mat->data[row_start][col_start]);
    }

    int row_offset = row_is_int == 1 ? row_index : row_start;
    int col_offset = col_is_int == 1 ? col_index : col_start;
    int row = row_is_int == 1 ? 1 : row_slicelength;
    int col = col_is_int == 1 ? 1 : col_slicelength;
    return creatMatrixFromSlice(self, row_offset, col_offset,row, col);
}

/*
 * Helper function to generalize the matrix creation from given ref matrix.
 */
PyObject* creatMatrixFromSlice(Matrix61c *self, int row_offset, int col_offset,
    int rows, int cols) {
    matrix *new_mat = NULL;
    int alloc = allocate_matrix_ref(&new_mat, self->mat, row_offset, col_offset,
        rows, cols);
    if (alloc) {
        PyErr_SetString(PyExc_RuntimeError, "Allocation failed!");
        return NULL;
    }
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    rv->mat = new_mat;
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
    return (PyObject *)rv;
}

/*
 * Helper function to find the start, stop, step and slicelength form given slice.
 */
int getFromSlice(PyObject* key, int total_size, Py_ssize_t* start, Py_ssize_t* stop,
    Py_ssize_t* step, Py_ssize_t* slicelength) {
    if (PySlice_GetIndicesEx(key, total_size, start, stop, step, slicelength) < 0) {
        PyErr_SetString(PyExc_RuntimeError, "Internal error: get indices from slice failed!");
        return -1;
    }
    if (*step != 1) {
        PyErr_SetString(PyExc_ValueError, "Step must be 1!");
        return -2;
    }
    if (*slicelength < 1) {
        PyErr_SetString(PyExc_ValueError, "Slice must have length at least 1!");
        return -2;
    }
    return 0;
}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c *self, PyObject *key, PyObject *v) {
    /* TODO: YOUR CODE HERE */
    if (self->mat->is_1d) { // 1D matrix
        if (!PyLong_Check(key) && !PySlice_Check(key)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument key!");
            return -1;
        }

        int total_size = self->mat->rows * self->mat->cols;
        if (PyLong_Check(key)) { // if is a int
            int index = (int) PyLong_AsLong(key);
            if (!PyLong_Check(v) && !PyFloat_Check(v)) {
                PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
                return -1;
            }

            double val = (double) PyFloat_AsDouble(v);
            if (index < 0 || index >= total_size) {
                PyErr_SetString(PyExc_IndexError, "Index out of bounds!");
                return -2;
            }

            int row = index / self->mat->cols;
            int col = index % self->mat->cols;
            self->mat->data[row][col] = val;
            return 0;
        }

        // if is a slice
        Py_ssize_t start, stop, step, slicelength;
        if (getFromSlice(key, total_size, &start, &stop, &step, &slicelength)) {
            return -1;
        }

        if (slicelength == 1) {  // the slice is 1 by 1
            if (!PyLong_Check(v) && !PyFloat_Check(v)) {
                PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
                return -1;
            }
            double val = (double) PyFloat_AsDouble(v);
            if (self->mat->rows == 1) {
                self->mat->data[0][start] = val;
                return 0;
            }
            self->mat->data[start][0] = val;
            return 0;
        }

        if (!PyList_Check(v)) { // the slice is 1d
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
            return -1;
        }
        if (PyList_Size(v) != slicelength) {
            PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
            return -3;
        }

        for (int i = 0; i < slicelength; i++) {
            PyObject* item = PyList_GetItem(v, i);
            if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                PyErr_SetString(PyExc_ValueError, "Invalid value for argument v!");
                return -3;
            }
        }

        if (self->mat->rows == 1) {
            for (int i = 0; i < slicelength; i++) {
                self->mat->data[0][start + i] = PyFloat_AsDouble(PyList_GetItem(v, i));
            }
        } else {
            for (int i = 0; i < slicelength; i++) {
                self->mat->data[start + i][0] = PyFloat_AsDouble(PyList_GetItem(v, i));
            }
        }
        return 0;
    }

    //2-D matrix
    if (!PyLong_Check(key) && !PySlice_Check(key) && !(PyTuple_Check(key) && PyTuple_Size(key) == 2)) {
        PyErr_SetString(PyExc_TypeError, "Invalid value for argument!");
        return -1;
    }

    if (PyLong_Check(key)) { // Integer index for row
        int row = (int) PyLong_AsLong(key);
        if (row < 0 || row >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of bounds!");
            return -2;
        }
        int slicelength = self->mat->cols;
        if (!PyList_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
            return -1;
        }
        if (PyList_Size(v) != slicelength) {
            PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
            return -3;
        }
        for (int i = 0; i < slicelength; i++) {
            PyObject* item = PyList_GetItem(v, i);
            if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                PyErr_SetString(PyExc_ValueError, "Invalid value for argument v!");
                return -3;
            }
        }

        for (int i = 0; i < slicelength; i++) {
            self->mat->data[row][i] = PyFloat_AsDouble(PyList_GetItem(v, i));
        }
        return 0;
    }

    if (PySlice_Check(key)) { // Slice index for rows
        Py_ssize_t start, stop, step, slicelength;
        if (getFromSlice(key, self->mat->rows, &start, &stop, &step, &slicelength)) {
            return -1;
        }
        if (!PyList_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
            return -1;
        }
        if (PyList_Size(v) != slicelength) {
            PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
            return -3;
        }
        for (int i = 0; i < slicelength; i++) {
            if (PyList_Size(PyList_GetItem(v, i)) != self->mat->cols) {
                PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
                return -3;
            }
            for (int j = 0; j < self->mat->cols; j++) {
                PyObject*  item = PyList_GetItem(PyList_GetItem(v, i), j);
                if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                    PyErr_SetString(PyExc_ValueError, "Invalid value for argument v!");
                    return -3;
                }
            }
        }

        for (int i = 0; i < slicelength; i++) {
            for (int j = 0; j < self->mat->cols; j++) {
                self->mat->data[start + i][j] = PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(v, i), j));
            }
        }
        return 0;
    }

    // Tuple index
    PyObject *row_key = PyTuple_GetItem(key, 0);
    PyObject *col_key = PyTuple_GetItem(key, 1);
    if (!(PyLong_Check(row_key) || PySlice_Check(row_key)) ||
        !(PyLong_Check(col_key) || PySlice_Check(col_key))) {
        PyErr_SetString(PyExc_TypeError, "Tuple elements must be integers or slices");
        return -1;
    }

    // Handle row index
    Py_ssize_t row_start, row_stop, row_step, row_slicelength = 0;
    int row_is_int = 0;
    int row_index = 0;
    if (PyLong_Check(row_key)) {
        row_is_int = 1;
        row_index = (int) PyLong_AsLong(row_key);

        if (row_index < 0 || row_index >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Row index out of bounds");
            return -2;
        }
    } else {
        if (getFromSlice(row_key, self->mat->rows, &row_start, &row_stop,
            &row_step, &row_slicelength)) {
            return -1;
        }
    }
    // Handle col index
    Py_ssize_t col_start, col_stop, col_step, col_slicelength = 0;
    int col_is_int = 0;
    int col_index = 0;
    if (PyLong_Check(col_key)) {
        col_is_int = 1;
        col_index = (int) PyLong_AsLong(col_key);

        if (col_index < 0 || col_index >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "Col index out of bounds");
            return -2;
        }
    } else {
        if (getFromSlice(col_key, self->mat->cols, &col_start, &col_stop,
            &col_step, &col_slicelength)) {
            return -1;
        }
    }

    if (row_is_int == 1 && col_is_int == 1) { // 1 by 1 slice
        if (!PyLong_Check(v) && !PyFloat_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
            return -1;
        }
        double val = (double) PyFloat_AsDouble(v);
        self->mat->data[row_index][col_index] = val;
        return 0;
    }
    if (row_slicelength == 1 && col_slicelength == 1) { // 1 by 1 slice
        if (!PyLong_Check(v) && !PyFloat_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
            return -1;
        }
        double val = (double) PyFloat_AsDouble(v);
        self->mat->data[row_start][col_start] = val;
        return 0;
    }

    if (!PyList_Check(v)) {
        PyErr_SetString(PyExc_TypeError, "Invalid value for argument v!");
        return -1;
    }
    if (!(row_is_int == 1 || row_slicelength == 1) && PyList_Size(v) != row_slicelength) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
        return -3;
    }
    if ((row_is_int == 1 || row_slicelength == 1) && PyList_Size(v) != col_slicelength){
        PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
        return -3;
    }

    if (col_is_int == 1 || col_slicelength == 1) {
        for (int i = 0; i < row_slicelength; i++) {
            PyObject* item = PyList_GetItem(v, i);
            if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                PyErr_SetString(PyExc_ValueError, "Invalid value for argument v!");
                return -3;
            }
        }
    } else if (row_is_int == 1 || row_slicelength == 1) {
        for (int i = 0; i < col_slicelength; i++) {
            PyObject* item = PyList_GetItem(v, i);
            if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                PyErr_SetString(PyExc_ValueError, "Invalid value for argument v!");
                return -3;
            }
        }
    } else {
        for (int i = 0; i < row_slicelength; i++) {
            if (PyList_Size(PyList_GetItem(v, i)) != col_slicelength) {
                PyErr_SetString(PyExc_ValueError, "Incorrect number of arguments v!");
                return -3;
            }
            for (int j = 0; j < col_slicelength; j++) {
                PyObject*  item = PyList_GetItem(PyList_GetItem(v, i), j);
                if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                    PyErr_SetString(PyExc_ValueError, "Invalid value for argument v!");
                    return -3;
                }
            }
        }
    }

    int row_offset = row_is_int == 1 ? row_index : row_start;
    int col_offset = col_is_int == 1 ? col_index : col_start;
    int row = row_is_int == 1 ? 1 : row_slicelength;
    int col = col_is_int == 1 ? 1 : col_slicelength;

    if (col_is_int == 1 || col_slicelength == 1) {
        for (int i = 0; i < row; i++) {
            self->mat->data[row_offset + i][col_offset] = PyFloat_AsDouble(PyList_GetItem(v, i));
        }
    } else if (row_is_int == 1 || row_slicelength == 1) {
        for (int i = 0; i < col; i++) {
            self->mat->data[row_offset][col_offset + i] = PyFloat_AsDouble(PyList_GetItem(v, i));
        }

    } else {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                self->mat->data[row_offset + i][col_offset + j] = PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(v, i), j));
            }
        }
    }
    return 0;
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL} /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor) Matrix61c_dealloc,
    .tp_repr = (reprfunc) Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
                Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc) Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}



