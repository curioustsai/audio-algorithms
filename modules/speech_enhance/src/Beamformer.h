#ifndef __BEAMFORMER_H__
#define __BEAMFORMER_H__

#include "basic_def.h"
#include "basic_op.h"
#include "complex.h"

typedef struct _Beamformer {
    uint32_t fn;
    uint32_t speech_cnt;
    uint32_t noise_cnt;
    uint32_t half_fftlen;
    uint32_t nchannel;

    complex float* speechRyy; // half_fftlen, nchannel*nchannel
    complex float* noiseRvv;  // half_fftlen, nchannel*nchannel
    complex float* bf_coef;   // half_fftlen, nchannel
	complex float* steering;
} Beamformer;

// Beamformer
int32_t Beamformer_Init(Beamformer* handle, uint32_t fftlen, uint32_t nchannel);
void Beamformer_UpdateSteeringVector(Beamformer *handle, uint8_t update_speech, uint8_t update_noise);
uint8_t Beamformer_UpdateSpeechMatrix(Beamformer* handle, complex float* X_itr, uint8_t speech_status);
uint8_t Beamformer_UpdateNoiseMatrix(Beamformer* handle, complex float* X_itr, uint8_t noise_status,
                                     float* spp);
int32_t Beamformer_UpdateMvdrFilter(Beamformer* handle, uint8_t update_speech,
                                    uint8_t update_noise);
int32_t Beamformer_UpdateMwfFilter(Beamformer* handle, uint8_t update_speech, uint8_t update_noise);
int32_t Beamformer_DoFilter(Beamformer* handle, complex float* X_itr, complex float* beamformed);
int32_t Beamformer_Release(Beamformer* handle);

#endif // __BEAMFORMER_H__
