#ifndef __SPEECH_ENHANCE_H__
#define __SPEECH_ENHANCE_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "basic_def.h"

int32_t SpeechEnhance_Init(void** pHandle, uint16_t sample_rate, uint16_t nchannel, uint16_t fftlen, uint16_t nframe);
int32_t SpeechEnhance_Process(void* handle, int16_t* mic_inputs, int16_t* output);
int32_t SpeechEnhance_Release(void* handle);

#ifdef __cplusplus
}
#endif 

#endif // __SPEECH_ENHANCE_H__
