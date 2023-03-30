
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
// #define abs(a) (((a) > 0) ? (a) : (-a))
// #define fabsf(a) (((a) > 0) ? (a) : (-a))

#ifdef __CMSIS_DSP__
#include "arm_math.h"

#define uiv_copy_q15 arm_copy_q15
#define uiv_copy_q31 arm_copy_q31
#define uiv_copy_f32 arm_copy_f32

#define uiv_fill_q15 arm_fill_q15
#define uiv_fill_q31 arm_fill_q31
#define uiv_fill_f32 arm_fill_f32

#define uiv_add_q15 arm_add_q15
#define uiv_add_q31 arm_add_q31
#define uiv_add_f32 arm_add_f32

#define uiv_sub_q15 arm_sub_q15
#define uiv_sub_q31 arm_sub_q31
#define uiv_sub_f32 arm_sub_f32

#define uiv_mult_q15 arm_mult_q15
#define uiv_mult_q31 arm_mult_q31
#define uiv_mult_f32 arm_mult_f32

#define uiv_cmplx_dot_prod_f32 arm_cmplx_dot_prod_f32
#define uiv_mean_f32 arm_mean_f32

#define uiv_cmplx_mag_squared_f32 arm_cmplx_mag_squared_f32
#define uiv_cmplx_mag_f32 arm_cmplx_mag_f32
#define uiv_max_f32 arm_max_f32

#else
#include "basic_op.h"

#endif
