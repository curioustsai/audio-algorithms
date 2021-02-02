#ifndef __BASIC_OP_H__
#define __BASIC_OP_H__

#include "stdint.h"
#include "math.h"
/* use sqrt in math */

/* arm_cmplx_dot_prod_f32 */

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
// #define abs(a) (((a) > 0) ? (a) : (-a))
#define fabsf(a) (((a) > 0) ? (a) : (-a))

/**
  @brief         Copies the elements of a f16 vector.
  @param[in]     pSrc       points to input vector
  @param[out]    pDst       points to output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */
void uiv_copy_q15(const int16_t *pSrc, int16_t *pDst, uint32_t block_size);
void uiv_copy_q31(const int32_t *pSrc, int32_t *pDst, uint32_t block_size);
void uiv_copy_f32(const float *pSrc, float *pDst, uint32_t block_size);

/*
  @brief         Fills a constant value into a f16 vector.
  @param[in]     value      input value to be filled
  @param[out]    pDst       points to output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */
void uiv_fill_q15(const int16_t value, int16_t *buf, uint32_t block_size);
void uiv_fill_q31(const int32_t value, int32_t *buf, uint32_t block_size);
void uiv_fill_f32(const float value, float *buf, uint32_t block_size);

/**
  @brief         Floating-point vector addition.
  @param[in]     pSrcA      points to first input vector
  @param[in]     pSrcB      points to second input vector
  @param[out]    pDst       points to output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */

void uiv_add_q15(const int16_t *pSrcA, const int16_t *pSrcB, int16_t *pDst, uint32_t block_size);
void uiv_add_q31(const int32_t *pSrcA, const int32_t *pSrcB, int32_t *pDst, uint32_t block_size);
void uiv_add_f32(const float *pSrcA, const float *pSrcB, float *pDst, uint32_t block_size);

/**
  @brief         Floating-point vector subtraction.
  @param[in]     pSrcA      points to the first input vector
  @param[in]     pSrcB      points to the second input vector
  @param[out]    pDst       points to the output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none
 */
void uiv_sub_q15(const int16_t *pSrcA, const int16_t *pSrcB, int16_t *pDst, uint32_t block_size);
void uiv_sub_q31(const int32_t *pSrcA, const int32_t *pSrcB, int32_t *pDst, uint32_t block_size);
void uiv_sub_f32(const float *pSrcA, const float *pSrcB, float *pDst, uint32_t block_size);

/**
  @brief         Floating-point vector multiplication.
  @param[in]     pSrcA      points to the first input vector.
  @param[in]     pSrcB      points to the second input vector.
  @param[out]    pDst       points to the output vector.
  @param[in]     blockSize  number of samples in each vector.
  @return        none
 */

void uiv_mult_q15(const int16_t *pSrcA, const int16_t *pSrcB, int16_t *pDst, uint32_t block_size);
void uiv_mult_q31(const int32_t *pSrcA, const int32_t *pSrcB, int32_t *pDst, uint32_t block_size);
void uiv_mult_f32(const float *pSrcA, const float *pSrcB, float *pDst, uint32_t block_size);

void uiv_cmplx_dot_prod_f32(const int32_t *pSrcA, const int32_t *pSrcB, uint32_t numSamples,
                            int32_t *realResult, int32_t *imagResult);

void uiv_mean_f32(const float *pSrc, uint32_t block_size, float *mean);

void uiv_cmplx_mag_squared_f32( const float * pSrc, float * pDst, uint32_t numSamples);
void uiv_cmplx_mag_f32( const float * pSrc, float * pDst, uint32_t numSamples);
void uiv_max_f32(const float *buf, uint32_t size, float * max_value , uint32_t *max_idx);

#endif // __BASIC_OP_H__

