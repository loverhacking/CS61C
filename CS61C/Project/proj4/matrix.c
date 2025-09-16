#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>

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
        double *row = result->data[i];
        int total_cols = result->cols;
        int paralleled_index = total_cols / 16 * 16;

        // Vectorized processing for the current row
        for (int j = 0; j < paralleled_index; j += 16) {
            _mm256_storeu_pd(row + j, _mm256_set_pd(rand_double(low, high), rand_double(low, high),
                rand_double(low, high), rand_double(low, high)));
            _mm256_storeu_pd(row + j + 4, _mm256_set_pd(rand_double(low, high), rand_double(low, high),
                rand_double(low, high), rand_double(low, high)));
            _mm256_storeu_pd(row + j + 8, _mm256_set_pd(rand_double(low, high), rand_double(low, high),
                rand_double(low, high), rand_double(low, high)));
            _mm256_storeu_pd(row + j + 12, _mm256_set_pd(rand_double(low, high), rand_double(low, high),
                rand_double(low, high), rand_double(low, high)));
        }

        // Process remaining elements in the row
        for (int j = paralleled_index; j < total_cols; j++) {
            row[j] = rand_double(low, high);
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
    __m256d mask_avx = _mm256_set1_pd(val);

#pragma omp parallel for
    for (int i = 0; i < mat->rows; i++) {
        double *row = mat->data[i];
        int total_cols = mat->cols;
        int paralleled_index = total_cols / 16 * 16;

        // Vectorized processing for the current row
        for (int j = 0; j < paralleled_index; j += 16) {
            _mm256_storeu_pd(row + j, mask_avx);
            _mm256_storeu_pd(row + j + 4, mask_avx);
            _mm256_storeu_pd(row + j + 8, mask_avx);
            _mm256_storeu_pd(row + j + 12, mask_avx);
        }

        // Process remaining elements in the row
        for (int j = paralleled_index; j < total_cols; j++) {
            row[j] = val;
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

#pragma omp parallel for
    for (int i = 0; i < result->rows; i++) {
        double *row_in1 = mat1->data[i];
        double *row_in2 = mat2->data[i];
        double *row_out = result->data[i];
        int total_cols = result->cols;
        int paralleled_index = total_cols / 16 * 16;

        // Declare AVX variables inside the loop to avoid data race
        __m256d data00, data01, data02, data03;
        __m256d data10, data11, data12, data13;

        // Vectorized processing for the current row
        for (int j = 0; j < paralleled_index; j += 16) {
            data00 = _mm256_loadu_pd(row_in1 + j);
            data01 = _mm256_loadu_pd(row_in1 + j + 4);
            data02 = _mm256_loadu_pd(row_in1 + j + 8);
            data03 = _mm256_loadu_pd(row_in1 + j + 12);

            data10 = _mm256_loadu_pd(row_in2 + j);
            data11 = _mm256_loadu_pd(row_in2 + j + 4);
            data12 = _mm256_loadu_pd(row_in2 + j + 8);
            data13 = _mm256_loadu_pd(row_in2 + j + 12);

            _mm256_storeu_pd(row_out + j, _mm256_add_pd(data00, data10));
            _mm256_storeu_pd(row_out + j + 4, _mm256_add_pd(data01, data11));
            _mm256_storeu_pd(row_out + j + 8, _mm256_add_pd(data02, data12));
            _mm256_storeu_pd(row_out + j + 12, _mm256_add_pd(data03, data13));
        }

        // Process remaining elements in the row
        for (int j = paralleled_index; j < total_cols; j++) {
            row_out[j] = row_in1[j] + row_in2[j];
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

#pragma omp parallel for
    for (int i = 0; i < result->rows; i++) {
        double *row_in1 = mat1->data[i];
        double *row_in2 = mat2->data[i];
        double *row_out = result->data[i];
        int total_cols = result->cols;
        int paralleled_index = total_cols / 16 * 16;

        // Declare AVX variables inside the loop to avoid data race
        __m256d data00, data01, data02, data03;
        __m256d data10, data11, data12, data13;

        // Vectorized processing for the current row
        for (int j = 0; j < paralleled_index; j += 16) {
            data00 = _mm256_loadu_pd(row_in1 + j);
            data01 = _mm256_loadu_pd(row_in1 + j + 4);
            data02 = _mm256_loadu_pd(row_in1 + j + 8);
            data03 = _mm256_loadu_pd(row_in1 + j + 12);

            data10 = _mm256_loadu_pd(row_in2 + j);
            data11 = _mm256_loadu_pd(row_in2 + j + 4);
            data12 = _mm256_loadu_pd(row_in2 + j + 8);
            data13 = _mm256_loadu_pd(row_in2 + j + 12);

            _mm256_storeu_pd(row_out + j, _mm256_sub_pd(data00, data10));
            _mm256_storeu_pd(row_out + j + 4, _mm256_sub_pd(data01, data11));
            _mm256_storeu_pd(row_out + j + 8, _mm256_sub_pd(data02, data12));
            _mm256_storeu_pd(row_out + j + 12, _mm256_sub_pd(data03, data13));
        }

        // Process remaining elements in the row
        for (int j = paralleled_index; j < total_cols; j++) {
            row_out[j] = row_in1[j] - row_in2[j];
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

    matrix *temp_result = NULL;
    allocate_matrix(&temp_result, result->rows, result->cols);

    int total_cols = result->cols;
    int paralleled_index = total_cols / 16 * 16;

    /*
     * e.g. A = [[a00, a01], [a10, a11], B = [[b00, b01], [b10, b11]]
     * compute C = A × B:
     * C[0][0] = a00×b00 + a01×b10
     * C[0][1] = a00×b01 + a01×b11
     * C[1][0] = a10×b00 + a11×b10
     * C[1][1] = a10×b01 + a11×b11
     * In the code, this process is broken down as follows:
     * For i=0 and k=0: Broadcast a00 to the vector.
     *      Calculate the first row of the temporary result: a00×[b00, b01,...] + initial value (0).
     * For i=0 and k=1: Broadcast a01 to the vector.
     *      Calculate the first row of the temporary result: a01×[b10, b11,...] + previous result.
     * Repeat this process until all rows and columns have been processed.
     */
#pragma omp parallel for
    for(int i = 0; i < mat1->rows; i++) {
        for (int k = 0;k < mat1->cols; k++) {
             __m256d left_data = _mm256_set1_pd(mat1->data[i][k]);
            for (int j = 0; j < paralleled_index; j += 16) {
                __m256d temp_sum0 = _mm256_loadu_pd(temp_result->data[i] + j);
                __m256d right_data0 = _mm256_loadu_pd(mat2->data[k] + j);

                __m256d temp_sum1 = _mm256_loadu_pd(temp_result->data[i] + j + 4);
                __m256d right_data1 = _mm256_loadu_pd(mat2->data[k] + j + 4);

                __m256d temp_sum2 = _mm256_loadu_pd(temp_result->data[i] + j + 8);
                __m256d right_data2 = _mm256_loadu_pd(mat2->data[k] + j + 8);

                __m256d temp_sum3 = _mm256_loadu_pd(temp_result->data[i] + j + 12);
                __m256d right_data3 = _mm256_loadu_pd(mat2->data[k] + j + 12);

                // temp_sum0 = left_data * right_data0 + temp_sum0
                _mm256_storeu_pd(temp_result->data[i] + j,
                                 _mm256_fmadd_pd(left_data, right_data0, temp_sum0));

                _mm256_storeu_pd(temp_result->data[i] + j + 4,
                                 _mm256_fmadd_pd(left_data, right_data1, temp_sum1));

                _mm256_storeu_pd(temp_result->data[i] + j + 8,
                                 _mm256_fmadd_pd(left_data, right_data2, temp_sum2));

                _mm256_storeu_pd(temp_result->data[i] + j + 12,
                                 _mm256_fmadd_pd(left_data, right_data3, temp_sum3));
            }

            for (int j = paralleled_index; j < total_cols; j++) {
                temp_result->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
            }
        }
    }
    result->data = temp_result->data;
    free(temp_result);
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

    int error_code = 0;
    for (int i = 0; i < pow; ++i) {
        if ((error_code = mul_matrix(result, result, mat)) != 0) {
            return error_code;
        }
    }
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
    __m256d zero_avx = _mm256_set1_pd(0);

#pragma omp parallel for
    for (int i = 0; i < mat->rows; i++) {
        double *row_in = mat->data[i];
        double *row_out = result->data[i];
        int total_cols = mat->cols;
        int paralleled_index = total_cols / 16 * 16;

        // Declare AVX variables inside the loop to avoid data race
        __m256d data0, data1, data2, data3;

        // Vectorized processing for the current row
        for (int j = 0; j < paralleled_index; j += 16) {
            data0 = _mm256_loadu_pd(row_in + j);
            data1 = _mm256_loadu_pd(row_in + j + 4);
            data2 = _mm256_loadu_pd(row_in + j + 8);
            data3 = _mm256_loadu_pd(row_in + j + 12);

            _mm256_storeu_pd(row_out + j, _mm256_sub_pd(zero_avx , data0));
            _mm256_storeu_pd(row_out + j + 4, _mm256_sub_pd(zero_avx , data1));
            _mm256_storeu_pd(row_out + j + 8, _mm256_sub_pd(zero_avx , data2));
            _mm256_storeu_pd(row_out + j + 12, _mm256_sub_pd(zero_avx , data3));
        }

        // Process remaining elements in the row
        for (int j = paralleled_index; j < total_cols; j++) {
            row_out[j] = -row_in[j];
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

    uint64_t mask_int = ~((uint64_t) 1 << 63); // 0111...1
    double mask_double;
    memcpy(&mask_double, &mask_int, sizeof(mask_double));
    __m256d mask_avx = _mm256_set1_pd(mask_double);

#pragma omp parallel for
    for (int i = 0; i < mat->rows; i++) {
        double *row_in = mat->data[i];
        double *row_out = result->data[i];
        int total_cols = mat->cols;
        int paralleled_index = total_cols / 16 * 16;

        // Declare AVX variables inside the loop to avoid data race
        __m256d data0, data1, data2, data3;

        // Vectorized processing for the current row
        for (int j = 0; j < paralleled_index; j += 16) {
            data0 = _mm256_loadu_pd(row_in + j);
            data1 = _mm256_loadu_pd(row_in + j + 4);
            data2 = _mm256_loadu_pd(row_in + j + 8);
            data3 = _mm256_loadu_pd(row_in + j + 12);
            // In IEEE 754 double precision floating point representation:
            // The highest position (63rd) is the symbol bit (0 for positive, 1 for negative)
            // The remaining 63 represent the index and the tail number
            // When this mask is bitwise ANDed with a double precision floating point number:
            // It clears the symbol bit (set to 0)
            // Keep the index and decimal part unchanged
            // This effectively converts any floating point number to its absolute value because:
            // The sign bit of the positive number is already 0 and remains unchanged
            // The sign bit of the negative number is cleared and becomes a positive number
            _mm256_storeu_pd(row_out + j, _mm256_and_pd(mask_avx, data0));
            _mm256_storeu_pd(row_out + j + 4, _mm256_and_pd(mask_avx, data1));
            _mm256_storeu_pd(row_out + j + 8, _mm256_and_pd(mask_avx, data2));
            _mm256_storeu_pd(row_out + j + 12, _mm256_and_pd(mask_avx, data3));
        }

        // Process remaining elements in the row
        for (int j = paralleled_index; j < total_cols; j++) {
            row_out[j] = row_in[j] > 0 ? row_in[j] : -row_in[j];
        }
    }

    return 0;
}

