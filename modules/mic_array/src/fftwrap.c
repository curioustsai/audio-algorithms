#include "fftwrap.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _FFTW_
#include <fftw3.h>

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

void uiv_fft(void *table, float *in, float *out) {
    int i;
    struct fftw_config *t = (struct fftw_config *)table;
    const int N = t->N;
    float *iptr = t->in;
    float *optr = t->out;

    for (i = 0; i < N; ++i) iptr[i] = in[i];

    /*
     * fftw: 
     * input real value of length N
     * output complex value of length (N/2+1) * 2, length in real value equals to N + 2
     * |bin 0, real| bin 0, imag | bin 1, real| bin 1, image| ... | bin N/2 + 1 real |, bin N/2 + 1 imag|
     */

    fftwf_execute(t->fft);

    for (i = 0; i < N + 2; ++i) out[i] = optr[i];
}

void uiv_ifft(void *table, float *in, float *out) {
    int i;
    struct fftw_config *t = (struct fftw_config *)table;
    const int N = t->N;
    float *iptr = t->in;
    float *optr = t->out;
    const float m = 1.0 / N;

    for (i = 0; i < N + 2; ++i) iptr[i] = in[i];

    fftwf_execute(t->ifft);
    for (i = 0; i < N; ++i) out[i] = optr[i] * m;
}

#elif __CMSIS_DSP__
#include "arm_math.h"
#include <assert.h>

typedef struct cmsisfft_config_ {
    float *in;
    float *out;
    arm_rfft_fast_instance_f32 *inst;
    int N;
} cmsisfft_config;

void *uiv_fft_init(int size) {
    cmsisfft_config *config = (cmsisfft_config *)malloc(sizeof(cmsisfft_config));
    config->inst = (arm_rfft_fast_instance_f32 *)malloc(sizeof(arm_rfft_fast_instance_f32));
    arm_status ret = arm_rfft_fast_init_f32(config->inst, (uint16_t)size);

    assert(ret == ARM_MATH_SUCCESS);

    config->in = (float *)malloc(sizeof(float) * size);
    config->out = (float *)malloc(sizeof(float) * (size + 2));
    config->N = size;

    return config;
}

void uiv_fft_destroy(void *config) {
    cmsisfft_config *handle = (cmsisfft_config *)config;
    free(handle->in);
    free(handle->out);

    free(config);
}

void uiv_fft(void *config, float *in, float *out) {
    cmsisfft_config *handle = (cmsisfft_config *)config;

    for (int i = 0; i < handle->N; ++i) { handle->in[i] = in[i]; }
    arm_rfft_fast_f32(handle->inst, handle->in, handle->out, 0);

    /*
     * rfft: 
     * input: real value of length N
     * output: complex value of length (N/2) * 2, length in real value equals to N 
     * |bin 0, real| bin N-1, real | bin 1, real| bin 1, image| ... | bin N/2 real |, bin N/2 imag|
     *
     * fft value bin 0 and N-1 is always real. They are combined at the first 2 real value.
     * grow up by a factor of fftLen
     */
    out[0] = handle->out[0];
    out[1] = 0;
    for (int i = 2; i < handle->N; ++i) { out[i] = handle->out[i]; }
    out[handle->N] = handle->out[1];
    out[handle->N + 1] = 0;
}

void uiv_ifft(void *config, float *in, float *out) {
    // scale down by a factor of 1 / fftLen
    cmsisfft_config *handle = (cmsisfft_config *)config;

    handle->in[0] = in[0];
    handle->in[1] = in[handle->N];
    for (int i = 2; i < handle->N; ++i) { handle->in[i] = in[i]; }
    arm_rfft_fast_f32(handle->inst, handle->in, handle->out, 1);
    for (int i = 0; i < handle->N; ++i) { out[i] = handle->out[i]; }
}

#else
#include "pffft.h"

typedef struct pffft_config_ {
    float *in;
    float *out;
    PFFFT_Setup *setup;
    int N;
} pffft_config;

void *uiv_fft_init(int size) {
    pffft_config *config = (pffft_config *)malloc(sizeof(pffft_config));
    config->setup = pffft_new_setup(size, PFFFT_REAL);
    config->in = (float *)malloc(sizeof(float) * size);
    config->out = (float *)malloc(sizeof(float) * (size + 2));
    config->N = size;

    return config;
}

void uiv_fft_destroy(void *config) {
    pffft_config *handle = (pffft_config *)config;
    free(handle->in);
    free(handle->out);
    pffft_destroy_setup(handle->setup);

    free(config);
}

void uiv_fft(void *config, float *in, float *out) {
    pffft_config *handle = (pffft_config *)config;

    for (int i = 0; i < handle->N; ++i) { handle->in[i] = in[i]; }
    pffft_transform_ordered(handle->setup, handle->in, handle->out, NULL, PFFFT_FORWARD);

    /*
     * pffft: 
     * input: real value of length N
     * output: complex value of length (N/2) * 2, length in real value equals to N 
     * |bin 0, real| bin N-1, real | bin 1, real| bin 1, image| ... | bin N/2 real |, bin N/2 imag|
     *
     * fft value bin 0 and N-1 is always real. They are combined at the first 2 real value.
     */
    out[0] = handle->out[0];
    out[1] = 0;
    for (int i = 2; i < handle->N; ++i) { out[i] = handle->out[i]; }
    out[handle->N] = handle->out[1];
    out[handle->N + 1] = 0;
}

void uiv_ifft(void *config, float *in, float *out) {
    pffft_config *handle = (pffft_config *)config;

    handle->in[0] = in[0];
    handle->in[1] = in[handle->N];
    for (int i = 2; i < handle->N; ++i) { handle->in[i] = in[i]; }
    pffft_transform_ordered(handle->setup, handle->in, handle->out, NULL, PFFFT_BACKWARD);

    float inv_N = 1.0f / handle->N;
    for (int i = 0; i < handle->N; ++i) { out[i] = handle->out[i] * inv_N; }
}

#endif
