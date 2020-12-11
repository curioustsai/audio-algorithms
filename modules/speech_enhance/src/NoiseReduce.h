#ifndef __NOISE_REDUCE_H__
#define __NOISE_REDUCE_H__

#include "basic_def.h"
#include "basic_op.h"

#define MAX_NBAND	3

typedef struct _NoiseReduce {
	uint16_t sample_rate;
	uint16_t half_fftlen;
	uint8_t speech_frame;
	uint8_t noise_frame;
	uint8_t bPostFilt;

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
int32_t NoiseReduce_Init(NoiseReduce* handle, uint16_t sample_rate, uint16_t fftlen, uint8_t bPostFilt);
int32_t NoiseReduce_EstimateNoise(NoiseReduce* handle, float* power, uint32_t frame_cnt, uint8_t cep_vad);
int32_t NoiseReduce_SnrVAD(NoiseReduce* handle);
int32_t NoiseReduce_WienerFilter(NoiseReduce* handle, float* input, float* output);
int32_t NoiseReduce_Release(NoiseReduce* handle);

#endif // __NOISE_REDUCE_H__
