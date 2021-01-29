#include "basic_op.h"

/**
  @brief         Copies the elements of a f16 vector.
  @param[in]     pSrc       points to input vector
  @param[out]    pDst       points to output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */
void uiv_copy_q15(const uiv_q15_t *pSrc, uiv_q15_t *pDst, uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrc[i]; }
}

void uiv_copy_q31(const uiv_q31_t *pSrc, uiv_q31_t *pDst, uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrc[i]; }
}

void uiv_copy_f32(const uiv_f32_t *pSrc, uiv_f32_t *pDst, uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrc[i]; }
}

/*
  @brief         Fills a constant value into a f16 vector.
  @param[in]     value      input value to be filled
  @param[out]    pDst       points to output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */
void uiv_fill_q15(const uiv_q15_t value, uiv_q15_t *buf, uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { buf[i] = value; }
}

void uiv_fill_q31(const uiv_q31_t value, uiv_q31_t *buf, uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { buf[i] = value; }
}

void uiv_fill_f32(const uiv_f32_t value, uiv_f32_t *buf, uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { buf[i] = value; }
}

/**
  @brief         Floating-point vector addition.
  @param[in]     pSrcA      points to first input vector
  @param[in]     pSrcB      points to second input vector
  @param[out]    pDst       points to output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */

void uiv_add_q15(const uiv_q15_t *pSrcA, const uiv_q15_t *pSrcB, uiv_q15_t *pDst,
                 uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] + pSrcB[i]; }
}

void uiv_add_q31(const uiv_q31_t *pSrcA, const uiv_q31_t *pSrcB, uiv_q31_t *pDst,
                 uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] + pSrcB[i]; }
}

void uiv_add_f32(const uiv_f32_t *pSrcA, const uiv_f32_t *pSrcB, uiv_f32_t *pDst,
                 uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] + pSrcB[i]; }
}

/**
  @brief         Floating-point vector subtraction.
  @param[in]     pSrcA      points to the first input vector
  @param[in]     pSrcB      points to the second input vector
  @param[out]    pDst       points to the output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */
void uiv_sub_q15(const uiv_q15_t *pSrcA, const uiv_q15_t *pSrcB, uiv_q15_t *pDst,
                 uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] - pSrcB[i]; }
}

void uiv_sub_q31(const uiv_q31_t *pSrcA, const uiv_q31_t *pSrcB, uiv_q31_t *pDst,
                 uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] - pSrcB[i]; }
}

void uiv_sub_f32(const uiv_f32_t *pSrcA, const uiv_f32_t *pSrcB, uiv_f32_t *pDst,
                 uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] - pSrcB[i]; }
}

/**
  @brief         Floating-point vector multiplication.
  @param[in]     pSrcA      points to the first input vector.
  @param[in]     pSrcB      points to the second input vector.
  @param[out]    pDst       points to the output vector.
  @param[in]     blockSize  number of samples in each vector.
  @return        none
 */

void uiv_mult_q15(const uiv_q15_t *pSrcA, const uiv_q15_t *pSrcB, uiv_q15_t *pDst,
                  uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] * pSrcB[i]; }
}

void uiv_mult_q31(const uiv_q31_t *pSrcA, const uiv_q31_t *pSrcB, uiv_q31_t *pDst,
                  uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] * pSrcB[i]; }
}

void uiv_mult_f32(const uiv_f32_t *pSrcA, const uiv_f32_t *pSrcB, uiv_f32_t *pDst,
                  uint32_t block_size) {
    uint32_t i;
    for (i = 0; i < block_size; i++) { pDst[i] = pSrcA[i] * pSrcB[i]; }
}

void uiv_cmplx_dot_prod_f32(const uiv_q31_t *pSrcA, const uiv_q31_t *pSrcB, uint32_t numSamples,
                            uiv_q31_t *realResult, uiv_q31_t *imagResult) {
    uint32_t i;
    uiv_q31_t a0, b0, c0, d0;
    uiv_q31_t real_sum = 0;
    uiv_q31_t imag_sum = 0;

    for (i = 0; i < numSamples; i++) {
        a0 = *pSrcA++;
        b0 = *pSrcA++;
        c0 = *pSrcB++;
        d0 = *pSrcB++;

        real_sum = a0 * c0 - b0 * d0;
        imag_sum = b0 * c0 + a0 * d0;
    }

    *realResult = real_sum;
    *imagResult = imag_sum;
}

void uiv_mean_f32(const uiv_f32_t *pSrc, uint32_t block_size, uiv_f32_t *mean) {
    uint32_t i;
    uiv_f32_t sum = 0;
    for (i = 0; i < block_size; i++) { sum += pSrc[i]; }
    *mean = sum / block_size;
}

void uiv_cmplx_mag_squared_f32(const uiv_f32_t *pSrc, uiv_f32_t *pDst, uint32_t numSamples) {
    uint32_t n;
    for (n = 0; n < numSamples; n++) {
        pDst[n] = pSrc[(2 * n) + 0] * pSrc[(2 * n) + 0];
        pDst[n] += pSrc[(2 * n) + 1] * pSrc[(2 * n) + 1];
    }
}

void uiv_cmplx_mag_f32(const uiv_f32_t *pSrc, uiv_f32_t *pDst, uint32_t numSamples) {
    uint32_t n;
    uiv_f32_t square;
    for (n = 0; n < numSamples; n++) {
        square = pSrc[(2 * n) + 0] * pSrc[(2 * n) + 0];
        square += pSrc[(2 * n) + 1] * pSrc[(2 * n) + 1];
        pDst[n] = sqrtf(square);
    }
}

void uiv_max_f32(const uiv_f32_t *buf, uint32_t size, uiv_f32_t *max_value, uint32_t *max_idx) {
    uint32_t idx = 0;
    uint32_t maxIdx_ = 0;
    uiv_f32_t maxValue_ = buf[0];

    for (idx = 1; idx < size; idx++) {
        if (buf[idx] > maxValue_) {
            maxValue_ = buf[idx];
            maxIdx_ = idx;
        }
    }
    *max_value = maxValue_;
    *max_idx = maxIdx_;
}
