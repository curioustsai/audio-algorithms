#ifndef __CEPSTRUM_VAD_H__
#define __CEPSTRUM_VAD_H__

#include "predefine.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _CepstrumVAD {
    uint32_t fftlen;
    uint32_t cepfreq_up;
    uint32_t cepfreq_dn;

    uint32_t pitchBufLen;
    uint32_t cepIdxLen;
    uint32_t pitch;
    uint32_t vad;

    float* cepData_sm;
    int16_t* cepIdxBuf;
    int16_t* pitchBuf;

    float* xpow;			// half_fftlen * 2
    float* cepDataAll;		// fftlen
    float* cepData_sm_max;	// cep_size

    void* fft_lookup;
} CepstrumVAD;

int32_t CepstrumVAD_Init(CepstrumVAD* handle, uint32_t fftlen, uint32_t sample_rate);
uint32_t CepstrumVAD_Process(CepstrumVAD* handle, float* input);
int32_t CepstrumVAD_Release(CepstrumVAD* handle);

#ifdef __cplusplus
}
#endif 

#endif
