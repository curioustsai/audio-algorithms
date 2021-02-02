#ifndef __SPEECH_ENHANCE_H__
#define __SPEECH_ENHANCE_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

int32_t SpeechEnhance_Init(void** pHandle, uint32_t sample_rate, uint32_t nchannel, uint32_t fftlen, uint32_t nframe);
int32_t SpeechEnhance_Process(void* handle, int16_t* mic_inputs, int16_t* output);
int32_t SpeechEnhance_Release(void* handle);

#ifdef __cplusplus
}
#endif 

#endif // __SPEECH_ENHANCE_H__
