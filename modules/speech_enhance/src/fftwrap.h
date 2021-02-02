#ifndef __FFT_WRAP_H__
#define __FFT_WRAP_H__

#define uiv_half_fftlen(fftlen) ((fftlen >> 1U) + 1U)

void *uiv_fft_init(int size);
void uiv_fft_destroy(void *table);

void uiv_fft(void *table, float *in, float *out);
void uiv_ifft(void *table, float *in, float *out);
void uiv_fft_shift(void* table, float *ptr);
void uiv_ifft_shift(void* table, float *ptr);


#endif // __FFT_WRAP_H__
