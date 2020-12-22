#include "cmatrix.h"

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
	    for (int k = 0; k < j; k++) {
		s += L[i * n + k] * conjf(L[j * n + k]);
	    }

	    L[i * n + j] = (i == j) ? sqrtf((float)(A[i * n + i] - s))
				    : (1.0 / L[j * n + j] * (A[i * n + j] - s));
	}
    }

    return L;
}

complex float *matmul(complex float *A, complex float *B, int M, int K, int N,
		      complex float *result) {
    for (int i = 0; i < M; i++) {
	for (int j = 0; j < N; j++) {
	    result[i * N + j] = 0;
	    for (int k = 0; k < K; k++) {
		result[i * N + j] += (A[i * K + k] * B[k * N + j]);
	    }
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

complex float *finversion(complex float *L, int n, complex float *inversion) {
    complex float *eye = calloc(n * n, sizeof(complex float));
    for (int i = 0; i < n; i++) {
	eye[i * n + i] = 1.f;
    }

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

#if 0
int main() {
    int n = 4;

    complex float *m = calloc(n * n, sizeof(complex float));
    for (int i = 0; i < n; i++) {
	/* m[i * n + i] = (complex float)(rand() % 256)*10; */
	m[i * n + i] = 512.0;
	for (int j = 0; j < i; j++) {
	    m[i * n + j] = (complex float)((float)(rand() % 256) +
					   I * (float)(rand() % 256));
	    m[j * n + i] = m[i * n + j];
	}
    }
    printf("\noriginal matrix\n");
    show_matrix(m, n, n);

    complex float *l = calloc(n * n, sizeof(complex float));
    complex float *lh = calloc(n * n, sizeof(complex float));
    complex float *llh = calloc(n * n, sizeof(complex float));
    complex float *l_inv = calloc(n * n, sizeof(complex float));
    complex float *l_linv = calloc(n * n, sizeof(complex float));
    complex float *test = calloc(n * n, sizeof(complex float));
    complex float *test1 = calloc(3 * 1, sizeof(complex float));

    cholesky(m, n, l);
    printf("\ndecomposition\n");
    show_matrix(l, n, n);

    hermitian(l, n, lh);
    printf("\nHermittian\n");
    show_matrix(lh, n, n);

    printf("\nLL^H\n");
    matmul(l, lh, n, n, n, llh);
    show_matrix(llh, n, n);

    printf("\nL inversion\n");
    finversion(l, n, l_inv);
    show_matrix(l_inv, n, n);

    printf("\nL matmul L inversion\n");
    matmul(l, l_inv, n, n, n, l_linv);
    show_matrix(l_linv, n, n);

    complex float a[3] = {1.f + I * 2.f, 3.f + I * 4.f, 5.f + I * 6.f};
    complex float b[3] = {1.f + I * 2.f, 3.f + I * 4.f, 5.f + I * 6.f};
    complex float c[3] = {1.f + I * 2.f, 3.f + I * 4.f, 5.f + I * 6.f};

    matmul(a, b, 3, 1, 3, test);
    show_matrix(test, 3, 3);
    matmul(test, c, 3, 3, 1, test1);
    show_matrix(test1, 3, 1);
    matmul(c, test1, 1, 3, 1, test);
    show_matrix(test, 1, 1);

    free(test);
    free(test1);
    free(m);
    free(l);
    free(lh);
    free(llh);
    free(l_inv);
    free(l_linv);

    return 0;
}
#endif
