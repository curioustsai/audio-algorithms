#ifndef __SPECFLAT_VAD_H__
#define __SPECFLAT_VAD_H__

#include <stdint.h>

typedef struct _SpecFlatVAD {
	float thrd_1kHz;
	float thrd_4kHz;

	uint32_t bin_100Hz;
	uint32_t bin_1kHz;
	uint32_t bin_2kHz;
	uint32_t bin_3kHz;
	uint32_t bin_4kHz;
	//uint16_t bin_8kHz;

	uint32_t vad;
	float specflat_1kHz;
	float specflat_2kHz;
	float specflat_4kHz;
	//float specflat_8kHz;
} SpecFlatVAD;

int32_t SpecFlatVAD_Init(SpecFlatVAD* handle, uint32_t fftlen, uint32_t sample_rate);
uint32_t SpecFlatVAD_Process(SpecFlatVAD* handle, float* input);
int32_t SpecFlatVAD_Release(SpecFlatVAD* handle);

#endif // __SPECFLAT_VAD_H__
