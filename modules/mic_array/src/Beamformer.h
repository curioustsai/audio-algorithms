#ifndef __BEAMFORMER_H__
#define __BEAMFORMER_H__

#include <stdint.h>
#include <complex.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _Beamformer {
    uint32_t fn;
    uint32_t speech_cnt;
    uint32_t noise_cnt;
    uint32_t half_fftlen;
    uint32_t nchannel;

    _Complex float* speechRyy; // half_fftlen, nchannel*nchannel
    _Complex float* noiseRvv;  // half_fftlen, nchannel*nchannel
    _Complex float* bf_coef;   // half_fftlen, nchannel
	_Complex float* steering;
} Beamformer;


// Beamformer
int32_t Beamformer_Init(Beamformer* handle, uint32_t fftlen, uint32_t nchannel);
void Beamformer_UpdateSteeringVector(Beamformer *handle, uint32_t update_speech, uint32_t update_noise);
uint32_t Beamformer_UpdateSpeechMatrix(Beamformer* handle, _Complex float* X_itr, uint32_t speech_status);
uint32_t Beamformer_UpdateNoiseMatrix(Beamformer* handle, _Complex float* X_itr, uint32_t noise_status,
                                     float* spp);
int32_t Beamformer_UpdateMvdrFilter(Beamformer* handle, uint32_t update_speech,
                                    uint32_t update_noise);
int32_t Beamformer_DoFilter(Beamformer* handle, _Complex float* X_itr, _Complex float* beamformed);
int32_t Beamformer_Release(Beamformer* handle);

#ifdef __cplusplus
}
#endif 

#endif // __BEAMFORMER_H__
