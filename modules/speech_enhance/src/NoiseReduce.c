#include "NoiseReduce.h"
#include "fftwrap.h"
#include "basic_op.h"
#include <stdlib.h>

/**
 * Implementation of "Unbiased MMSE-Based Noise Power Estimation With Low Complexity and Low Tracking Delay"
 */
int32_t NoiseReduce_Init(NoiseReduce* handle, uint32_t sample_rate, uint32_t fftlen,
                         uint32_t bPostFilt) {
    uint32_t half_fftlen = uiv_half_fftlen(fftlen);

    handle->sample_rate = sample_rate;
    handle->half_fftlen = half_fftlen;
    handle->bPostFilt = bPostFilt;

    handle->snr_thrd_H = 7.0f;
    handle->snr_thrd_L = 4.0f;

    //handle->g_min_db = -6.0f;
    //handle->g_min = powf(10.f, handle->g_min_db / 20.0f);
    handle->g_min = 0.501187f;

    handle->Ypw = (float*)calloc(half_fftlen, sizeof(float));
    handle->Npw = (float*)calloc(half_fftlen, sizeof(float));
    handle->spp = (float*)calloc(half_fftlen, sizeof(float));

    /* handle->speech_bin = (uint32_t*)calloc(half_fftlen, sizeof(uint32_t)); */
    /* handle->noise_bin = (uint32_t*)calloc(half_fftlen, sizeof(uint32_t)); */

    if (handle->bPostFilt) {
        handle->last_Ypw = (float*)calloc(half_fftlen, sizeof(float));
        handle->last_Npw = (float*)calloc(half_fftlen, sizeof(float));
        handle->post_snr = (float*)calloc(half_fftlen, sizeof(float));
    }

    return 0;
}

int32_t NoiseReduce_Release(NoiseReduce* handle) {
    free(handle->Ypw);
    free(handle->Npw);
    free(handle->spp);
    //handle->speech_bin = NULL;
    //handle->noise_bin = NULL;

    if (handle->bPostFilt) {
        free(handle->last_Ypw);
        free(handle->last_Npw);
        free(handle->post_snr);
    }

    return 0;
}

int32_t NoiseReduce_EstimateNoise(NoiseReduce* handle, float* ref_power, uint32_t frame_cnt,
                                  uint32_t cep_vad) {
    uint32_t idx;
    uint32_t half_fftlen = handle->half_fftlen;

    const float ax = 0.8f;
    const float axc = 1 - ax; // noise output smoothing time constant(8)
    const float ap = 0.9f;
    const float apc = 1 - ap;  // speech prob smoothing time constant(23)
    const float psthr = 0.99f; // threshold for smoothed speech probability [0.99] (24)
    const float pnsaf = 0.01f; // noise probability [0.01] (24)
    const float pspri = 0.5f;  // prior speech probabilty [0.5] (18)
    //const float asnr = 15.0f;			// active SNR in dB [15] (18)
    //const float psini = 0.5f;			// initial speech probability [0.5] (23)
    //const float tavini = 0.064f * 3.0f;		// assumed speech absent time at start [64 ms]

    const float xih1 = 31.6227766017f; // speech - present SNR at asnr = 15dB i.e. 10 ^ (asnr / 10)
    const float xih1r = xih1 / (1.0f + xih1);
    const float pfac = ((1.0f - pspri) / pspri) * (1.0f + xih1); // p(noise) / p(speech) (18)

    float ph1y, noise_r, ax_vad;
    float temp;

    for (idx = 0; idx < half_fftlen; ++idx)
        handle->Ypw[idx] += 0.4f * (ref_power[idx] - handle->Ypw[idx]);

    if (handle->bPostFilt) uiv_copy_f32(handle->Npw, handle->last_Npw, half_fftlen);

    if (frame_cnt < 16) {
        for (idx = 0; idx < half_fftlen; ++idx)
            handle->Npw[idx] += 0.3f * (ref_power[idx] - handle->Npw[idx]);
    } else {
        for (idx = 0; idx < half_fftlen; ++idx) {
            // a-posterior speech presence prob (18)
            temp = ref_power[idx] / (handle->Npw[idx] + 1e-16f);
            temp = -1.0f * xih1r * temp;
            temp = expf(temp);
            ph1y = 1.0f / (1.0f + pfac * temp);

            // smoothed sppech presence prob (23)
            handle->spp[idx] += apc * (ph1y - handle->spp[idx]);

            // limit ph1y (24), safe-net
            if (handle->spp[idx] > psthr)
                ph1y = min(ph1y, 1.f - pnsaf);
            else
                ph1y = min(ph1y, 1.f);

            // estimated raw noise spectrum (22)
            noise_r = (1.0f - ph1y) * ref_power[idx] + ph1y * handle->Npw[idx];

            // smooth the noise estimate (8)
            if (cep_vad == 0)
                handle->Npw[idx] += axc * (noise_r - handle->Npw[idx]);
            else {
                ax_vad = (handle->Npw[idx] < ref_power[idx]) ? 0.1f : 0.2f;
                handle->Npw[idx] += ax_vad * (noise_r - handle->Npw[idx]);
            }
        }
    }

    return 0;
}

int32_t NoiseReduce_SnrVAD(NoiseReduce* handle) {
    uint32_t idx;
    uint32_t sample_rate = handle->sample_rate;
    uint32_t fftlen = (handle->half_fftlen) << 1;
    uint32_t half_fftlen = handle->half_fftlen;

    uint32_t f1 = (uint32_t)(50.f * fftlen / sample_rate);
    uint32_t f2 = (uint32_t)(1000.f * fftlen / sample_rate);
    uint32_t f3 = (uint32_t)(2000.f * fftlen / sample_rate);

    float snr_thrd_H = handle->snr_thrd_H;
    float snr_thrd_L = handle->snr_thrd_L;

    float mag_band[MAX_NBAND], noise_band[MAX_NBAND], snr_band[MAX_NBAND];
    float sum_Ypw, sum_Npw;
    float mag_db, noise_db, snr_db;

    // Band0
    for (sum_Ypw = 0.f, sum_Npw = 0.f, idx = f1; idx < f3; ++idx) {
        sum_Ypw += (handle->Ypw[idx]);
        sum_Npw += (handle->Npw[idx]);
    }
    mag_band[0] = 10.f * log10f(sum_Ypw + 1.0f);
    noise_band[0] = 10.f * log10f(sum_Npw + 1.0f);
    snr_band[0] = mag_band[0] - noise_band[0];

    // Band1
    for (sum_Ypw = 0.f, sum_Npw = 0.f, idx = f2; idx < half_fftlen; ++idx) {
        sum_Ypw += (handle->Ypw[idx]);
        sum_Npw += (handle->Npw[idx]);
    }
    mag_band[1] = 10.f * log10f(sum_Ypw + 1.0f);
    noise_band[1] = 10.f * log10f(sum_Npw + 1.0f);
    snr_band[1] = mag_band[1] - noise_band[1];

    // Band All
    for (sum_Ypw = 0.f, sum_Npw = 0.f, idx = 0; idx < half_fftlen; ++idx) {
        sum_Ypw += (handle->Ypw[idx]);
        sum_Npw += (handle->Npw[idx]);
    }
    mag_band[2] = 10.f * log10f(sum_Ypw + 1.0f);
    noise_band[2] = 10.f * log10f(sum_Npw + 1.0f);
    snr_band[2] = mag_band[1] - noise_band[1];

    handle->snr_max = max(snr_band[1], snr_band[2]);

    handle->speech_frame = 0;
    if (snr_band[0] > snr_thrd_H || snr_band[1] > snr_thrd_H || snr_band[2] > snr_thrd_H)
        handle->speech_frame = 1;

    handle->noise_frame = 1;
    if (snr_band[0] > snr_thrd_L || snr_band[1] > snr_thrd_L || snr_band[2] > snr_thrd_L)
        handle->noise_frame = 0;

    for (idx = 0; idx < half_fftlen; ++idx) {
        mag_db = 10.f * log10f(handle->Ypw[idx] + 1.0f);
        noise_db = 10.f * log10f(handle->Npw[idx] + 1.0f);
        snr_db = mag_db - noise_db;

        //handle->speech_bin[idx] = snr_db > snr_thrd_H;
        //handle->noise_bin[idx] = snr_db < snr_thrd_L;

        if (handle->bPostFilt) handle->post_snr[idx] = snr_db;
    }

    return 0;
}

int32_t NoiseReduce_WienerFilter(NoiseReduce* handle, float* input, float* output) {
    float alpha = 0.9f; // range from [0.9, 0.98]
    float g_min = handle->g_min;
    float prior_snr, snr, gain;

    uint32_t idx;
    uint32_t half_fftlen = handle->half_fftlen;

    for (idx = 0; idx < half_fftlen; ++idx) {
        prior_snr = handle->last_Ypw[idx] / (handle->last_Npw[idx] + 1e-12f);
        snr = alpha * prior_snr + (1 - alpha) * max(handle->post_snr[idx] - 1.0f, 0.0f);
        gain = snr / (snr + 1.0f);
        gain = max(gain, g_min);

        output[2 * idx] = gain * input[2 * idx];
        output[2 * idx + 1] = gain * input[2 * idx + 1];

        handle->last_Ypw[idx] =
            (output[2 * idx] * output[2 * idx] + output[2 * idx + 1] * output[2 * idx + 1]);
    }

    return 0;
}
