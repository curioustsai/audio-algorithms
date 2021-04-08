#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "cmatrix.h"
#include "complex.h"

TEST(cmatrix, matmul) {
    _Complex float *test = (_Complex float *)calloc(3 * 3, sizeof(_Complex float));

    // c++11 not support _complex initialization ...
    float a[3 * 2] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    float b[3 * 2] = {7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    float result_ab[3 * 3 * 2] = {-9.0, 22.0,  -11.0, 28.0,  -13.0, 34.0,  -11.0, 52.0,  -13.0,
                                  66.0, -15.0, 80.0,  -13.0, 82.0,  -15.0, 104.0, -17.0, 126.0};

    _Complex float *result = (_Complex float *)result_ab;

    // a: 3x1, b: 1x3, result: 3x3
    matmul((_Complex float *)a, (_Complex float *)b, 3, 1, 3, test);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float diff_real = creal(test[i * 3 + j]) - creal(result[i * 3 + j]);
            float diff_imag = cimag(test[i * 3 + j]) - cimag(result[i * 3 + j]);

            ASSERT_EQ(diff_real, 0.0);
            ASSERT_EQ(diff_imag, 0.0);
        }
    }

    float result_ba[2] = {-39.0, 214.0};

    // a: 1x3, b:3x1, result: 1x1
    matmul((_Complex float *)a, (_Complex float *)b, 1, 3, 1, test);

    float real_part = creal(test[0]);
    float imag_part = cimag(test[0]);
    ASSERT_EQ(real_part, result_ba[0]);
    ASSERT_EQ(imag_part, result_ba[1]);

    // a: 3x3, b: 3x3 result: 3x3
    float a33[3 * 3 * 2] = {1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,
                            10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0};
    float b33[3 * 3 * 2] = {7.0,   -8.0, 9.0,   -10.0, 11.0,  -12.0, 13.0,  -14.0, 15.0,
                            -16.0, 17.0, -18.0, 19.0,  -20.0, 21.0,  -22.0, 23.0,  -24.0};
    float result_ab33[9 * 2] = {333.0, 30.0,   375.0, 36.0,   417.0, 42.0,   819.0, 12.0,   933.0,
                                18.0,  1047.0, 24.0,  1305.0, -6.0,  1491.0, 0.0,   1677.0, 6.0};
    result = (_Complex float *)result_ab33;
    matmul((_Complex float *)a33, (_Complex float *)b33, 3, 3, 3, test);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float diff_real = creal(test[i * 3 + j]) - creal(result[i * 3 + j]);
            float diff_imag = cimag(test[i * 3 + j]) - cimag(result[i * 3 + j]);

            ASSERT_EQ(diff_real, 0.0);
            ASSERT_EQ(diff_imag, 0.0);
        }
    }

    free(test);
}

TEST(cmatrix, hermitian) {
    _Complex float *test = (_Complex float *)calloc(3 * 3, sizeof(_Complex float));
    float a33[3 * 3 * 2] = {1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,
                            10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0};
    float result_a33[3 * 3 * 2] = {1.0,   -2.0, 7.0,   -8.0, 13.0, -14.0, 3.0,   -4.0, 9,
                                   -10.0, 15.0, -16.0, 5.0,  -6.0, 11.0,  -12.0, 17.0, -18.0};
    _Complex float *result = (_Complex float *)result_a33;

    hermitian((_Complex float *)a33, 3, test);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float diff_real = creal(test[i * 3 + j]) - creal(result[i * 3 + j]);
            float diff_imag = cimag(test[i * 3 + j]) - cimag(result[i * 3 + j]);

            ASSERT_EQ(diff_real, 0.0);
            ASSERT_EQ(diff_imag, 0.0);
        }
    }

    free(test);
}

TEST(cmatrix, cholesky) {
    int n = 2;
    _Complex float *test = (_Complex float *)calloc(n * n, sizeof(_Complex float));
    float a22[4 * 2] = {1.0, 0.0, 0.0, -2.0, 0.0, 2.0, 5.0, 0.0};
    float result_choleksy[4 * 2] = {1.0, 0.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0};
    _Complex float *result = (_Complex float *)result_choleksy;

    cholesky((_Complex float *)a22, n, test);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float diff_real = creal(test[i * n + j]) - creal(result[i * n + j]);
            float diff_imag = cimag(test[i * n + j]) - cimag(result[i * n + j]);

            ASSERT_EQ(diff_real, 0.0);
            ASSERT_EQ(diff_imag, 0.0);
        }
    }

    free(test);
}

TEST(cmatrix, inversion) {
    int n = 2;
    _Complex float *test = (_Complex float *)calloc(n * n, sizeof(_Complex float));
    float a22[4 * 2] = {1.0, 0.0, 0.0, 0.0, 0.0, 2.0, 5.0, 0.0};
    float result_inv[4 * 2] = {1.0, 0.0, 0.0, 0.0, 0.0, -0.4, 0.2, 0.0};
    _Complex float *result = (_Complex float *)result_inv;

    finversion((_Complex float *)a22, n, test);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float diff_real = creal(test[i * n + j]) - creal(result[i * n + j]);
            float diff_imag = cimag(test[i * n + j]) - cimag(result[i * n + j]);

            ASSERT_EQ(diff_real, 0.0);
            ASSERT_EQ(diff_imag, 0.0);
        }
    }

    free(test);
}
