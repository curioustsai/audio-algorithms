#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void show_matrix(complex float *A, int M, int N);
complex float *matmul(complex float *A, complex float *B, int M, int K, int N, complex float *result);
complex float *hermitian(complex float *A, int n, complex float *hermit);
complex float *finversion(complex float *L, int n, complex float *inversion);
complex float *cholesky(complex float *A, int n, complex float *L);
