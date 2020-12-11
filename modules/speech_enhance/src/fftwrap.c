#include "fftwrap.h"
#include <fftw3.h>
#include <stdlib.h>

struct fftw_config {
    float *in;
    float *out;
    fftwf_plan fft;
    fftwf_plan ifft;
    int N;
};

/**
 * initialize fft
 * @param size: fft size
 * @return handle pointer
 */

void *uiv_fft_init(int size) {
    struct fftw_config *table = (struct fftw_config *)malloc(sizeof(struct fftw_config));
    table->in = fftwf_malloc(sizeof(float) * (size + 2));
    table->out = fftwf_malloc(sizeof(float) * (size + 2));

    table->fft = fftwf_plan_dft_r2c_1d(size, table->in, (fftwf_complex *)table->out, FFTW_PATIENT);
    table->ifft = fftwf_plan_dft_c2r_1d(size, (fftwf_complex *)table->in, table->out, FFTW_PATIENT);

    table->N = size;
    return table;
}

void uiv_fft_destroy(void *table) {
    struct fftw_config *t = (struct fftw_config *)table;
    fftwf_destroy_plan(t->fft);
    fftwf_destroy_plan(t->ifft);
    fftwf_free(t->in);
    fftwf_free(t->out);
    free(table);
}

void uiv_fft(void *table, uiv_f32_t *in, uiv_f32_t *out) {
    int i;
    struct fftw_config *t = (struct fftw_config *)table;
    const int N = t->N;
    float *iptr = t->in;
    float *optr = t->out;

    for (i = 0; i < N; ++i) iptr[i] = in[i];

    fftwf_execute(t->fft);

    for (i = 0; i < N+2; ++i) out[i] = optr[i];
}

void uiv_ifft(void *table, uiv_f32_t *in, uiv_f32_t *out) {
    int i;
    struct fftw_config *t = (struct fftw_config *)table;
    const int N = t->N;
    float *iptr = t->in;
    float *optr = t->out;
    const float m = 1.0 / N;

    for (i = 0; i < N+2; ++i) iptr[i] = in[i];

    fftwf_execute(t->ifft);

    for (i = 0; i < N; ++i) out[i] = optr[i] * m;
}

uint32_t uiv_half_fftlen(uint32_t fftlen) {
	return (fftlen >> 1U) + 1U;
}
