#include "predefine.h"
#include "SpecFlatVAD.h"


int32_t SpecFlatVAD_Init(SpecFlatVAD* handle, uint32_t fftlen, uint32_t sample_rate)
{
	handle->thrd_1kHz = 0.3f;
	handle->thrd_4kHz = 0.4f;

	handle->specflat_1kHz = 0.f;
	handle->specflat_2kHz = 0.f;
	handle->specflat_4kHz = 0.f;
	//handle->specflat_8kHz = 0.f;

	handle->bin_100Hz = 100 * fftlen / sample_rate;
	handle->bin_1kHz = 1000 * fftlen / sample_rate;
	handle->bin_2kHz = 2000 * fftlen / sample_rate;
	handle->bin_4kHz = 4000 * fftlen / sample_rate;
	//handle->bin_8kHz = 8000 * fftlen / sample_rate;

	handle->vad = 0;

	return 0;
}

int32_t SpecFlatVAD_Release(SpecFlatVAD* handle)
{
	return 0;
}

uint32_t SpecFlatVAD_Process(SpecFlatVAD* handle, float* ref_pow)
{
	uint16_t idx;
	float alpha = 0.3f;
	float spflt_num, spflt_den, spflt;
	uint16_t spflt_len;

	spflt_num = 0.f;
	spflt_den = 0.f;
	for (idx = handle->bin_100Hz; idx < handle->bin_1kHz; ++idx)
	{
		spflt_num += logf(ref_pow[idx]);
		spflt_den += ref_pow[idx];
	}

	spflt_len = handle->bin_1kHz - handle->bin_100Hz;
	spflt_num /= spflt_len;
	spflt_den /= spflt_len;
	spflt = expf(spflt_num) / (spflt_den + 1e-12f);
	handle->specflat_1kHz += alpha * (spflt - handle->specflat_1kHz);

	spflt_num = 0.f;
	spflt_den = 0.f;
	for (idx = handle->bin_2kHz; idx < handle->bin_4kHz; ++idx)
	{
		spflt_num += logf(ref_pow[idx]);
		spflt_den += ref_pow[idx];
	}
	spflt_len = handle->bin_4kHz - handle->bin_2kHz;
	spflt_num /= spflt_len;
	spflt_den /= spflt_len;
	spflt = expf(spflt_num) / (spflt_den + 1e-12f);
	handle->specflat_4kHz += alpha * (spflt - handle->specflat_4kHz);

	handle->vad = 0;
	if (handle->specflat_1kHz < handle->thrd_1kHz && 
		handle->specflat_4kHz < handle->thrd_4kHz)
		handle->vad = 1;

	return handle->vad;
}
