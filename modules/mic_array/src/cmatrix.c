#include "cmatrix.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void show_matrix(complex float *A, int M, int N) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++)
            printf("%2.5f + %2.fi\t", creal(A[i * N + j]), cimag(A[i * N + j]));
        printf("\n");
    }
}

complex float *cholesky(complex float *A, int n, complex float *L) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < (i + 1); j++) {
            complex float s = 0;
            for (int k = 0; k < j; k++) { s += L[i * n + k] * conjf(L[j * n + k]); }

            L[i * n + j] = (i == j) ? sqrtf((float)(A[i * n + i] - s))
                                    : (1.0f / L[j * n + j] * (A[i * n + j] - s));
        }
    }

    return L;
}

complex float *matmul(complex float *A, complex float *B, int M, int K, int N,
                      complex float *result) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            result[i * N + j] = 0;
            for (int k = 0; k < K; k++) { result[i * N + j] += (A[i * K + k] * B[k * N + j]); }
        }
    }

    return result;
}

complex float *hermitian(complex float *A, int n, complex float *hermit) {
    for (int i = 0; i < n; i++) {
        hermit[i * n + i] = conjf(A[i * n + i]);
        for (int j = 0; j < i; j++) {
            hermit[i * n + j] = conjf(A[j * n + i]);
            hermit[j * n + i] = conjf(A[i * n + j]);
        }
    }

    return hermit;
}

/* forward inversion */
complex float *finversion(complex float *L, int n, complex float *inversion) {
    complex float *eye = calloc(n * n, sizeof(complex float));
    for (int i = 0; i < n; i++) { eye[i * n + i] = 1.f; }

    for (int j = 0; j < n; j++) {
        for (int i = j; i < n; i++) {
            inversion[i * n + j] = eye[i * n + j] / L[i * n + i];
            for (int k = i + 1; k < n; k++) {
                eye[k * n + j] -= L[k * n + i] * inversion[i * n + j];
            }
        }
    }

    free(eye);

    return inversion;
}

