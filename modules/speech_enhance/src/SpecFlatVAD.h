#ifndef __SPECFLAT_VAD_H__
#define __SPECFLAT_VAD_H__

#include "basic_def.h"
#include "basic_op.h"

typedef struct _SpecFlatVAD {
	float thrd_1kHz;
	float thrd_4kHz;

	uint16_t bin_100Hz;
	uint16_t bin_1kHz;
	uint16_t bin_2kHz;
	uint16_t bin_3kHz;
	uint16_t bin_4kHz;
	//uint16_t bin_8kHz;

	uint8_t vad;
	float specflat_1kHz;
	float specflat_2kHz;
	float specflat_4kHz;
	//float specflat_8kHz;
} SpecFlatVAD;

int32_t SpecFlatVAD_Init(SpecFlatVAD* handle, uint16_t fftlen, uint16_t sample_rate);
uint8_t SpecFlatVAD_Process(SpecFlatVAD* handle, float* input);
int32_t SpecFlatVAD_Release(SpecFlatVAD* handle);

#endif // __SPECFLAT_VAD_H__
