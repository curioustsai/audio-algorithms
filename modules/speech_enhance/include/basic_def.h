#ifndef __BAISC_DEF_H__
#define __BAISC_DEF_H__

#define STATUS_SUCCESS 0
#define STATUS_FAIL -1

#ifndef NULL
#define NULL 0
#endif 

#include <stdint.h>


#ifdef FIXED_POINT
typedef int16_t uiv_q15_t;
typedef int32_t uiv_q31_t;
typedef float uiv_f32_t;
// #elif defined (FLOATING_POINT)
#else
typedef float uiv_q15_t;
typedef float uiv_q31_t;
typedef float uiv_f32_t;

#endif

#endif // __BAISC_DEF_H__
