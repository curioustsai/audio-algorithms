#ifndef __NOISE_REDUCE_H__
#define __NOISE_REDUCE_H__

#include "predefine.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_NBAND	3

typedef struct _NoiseReduce {
	uint32_t sample_rate;
	uint32_t half_fftlen;
	uint32_t speech_frame;
	uint32_t noise_frame;
	uint32_t bPostFilt;

	float snr_thrd_H;
	float snr_thrd_L;
	float g_min_db;
	float g_min;
	float snr_max;

	float* Ypw;	// half_fflen, real
	float* Npw; // half_fflen, real
	float* spp; // half_fflen, real, init: 0.5
	//uint8_t* speech_bin; // half_fflen
	//uint8_t* noise_bin; // half_fflen

	// post filter
	float* last_Ypw;
	float* last_Npw; // half_fflen, real
	float* post_snr; // half_fflen, real
} NoiseReduce;

// int32_t NoiseReduce_QueryMemSize(MemMgr* hMemMgr, uint16_t fftlen, uint8_t bPostFilt);
int32_t NoiseReduce_Init(NoiseReduce* handle, uint32_t sample_rate, uint32_t fftlen, uint32_t bPostFilt);
int32_t NoiseReduce_EstimateNoise(NoiseReduce* handle, float* power, uint32_t frame_cnt, uint32_t cep_vad);
int32_t NoiseReduce_SnrVAD(NoiseReduce* handle);
int32_t NoiseReduce_WienerFilter(NoiseReduce* handle, float* input, float* output);
int32_t NoiseReduce_Release(NoiseReduce* handle);

#ifdef __cplusplus
}
#endif 

#endif // __NOISE_REDUCE_H__
