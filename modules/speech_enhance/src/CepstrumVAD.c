#include "CepstrumVAD.h"
#include "fftwrap.h"
#include <stdlib.h>

int32_t CepstrumVAD_Init(CepstrumVAD* handle, uint16_t fftlen, uint16_t sample_rate) {
    uint32_t cep_size;
    uint32_t half_fftlen = uiv_half_fftlen(fftlen);

    handle->fftlen = fftlen;
    handle->cepfreq_up = sample_rate / 40;
    handle->cepfreq_dn = sample_rate / 600;
    handle->pitchBufLen = 5;
    handle->cepIdxLen = 5;
    handle->pitch = 0;
    handle->fft_lookup = uiv_fft_init(fftlen);

    cep_size = handle->cepfreq_up - handle->cepfreq_dn + 1;

    handle->cepData_sm = (float*)malloc(cep_size * sizeof(float));
    handle->cepIdxBuf = (int16_t*)malloc(handle->cepIdxLen * sizeof(int16_t));
    handle->pitchBuf = (int16_t*)malloc(handle->pitchBufLen * sizeof(int16_t));

    handle->xpow = (float*)malloc(half_fftlen * 2U * sizeof(float));
    handle->cepDataAll = (float*)malloc(fftlen * sizeof(float));
    handle->cepData_sm_max = (float*)malloc(fftlen * sizeof(float));

    for (int i = 0; i < cep_size; i++) { handle->cepData_sm[i] = 0; }
    for (int i = 0; i < handle->cepIdxLen; i++) { handle->cepIdxBuf[i] = 0; }
    for (int i = 0; i < handle->pitchBufLen; i++) { handle->pitchBuf[i] = 0; }

    return STATUS_SUCCESS;
}

int32_t CepstrumVAD_Release(CepstrumVAD* handle) {
    free(handle->cepData_sm);
    free(handle->cepIdxBuf);
    free(handle->pitchBuf);

    free(handle->xpow);
    free(handle->cepDataAll);
    free(handle->cepData_sm_max);

    uiv_fft_destroy(handle->fft_lookup);

    return STATUS_SUCCESS;
}

uint32_t CepstrumVAD_Process(CepstrumVAD* handle, float* ref_pow) {
    uint32_t step = 7;
    uint32_t fftlen = handle->fftlen;
    uint32_t half_fftlen = uiv_half_fftlen(fftlen);
    uint32_t cepfreq_up = handle->cepfreq_up;
    uint32_t cepfreq_dn = handle->cepfreq_dn;
    uint32_t cep_size = cepfreq_up - cepfreq_dn + 1;
    uint32_t idx, idx_c;
    uint32_t idx_dist_sum;
    uint32_t pitch_dist;
    uint32_t max_idx;
    int32_t max_idx_q15;

    float alpha = 0.5f;
    float cepMax_thrd = 0.09f;
    float cepMax_thrdLow = 0.07f;
    float *xpow, *cepData, *cepDataAll, *cepData_sm_max;
    float cepData_sm, cepData_max;

    xpow = handle->xpow;                     // sizeof(float) * half_fftlen *2
    cepDataAll = handle->cepDataAll;         // sizeof(float) * fftlen
    cepData_sm_max = handle->cepData_sm_max; // sizeof(float) * (cepfreq_up-cepfreq_dn+1)

    for (idx = 0; idx < half_fftlen; ++idx) {
        xpow[2 * idx] = log10f(ref_pow[idx]);
        xpow[2 * idx + 1] = 0;
    }

    uiv_ifft(handle->fft_lookup, xpow, cepDataAll);
    cepData = &cepDataAll[cepfreq_dn - 1 - step]; // range from [cepfreq_dn-1-step, cepfreq_up+step]
    for (int i = 0; i < cep_size; i++) { cepData_sm_max[i] = 0.0f; }

    for (idx = 0; idx < 2 * step; ++idx) {
        for (idx_c = 0; idx_c < cep_size; ++idx_c) {
            cepData_sm = alpha * handle->cepData_sm[idx_c] + (1 - alpha) * cepData[idx + idx_c];
            cepData_sm_max[idx_c] = max(cepData_sm_max[idx_c], cepData_sm);
        }
    }

    for (int i = 0; i < cep_size; i++) { handle->cepData_sm[i] = cepData_sm_max[i]; }
    uiv_max_f32(handle->cepData_sm, cep_size, &cepData_max, &max_idx);
    max_idx_q15 = (uiv_q15_t)max_idx;

    idx_dist_sum = abs(max_idx_q15 - handle->cepIdxBuf[idx]) < 5;
    for (idx = 1; idx < handle->cepIdxLen; ++idx) {
        idx_dist_sum += abs(max_idx_q15 - handle->cepIdxBuf[idx]) < 5;
    }

    for (int i = 1; i < handle->cepIdxLen; i++) { handle->cepIdxBuf[i] = handle->cepIdxBuf[i - 1]; }
    handle->cepIdxBuf[0] = max_idx_q15;

    handle->vad = 0;
    pitch_dist = abs((int)max_idx_q15 - (int)handle->pitch);

    if (cepData_max > cepMax_thrd) {
        handle->vad = 1;
        handle->pitch = max_idx_q15;
    } else if ((pitch_dist < 15) && handle->pitch != 0) {
        handle->vad = 1;
        if (cepData_max > cepMax_thrdLow) handle->pitch = max_idx_q15;
    } else if ((idx_dist_sum > 4) && (cepData_max > cepMax_thrdLow)) {
        handle->vad = 1;
        handle->pitch = max_idx_q15;
    }

    for (int i = 1; i < handle->pitchBufLen; i++) { handle->pitchBuf[i] = handle->pitchBuf[i - 1]; }
    handle->cepIdxBuf[0] = max_idx_q15;

    if (handle->vad)
        handle->pitchBuf[0] = handle->pitch;
    else
        handle->pitchBuf[0] = 0;

    // all value in pitch buf is 0, set pitch to 0.
    idx = 0;
    while ((handle->pitchBuf[idx] == 0) && (idx < handle->pitchBufLen)) ++idx;
    if (idx == handle->pitchBufLen) handle->pitch = 0;

    return handle->vad;
}
