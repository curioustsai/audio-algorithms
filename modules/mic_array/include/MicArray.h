#ifndef __MIC_ARRAY_H__
#define __MIC_ARRAY_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

int32_t MicArray_Init(void** pHandle, uint32_t sample_rate, uint32_t nchannel, uint32_t fftlen, uint32_t nframe);
int32_t MicArray_Process(void* handle, int16_t* mic_inputs, int16_t* output);
int32_t MicArray_Release(void* handle);

#ifdef __cplusplus
}
#endif 

#endif // __MIC_ARRAY_H__
