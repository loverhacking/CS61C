#include "transpose.h"

/* The naive transpose function as a reference. */
void transpose_naive(int n, int blocksize, int *dst, int *src) {
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            dst[y + x * n] = src[x + y * n];
        }
    }
}

/* Implement cache blocking below. You should NOT assume that n is a
 * multiple of the block size. */
void transpose_blocking(int n, int blocksize, int *dst, int *src) {
    for (int i0 = 0; i0 < n; i0 += blocksize) {
        for (int j0 = 0; j0 < n; j0 += blocksize) {
            int i_end = i0 + blocksize < n ? i0 + blocksize : n;
            int j_end = j0 + blocksize < n ? j0 + blocksize : n;
            for (int i = i0; i < i_end; i++) {
                for (int j = j0; j < j_end; j++) {
                    dst[j + i * n] = src[i + j * n];
                }
            }
        }
    }
}
