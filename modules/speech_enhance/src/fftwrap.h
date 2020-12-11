#ifndef __FFT_WRAP_H__
#define __FFT_WRAP_H__

#include "basic_def.h"

void *uiv_fft_init(int size);
void uiv_fft_destroy(void *table);

void uiv_fft(void *table, uiv_f32_t *in, uiv_f32_t *out);
void uiv_ifft(void *table, uiv_f32_t *in, uiv_f32_t *out);
uint32_t uiv_half_fftlen(uint32_t fftlen);


#endif // __FFT_WRAP_H__
