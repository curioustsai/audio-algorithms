#ifndef __BIQUAD_H__
#define __BIQUAD_H__

#ifdef __cplusplus
extern "C" {
#endif 

// #include <stdint.h>
// typedef int16_t Word16;
// typedef int32_t Word32;
// typedef int64_t Word64;

// typedef struct _BiquiadFilter {
// 	Word16 coef[5];  // b0, b1, b2, a1, a2
// 	Word16 state[2];
// } BiquadFilter;

typedef struct _BiquiadFilter {
	float coef[5];  // b0, b1, b2, a1, a2
	float state[2];
} BiquadFilter;
		
void Biquad_Init(BiquadFilter *handle);
void Biquad_Process(BiquadFilter *bqfilter, float *input, float *output, int num);

#ifdef __cplusplus
}
#endif 

#endif // __BIQUAD_H__

