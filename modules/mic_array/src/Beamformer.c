#include "Beamformer.h"
#include "cmatrix.h"
#include "fftwrap.h"
#include <math.h>
#include <stdlib.h>

int32_t Beamformer_Init(Beamformer *handle, uint32_t fftlen, uint32_t nchannel) {
    uint32_t i, c;
    uint32_t half_fftlen;

    half_fftlen = uiv_half_fftlen(fftlen);

    handle->fn = 0;
    handle->half_fftlen = half_fftlen;
    handle->nchannel = nchannel;
    handle->speech_cnt = 0;
    handle->noise_cnt = 0;

    handle->speechRyy =
        (complex float *)calloc(half_fftlen * nchannel * nchannel, sizeof(complex float));
    handle->noiseRvv =
        (complex float *)calloc(half_fftlen * nchannel * nchannel, sizeof(complex float));
    handle->bf_coef = (complex float *)calloc(half_fftlen * nchannel, sizeof(complex float));
    handle->steering = (complex float *)calloc(half_fftlen * nchannel, sizeof(complex float));

    for (i = 0; i < half_fftlen; ++i) {
        for (c = 0; c < nchannel; ++c) {
            handle->bf_coef[i * nchannel + c] = 1.f / sqrtf(nchannel);
            handle->steering[i * nchannel + c] = 1.f / sqrtf(nchannel);
        }

        for (c = 0; c < nchannel; ++c) {
            handle->speechRyy[i * nchannel * nchannel + c * nchannel + c] = 4096; //1.f / sqrtf(nchannel);
            handle->noiseRvv[i * nchannel * nchannel + c * nchannel + c] = 4096; //1.f / sqrtf(nchannel);
        }
    }

    return 0;
}

int32_t Beamformer_Release(Beamformer *handle) {
    free(handle->speechRyy);
    free(handle->noiseRvv);
    free(handle->bf_coef);
    free(handle->steering);

    return 0;
}

uint32_t Beamformer_UpdateSpeechMatrix(Beamformer *handle, complex float *X_itr,
                                      uint32_t speech_status) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    float alpha;
    uint32_t update_speech;

    handle->fn++;
    update_speech = 0;
    if (speech_status == 0 && handle->fn >= 50) { return update_speech; }

    handle->speech_cnt++;
    if (handle->speech_cnt >= 32768) handle->speech_cnt = 32768;
    alpha = (handle->speech_cnt > 100) ? 0.95f : 0.5f;

    for (int k = 0; k < half_fftlen; k++) {
        complex float *speechRyy = &handle->speechRyy[k * nchannel * nchannel];
        complex float *X = &X_itr[k * nchannel];
        complex float Ryy = 0;

        for (int i = 0; i < nchannel; i++) {
            Ryy = X[i] * conjf(X[i]);
            speechRyy[i * nchannel + i] =
                alpha * speechRyy[i * nchannel + i] + (1.0f - alpha) * Ryy;

            for (int j = i + 1; j < nchannel; j++) {
                Ryy = X[i] * conjf(X[j]);
                speechRyy[i * nchannel + j] =
                    alpha * speechRyy[i * nchannel + j] + (1.0f - alpha) * Ryy;
                speechRyy[j * nchannel + i] = conjf(speechRyy[i * nchannel + j]);
            }
        }
    }

    if ((handle->speech_cnt < 100) || (handle->speech_cnt % 2 == 0)) update_speech = 1;

    return update_speech;
}

uint32_t Beamformer_UpdateNoiseMatrix(Beamformer *handle, complex float *X_itr, uint32_t noise_status,
                                     float *spp) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    float alpha;
    uint32_t update_noise;

    update_noise = 0;
    if (noise_status == 0) { return update_noise; }

    handle->noise_cnt++;
    if (handle->noise_cnt >= 32768) handle->noise_cnt = 32768;
    alpha = (handle->noise_cnt > 100) ? 0.95f : 0.5f;

    for (int k = 0; k < half_fftlen; k++) {
        complex float *noiseRvv = &handle->noiseRvv[k * nchannel * nchannel];
        complex float *X = &X_itr[k * nchannel];
        complex float Rvv = 0;
        float npp = 1 - spp[k];

        for (int i = 0; i < nchannel; i++) {
            complex float X_npp = X[i] * npp;
            Rvv = X_npp * conjf(X_npp);
            noiseRvv[i * nchannel + i] = alpha * noiseRvv[i * nchannel + i] + (1.0f - alpha) * Rvv;

            for (int j = i + 1; j < nchannel; j++) {
                Rvv = X[i] * conjf(X[j]);
                Rvv *= (npp * npp);
                noiseRvv[i * nchannel + j] =
                    alpha * noiseRvv[i * nchannel + j] + (1.0f - alpha) * Rvv;
                noiseRvv[j * nchannel + i] = conjf(noiseRvv[i * nchannel + j]);
            }
        }
    }

    if (handle->noise_cnt % 2 == 0) update_noise = 1;

    return update_noise;
}

void Beamformer_UpdateSteeringVector(Beamformer *handle, uint32_t update_speech,
                                     uint32_t update_noise) {
    if ((update_speech == 0) && (update_noise == 0)) return;

    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    complex float Rxx[nchannel * nchannel];

    for (int k = 0; k < half_fftlen; k++) {
        complex float *Ryy = &handle->speechRyy[k * nchannel * nchannel];
        complex float *Rvv = &handle->noiseRvv[k * nchannel * nchannel];
        complex float *steering = &handle->steering[k * nchannel];

        float traceRyy = 0, traceRvv = 0;
        for (int n = 0; n < nchannel; n++) {
            traceRvv = Rvv[n * nchannel + n];
            traceRyy = Ryy[n * nchannel + n];
        }

        if (traceRyy > 2 * traceRvv * nchannel) {
            for (int i = 0; i < nchannel; i++) {
                for (int j = 0; j < nchannel; j++) {
                    Rxx[i * nchannel + j] = Ryy[i * nchannel + j] - Rvv[i * nchannel + j];
                }
            }
        } else {
            for (int i = 0; i < nchannel; i++) {
                for (int j = 0; j < nchannel; j++) {
                    Rxx[i * nchannel + j] = Ryy[i * nchannel + j];
                }
            }
        }

        // np.matl(Rxx, v)
        complex float v[nchannel];
        for (int i = 0; i < nchannel; i++) {
            v[i] = 0;
            for (int j = 0; j < nchannel; j++) { v[i] += Rxx[i * nchannel + j] * steering[j]; }
        }

        // normalize
        float norm = 0;
        for (int i = 0; i < nchannel; i++) { norm += (v[i] * conjf(v[i])); }
        norm = sqrtf(norm);
        for (int i = 0; i < nchannel; i++) {
            v[i] /= norm;
            steering[i] = v[i];
        }
    }
}

int32_t Beamformer_UpdateMvdrFilter(Beamformer *handle, uint32_t update_speech,
                                    uint32_t update_noise) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;

    complex float *chol = calloc(nchannel * nchannel, sizeof(complex float));
    complex float *chol_inv = calloc(nchannel * nchannel, sizeof(complex float));
    complex float *chol_inv_hmt = calloc(nchannel * nchannel, sizeof(complex float));
    complex float *Rvv_inv = calloc(nchannel * nchannel, sizeof(complex float));
    complex float *num = calloc(nchannel * 1, sizeof(complex float));
    complex float *steering_hmt = calloc(nchannel * 1, sizeof(complex float));
    complex float den;

    if ((0 == update_speech) && (0 == update_noise)) { return -1; }

    for (int k = 0; k < half_fftlen; k++) {
        complex float *steering = &handle->steering[k * nchannel];
        complex float *Rvv = &handle->noiseRvv[k * nchannel * nchannel];
        complex float *bf_coef = &handle->bf_coef[k * nchannel];

        cholesky(Rvv, nchannel, chol);
        finversion(chol, nchannel, chol_inv);
        hermitian(chol_inv, nchannel, chol_inv_hmt);
        matmul(chol_inv_hmt, chol_inv, nchannel, nchannel, nchannel, Rvv_inv);

        matmul(Rvv_inv, steering, nchannel, nchannel, 1, num);
        /* hermitian(steering, nchannel, steering_hmt); */
        for (int n = 0; n < nchannel; n++) { steering_hmt[n] = conjf(steering[n]); }
        matmul(steering_hmt, num, 1, nchannel, 1, &den);

        for (int n = 0; n < nchannel; n++) {
            complex float coef = num[n] / (den + 1e-12);
            bf_coef[n] = 0.9f * bf_coef[n] + 0.1 * coef;
            /* bf_coef[n] = num[n] / (den + 1e-12); */
        }
    }

    free(chol);
    free(chol_inv);
    free(chol_inv_hmt);
    free(Rvv_inv);
    free(steering_hmt);
    free(num);

    return 0;
}

int32_t Beamformer_DoFilter(Beamformer *handle, complex float *X_itr, complex float *beamformed) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    complex float *w = handle->bf_coef;

    for (int k = 0; k < half_fftlen; k++) {
        beamformed[k] = 0;
        for (int c = 0; c < nchannel; c++) {
            beamformed[k] += (conjf(w[k * nchannel + c]) * X_itr[k * nchannel + c]);
        }
        beamformed[k] /= sqrtf(nchannel);
    }

    return 0;
}
