#ifndef __SPEECH_ENHANCE_INTERNAL_H__
#define __SPEECH_ENHANCE_INTERNAL_H__

#include "AutoGainCtrl.h"
#include "Beamformer.h"
#include "CepstrumVAD.h"
#include "NoiseReduce.h"
#include "SoundLocater.h"
#include "SpecFlatVAD.h"
#include "Biquad.h"
#include "basic_op.h"
#include "fftwrap.h"

#define MAX_NCHANNEL 4

// #define _FFT_ONLY
#define _DC_REMOVAL
// #define _NS_ENABLE
// #define _AGC_ENABLE

typedef struct _SpeechEnhance {
    CepstrumVAD stCepstrumVAD;
    NoiseReduce stSnrEst;
    Beamformer stBeamformer;
    SoundLocater stSoundLocater;
	BiquadFilter stBiquad;

#ifdef _NS_ENABLE
    NoiseReduce stPostFilt;
#endif
#ifdef _AGC_ENABLE
    AutoGainCtrl stAGC;
#endif

    void* fft_lookup;
    float alpha_dc; // 0.98
    float state_y[MAX_NCHANNEL];
    float state_x[MAX_NCHANNEL];

    float* fftwin; // sizeof(float) * fftlen
    float* inputs_last;  // sizeof(float) * nshift * nchannel
    float* overlap;      // sizeof(float) * nframe

    float* dc_remove;   // sizeof(float) * nframe * nchannel;
    float* inputs_t;    // sizeof(float) * fftlen
    float* inputs_f;    // sizeof(float) * half_fftlen * 2 * nchannel
    float* ref_power;   // sizeof(float) * half_fftlen
    float* X_itr;       // sizeof(float) * half_fftlen * 2 * nchannel
    float* beamformed;  // sizeof(float) * half_fftlen * 2
    float* beamformed_power; // sizeof(float) * half_fftlen 
    float* output_ifft; // sizeof(float) * fftlen
#ifdef _AGC_ENABLE
	float* agc_out;		// sizeof(float) * nframe
#endif

    uint32_t frame_cnt;
    uint32_t sample_rate;
    uint32_t nframe;
    uint32_t nchannel;
    uint32_t nshift;
    uint32_t fftlen;
    uint32_t ref_ch;
} SpeechEnhance;

#endif // __SPEECH_ENHANCE_INTERNAL_H__
