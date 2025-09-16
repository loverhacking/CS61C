#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/

/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fieds of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix(matrix **mat, int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (rows <= 0 || cols <= 0) {
        return -1;
    }
    matrix *new_mat = malloc(sizeof(matrix));
    if (new_mat == NULL) {
        return -1;
    }

    new_mat->data = malloc(rows * sizeof(double*));
    if (new_mat->data == NULL) {
        free(new_mat);
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        new_mat->data[i] = calloc(cols, sizeof(double));
        if (new_mat->data[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(new_mat->data[j]);
            }
            free(new_mat->data);
            free(new_mat);
            return -1;
        }
    }
    new_mat->rows = rows;
    new_mat->cols = cols;
    new_mat->is_1d = (rows == 1 || cols == 1) ? 1 : 0;
    new_mat->ref_cnt = 1;
    new_mat->parent = NULL;

    *mat = new_mat;
    return 0;
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset,
                        int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (rows <= 0 || cols <= 0 || row_offset < 0 || col_offset < 0 ||
        rows + row_offset > from->rows || cols + col_offset > from->cols) {
        return -1;
    }

    matrix *new_mat = malloc(sizeof(matrix));
    if (new_mat == NULL) {
        return -1;
    }

    new_mat->data = malloc(rows * sizeof(double*));
    if (new_mat->data == NULL) {
        free(new_mat);
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        new_mat->data[i] = from->data[row_offset + i] + col_offset;
    }

    new_mat->rows = rows;
    new_mat->cols = cols;
    new_mat->is_1d = (rows == 1 || cols == 1) ? 1 : 0;
    new_mat->ref_cnt = 1;
    new_mat->parent = from;

    from->ref_cnt++;
    *mat = new_mat;
    return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    /* TODO: YOUR CODE HERE */
    matrix * cur = mat;
    while (cur) {
        // If the matrix has a parent, it's a slice.
        // The function frees the slice structure and moves to the parent,
        // decrementing its reference count.
        if (cur->parent) {
            matrix * parent = cur->parent;
            free(cur);
            cur = parent;
            cur->ref_cnt--;
            // If at any point a parent's reference count remains positive after decrementing,
            // the loop terminates early since other references still exist.
            if (cur->ref_cnt > 0) {
                break;
            }
        } else {
            // If the matrix has no parent, it's a root.
            // The function decrements its reference count.
            // If the count reaches zero, it frees the data and the matrix structure.
            cur->ref_cnt--;
            if (cur->ref_cnt == 0) {
                for (int i = 0; i < cur->rows; i++) {
                    free(cur->data[i]);
                }
                free(cur->data);
                free(cur);
            }
            break;
        }
    }
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double get(matrix *mat, int row, int col) {
    /* TODO: YOUR CODE HERE */
    return mat->data[row][col];
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void set(matrix *mat, int row, int col, double val) {
    /* TODO: YOUR CODE HERE */
    mat->data[row][col] = val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
    /* TODO: YOUR CODE HERE */
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            mat->data[i][j] = val;
        }
    }
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        return -2;
    }
    if (result->rows != mat1->rows || result->cols != mat1->cols) {
        return -3;
    }
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
        }
    }
    return 0;
}

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        return -2;
    }
    if (result->rows != mat1->rows || result->cols != mat1->cols) {
        return -3;
    }
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = mat1->data[i][j] - mat2->data[i][j];
        }
    }
    return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    if (mat1->cols != mat2->rows) {
        return -2;
    }
    if (result->rows != mat1->rows || result->cols != mat2->cols) {
        return -3;
    }

    for (int i = 0; i < mat1->rows; i++) {
        for (int j = 0; j < mat2->cols; j++) {
            double sum = 0;
            for (int k = 0; k < mat1->cols; k++) {
                sum += mat1->data[i][k] * mat2->data[k][j];
            }
            result->data[i][j] = sum;
        }
    }
    return 0;
}

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise multiplication.
 */
int pow_matrix(matrix *result, matrix *mat, int pow) {
    /* TODO: YOUR CODE HERE */
    if (result->rows != mat->rows || result->cols != mat->cols ||
        result->rows != result->cols) {
        return -2;
    }
    if (pow < 0) {
        return -3;
    }

    // initial result->data to be I (identity matrix)
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = i == j;
        }
    }

    // Create a temporary matrix for storing intermediate results
    matrix temp;
    temp.rows = result->rows;
    temp.cols = result->cols;
    temp.data = (double **)malloc(temp.rows * sizeof(double *));
    for (int i = 0; i < temp.rows; i++) {
        temp.data[i] = (double *)malloc(temp.cols * sizeof(double));
    }

    for (int l = 0; l < pow; l++) {
        for (int i = 0; i < result->rows; i++) {
            for (int j = 0; j < mat->cols; j++) {
                double sum = 0.0;
                for (int k = 0; k < result->cols; k++) {
                    sum += result->data[i][k] * mat->data[k][j];
                }
                temp.data[i][j] = sum;
            }
        }

        // Copy temp to result
        for (int i = 0; i < result->rows; i++) {
            for (int j = 0; j < result->cols; j++) {
                result->data[i][j] = temp.data[i][j];
            }
        }
    }

    // Free temporary matrix
    for (int i = 0; i < temp.rows; i++) {
        free(temp.data[i]);
    }
    free(temp.data);
    return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    /* TODO: YOUR CODE HERE */
    if (mat->rows != result->rows || mat->cols != result->cols) {
        return -2;
    }
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = -mat->data[i][j];
        }
    }
    return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    /* TODO: YOUR CODE HERE */
    if (mat->rows != result->rows || mat->cols != result->cols) {
        return -2;
    }
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = mat->data[i][j] > 0 ? mat->data[i][j] : -mat->data[i][j];
        }
    }
    return 0;
}

