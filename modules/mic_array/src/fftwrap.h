#ifndef __FFT_WRAP_H__
#define __FFT_WRAP_H__

#define uiv_half_fftlen(fftlen) ((fftlen >> 1U) + 1U)

/* 
 * init fft handle
 * @param size: size of fft len (N)
 * @return handle pointer
 */
void *uiv_fft_init(int size);

/* 
 * release fft handle
 * @param table fft handle pointer
 */
void uiv_fft_destroy(void *table);

/* 
 * @param table: handle of fft config
 * @param in: real value of time domain signal, size of N (fftlen).
 * @param out: real value of frequency domain signal, sizeof N + 2 (fftlen + 2)
 *  |bin 0, real| bin 0, imag | bin 1, real| bin 1, image| ... | bin N/2 + 1 real |, bin N/2 + 1 imag|
 */
void uiv_fft(void *table, float *in, float *out);

/* 
 * @param table: handle of fft config
 * @param in: real value of frequency domain signal, sizeof N + 2 (fftlen + 2)
 *  |bin 0, real| bin 0, imag | bin 1, real| bin 1, image| ... | bin N/2 + 1 real |, bin N/2 + 1 imag|
 * @param out: real value of time domain signal, size of N (fftlen).
 */
void uiv_ifft(void *table, float *in, float *out);

#endif // __FFT_WRAP_H__
