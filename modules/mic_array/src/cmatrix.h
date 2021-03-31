#ifndef __CMATRIX_H__
#define __CMATRIX_H__

#include <complex.h>

#ifdef __cplusplus
extern "C" {
#endif 

void show_matrix(_Complex float *A, int M, int N);
_Complex float *matmul(_Complex float *A, _Complex float *B, int M, int K, int N, _Complex float *result);
_Complex float *hermitian(_Complex float *A, int n, _Complex float *hermit);

/* foward inversion, only works for lower triangle matrix */
_Complex float *finversion(_Complex float *L, int n, _Complex float *inversion);
_Complex float *cholesky(_Complex float *A, int n, _Complex float *L);

#ifdef __cplusplus
}
#endif 

#endif /* end of include guard: __CMATRIX_H__ */
