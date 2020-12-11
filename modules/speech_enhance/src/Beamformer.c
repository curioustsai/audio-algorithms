#include "Beamformer.h"
#include "fftwrap.h"
#include <stdlib.h>

#define MAX_NCHANNEL 4
int32_t Beamformer_Init(Beamformer* handle, uint32_t fftlen, uint32_t nchannel) {
    uint32_t i, c;
    uint32_t half_fftlen;

    half_fftlen = uiv_half_fftlen(fftlen);

    handle->fn = 0;
    handle->half_fftlen = half_fftlen;
    handle->nchannel = nchannel;
    handle->speech_cnt = 0;
    handle->noise_cnt = 0;

    handle->mu = 1.0f;
    handle->ref_ch = 0;

    handle->speechRyy =
        (complex float*)malloc(half_fftlen * nchannel * nchannel * sizeof(complex float));
    handle->noiseRvv =
        (complex float*)malloc(half_fftlen * nchannel * nchannel * sizeof(complex float));
    handle->bf_coef = (complex float*)malloc(half_fftlen * nchannel * sizeof(complex float));
    handle->steering = (complex float*)malloc(half_fftlen * nchannel * sizeof(complex float));

    for (i = 0; i < half_fftlen; ++i) {
        for (c = 0; c < nchannel; ++c) {
            handle->bf_coef[i * nchannel + c] = 1.f / sqrtf(nchannel);
			handle->steering[i * nchannel + c ] = 1.f / sqrtf(nchannel);
        }

        for (c = 0; c < nchannel; ++c) {
            handle->speechRyy[i * nchannel * nchannel + c * nchannel + c] = 4096.f;
            handle->noiseRvv[i * nchannel * nchannel + c * nchannel + c] = 4096.f;
        }
    }

    return STATUS_SUCCESS;
}

int32_t Beamformer_Release(Beamformer* handle) {
    free(handle->speechRyy);
    free(handle->noiseRvv);
    free(handle->bf_coef);
    free(handle->steering);

    return STATUS_SUCCESS;
}

uint8_t Beamformer_UpdateSpeechMatrix(Beamformer* handle, complex float* X_itr,
                                      uint8_t speech_status) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    float alpha;
    uint8_t update_speech;

    handle->fn++;
    update_speech = 0;
    if (speech_status == 0 && handle->fn >= 50) { return update_speech; }

    handle->speech_cnt++;
    if (handle->speech_cnt >= 32768) handle->speech_cnt = 32768;
    alpha = (handle->speech_cnt > 100) ? 0.95f : 0.5f;

    for (int k = 0; k < half_fftlen; k++) {
        complex float* speechRyy = &handle->speechRyy[k * nchannel * nchannel];
        complex float Ryy = 0;

        for (int i = 0; i < nchannel; i++) {
            Ryy = X_itr[i * nchannel + i] * conjf(X_itr[i * nchannel + i]);
            speechRyy[i * nchannel + i] =
                alpha * speechRyy[i * nchannel + i] + (1.0f - alpha) * Ryy;

            for (int j = i + 1; j < nchannel; j++) {
                Ryy = X_itr[i * nchannel + j] * conjf(X_itr[i * nchannel + j]);
                speechRyy[i * nchannel + j] =
                    alpha * speechRyy[i * nchannel + j] + (1.0f - alpha) * Ryy;
                speechRyy[j * nchannel + i] = speechRyy[i * nchannel + j];
            }
        }
    }

    if ((handle->speech_cnt < 100) || (handle->speech_cnt % 10 == 0)) { update_speech = 1; }

    return update_speech;
}

uint8_t Beamformer_UpdateNoiseMatrix(Beamformer* handle, complex float* X_itr, uint8_t noise_status,
                                     float* spp) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    float alpha;
    uint8_t update_noise;

    update_noise = 0;
    if (noise_status == 0 && handle->fn >= 50) { return update_noise; }
    handle->noise_cnt++;
    alpha = (handle->noise_cnt > 100) ? 0.95f : 0.5f;

    for (int k = 0; k < half_fftlen; k++) {
        complex float* noiseRvv = &handle->noiseRvv[k * nchannel * nchannel];
        complex float Rvv = 0;
        float npp = 1 - spp[k];

        for (int i = 0; i < nchannel; i++) {
            Rvv = X_itr[i * nchannel + i] * conjf(X_itr[i * nchannel + i]);
            Rvv = Rvv * npp * npp;
            noiseRvv[i * nchannel + i] = alpha * noiseRvv[i * nchannel + i] + (1.0f - alpha) * Rvv;

            for (int j = i + 1; j < nchannel; j++) {
                Rvv = X_itr[i * nchannel + j] * conjf(X_itr[i * nchannel + j]);
                noiseRvv[i * nchannel + j] =
                    alpha * noiseRvv[i * nchannel + j] + (1.0f - alpha) * Rvv;
                noiseRvv[j * nchannel + i] = noiseRvv[i * nchannel + j];
            }
        }
    }

    if (handle->noise_cnt % 10 == 0) update_noise = 1;

    return update_noise;
}

void Beamformer_UpdateSteeringVector(Beamformer *handle, uint8_t update_speech, uint8_t update_noise) {
	if ((update_speech == 0) && (update_noise == 0)) return;

	uint32_t half_fftlen = handle->half_fftlen;
	uint32_t nchannel = handle->nchannel;

	for (int k = 0; k < half_fftlen; k++) {
		complex float *Ryy = &handle->speechRyy[k*nchannel*nchannel];
		/* complex float *Rvv = &handle->noiseRvv[k*nchannel*nchannel]; */
		//TODO: denoise steering vector
		complex float *Rxx = Ryy;
		complex float *steering = &handle->steering[k*nchannel];

		// np.matl(Rxx, v)
		complex float v[nchannel];
		for (int i =0; i < nchannel; i++) { 
			v[i] = 0;
			for (int j= 0; j < nchannel; j ++) { 
				v[i] = Rxx[i*nchannel+j] * steering[j];
			}
		}
		
		// normalize
		float norm = 0;
		for (int i = 0; i < nchannel; i++) { 
			norm += (creal(v[i])*creal(v[i]) + cimag(v[i])*cimag(v[i]));
		}
		norm = sqrtf(norm);
		for (int i = 0; i < nchannel; i++) {
			v[i] /= norm;
			steering[i] = v[i];
		}
	}
}

int32_t Beamformer_UpdateMvdrFilter(Beamformer* handle, uint8_t update_speech,
                                    uint8_t update_noise) {
    uint32_t half_fftlen = handle->half_fftlen;
	uint32_t nchannel = handle->nchannel;

    if ((0 == update_speech) && (0 == update_noise)) { return STATUS_FAIL; }

	/* for (int k = 0; k < half_fftlen; k++) { */
	/* 	complex float *steering = &handle->steering[k*nchannel]; */
	/* 	complex float *Rvv = &handle->noiseRvv[k*nchannel*nchannel]; */
	/* 	complex float *bf_coef = &handle->bf_coef[k*nchannel]; */
    /*  */
	/* 	for (int i = 0; i < nchannel; i++) { */
	/* 		for(int j =0; j < nchannel; j ++) {  */
	/* 		#<{(| step 0: inverse Rvv |)}># */
    /*  */
	/* 		} */
	/* 	} */
	/* } */

    return STATUS_SUCCESS;
}


int32_t Beamformer_DoFilter(Beamformer* handle, complex float* X_itr, complex float* beamformed) {
    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t nchannel = handle->nchannel;
    complex float *w = handle->bf_coef;

	for (int k =0; k < half_fftlen; k++) {
		beamformed[k] = 0;
		for (int c = 0; c < nchannel; c++) {
			beamformed[k] += (conjf(w[k*nchannel+c]) * X_itr[k*nchannel+c]);	
		}
	}


    return STATUS_SUCCESS;
}
