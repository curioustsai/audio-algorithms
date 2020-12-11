#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoundLocater.h"
#include "basic_op.h"
#include "fftwrap.h"

#define ANGLE_UNVALID 1000

void update_buffer_f32(float *data_buf, float new_data, int buf_len) {
    for (int index = 0; index < buf_len - 1; index++) { data_buf[index + 1] = data_buf[index]; }
    data_buf[0] = new_data;
}

void update_buffer_int32(int32_t *data_buf, uint32_t new_data, int buf_len) {
    for (int index = 0; index < buf_len - 1; index++) { data_buf[index + 1] = data_buf[index]; }
    data_buf[0] = new_data;
}

float amax_f32(float *array, int len, int *index) {
    float max_value = array[0];
    int max_index = 0;

    for (int i = 1; i < len; i++) {
        if (array[i] > max_value) {
            max_value = array[i];
            max_index = i;
        }
    }

    if (index != NULL) *index = max_index;

    return max_value;
}

void SoundLocater_Init(SoundLocater *handle, uint32_t fs, uint32_t fftlen, uint32_t nchannel) {
    handle->fn = 0;
    handle->fs = fs;
    handle->sound_speed = 343.0f;

    handle->nchannel = nchannel;
    handle->fftlen = fftlen;
    handle->half_fftlen = uiv_half_fftlen(fftlen);
    handle->free_tracking_mode = 0;
    handle->target_angle = 252;

    handle->num_pair = (nchannel >= 3) ? 3 : 1;

    /**
	 * GCC-Phat
	 */
    handle->num_interpolation = 1;
    handle->num_angle_samples = 181;
    handle->fftlen_interpl = handle->fftlen * handle->num_interpolation;
    handle->half_fftlen_interpl = uiv_half_fftlen(handle->fftlen_interpl);
    handle->candidate_angle = (float *)calloc(handle->num_angle_samples, sizeof(float));
    handle->mapper =
        (uint32_t *)calloc(handle->num_angle_samples * handle->num_pair, sizeof(uint32_t));
    handle->xcorr =
        (complex float *)calloc(handle->half_fftlen * handle->num_pair,  sizeof(complex float));

    handle->theta_pair_rad = (float *)calloc(handle->num_pair, sizeof(float));
    handle->theta_pair_deg = (float *)calloc(handle->num_pair, sizeof(float));
    handle->fft_lookup = uiv_fft_init(handle->fftlen_interpl);

    /**
	 * Projection
	 */
    handle->micPos = malloc(handle->num_pair * sizeof(float[3]));
    handle->basis = malloc(handle->num_pair * sizeof(float[3]));

    handle->micPos[0][0] = 2.f / 100;
    handle->micPos[0][1] = 0.f;
    handle->micPos[0][2] = 0.f;

    handle->micPos[1][0] = -2.f / 100;
    handle->micPos[1][1] = 0.f;
    handle->micPos[1][2] = 0.f;

    handle->micPos[2][0] = 0.f;
    handle->micPos[2][1] = 5.23f / 100;
    handle->micPos[2][2] = 0.f;

    handle->theta_rad = 0;
    handle->phi_rad = 0;
    handle->theta_deg = 0;
    handle->phi_deg = 0;
    handle->energy = 0;

    handle->basis[0][0] = handle->micPos[1][0] - handle->micPos[0][0];
    handle->basis[0][1] = handle->micPos[1][1] - handle->micPos[0][1];
    handle->basis[0][2] = handle->micPos[1][2] - handle->micPos[0][2];

    if (handle->nchannel >= 3) {
        handle->basis[1][0] = handle->micPos[2][0] - handle->micPos[0][0];
        handle->basis[1][1] = handle->micPos[2][1] - handle->micPos[0][1];
        handle->basis[1][2] = handle->micPos[2][2] - handle->micPos[0][2];

        handle->basis[2][0] = handle->micPos[2][0] - handle->micPos[1][0];
        handle->basis[2][1] = handle->micPos[2][1] - handle->micPos[1][1];
        handle->basis[2][2] = handle->micPos[2][2] - handle->micPos[1][2];

        handle->project_matrix = (float *)malloc(3 * 3 * sizeof(float)); // projection on 3D
        memset(handle->project_matrix, 0, 3 * 3 * sizeof(float));

        for (int pair_id = 0; pair_id < 3; pair_id++) {
            for (int i = 0; i < 3; i++) {
                handle->project_matrix[i * 3 + i] +=
                    handle->basis[pair_id][i] * handle->basis[pair_id][i];
                for (int j = i + 1; j < 3; j++) {
                    handle->project_matrix[i * 3 + j] +=
                        handle->basis[pair_id][i] * handle->basis[pair_id][j];
                    handle->project_matrix[j * 3 + i] += handle->project_matrix[i * 3 + j];
                }
            }
        }

        handle->trace_matrix = handle->project_matrix[0] + handle->project_matrix[1 * 3 + 1] +
                               handle->project_matrix[2 * 3 + 2];
    }

    for (int q = 0; q < handle->num_angle_samples; q++) {
        handle->candidate_angle[q] = (float)q / (float)(handle->num_angle_samples - 1) * M_PI;
    }

    for (int pair_id = 0; pair_id < handle->num_pair; pair_id++) {
        float c = handle->sound_speed;
        float mic_dist = 0.f;

        for (int axis = 0; axis < 3; axis++) {
            mic_dist += (handle->basis[pair_id][axis] * handle->basis[pair_id][axis]);
        }
        mic_dist = sqrtf(mic_dist);

        for (int q = 0; q < handle->num_angle_samples; q++) {
            float tdoa = roundf(handle->num_interpolation * mic_dist * fs / c *
                                cosf(handle->candidate_angle[q])) /
                         handle->num_interpolation;

            handle->mapper[pair_id * handle->num_angle_samples + q] =
                (uint32_t)(handle->num_interpolation * tdoa +
                           (tdoa < 0) * handle->num_interpolation * handle->fftlen);
        }
    }

    handle->xcorr_interpl =
        (complex float *)malloc(handle->half_fftlen_interpl * sizeof(complex float));
    handle->gphat = (float *)malloc(handle->num_angle_samples * sizeof(float));
    handle->ifft = (float *)malloc(handle->fftlen_interpl * sizeof(float));

    /**
	 * Cluster
	 */
    handle->angleBufLen = 62;
    handle->angleCluster = 0;

    handle->angleStep = 5;
    handle->vadBuf = (int32_t *)calloc(handle->angleBufLen, sizeof(int32_t));
    handle->angleBuf = (int32_t *)calloc(handle->angleBufLen, sizeof(int32_t));
    handle->gccValueBuf = (float *)calloc(handle->angleBufLen, sizeof(float));

    int angle_range = (handle->num_pair >= 3) ? 360 : 180;
    int step = handle->angleStep;

    handle->weightBuf = (float *)calloc((angle_range + 2 * step), sizeof(float));
    handle->numBuf = (uint32_t *)calloc((angle_range + 2 * step), sizeof(uint32_t));
    handle->numBuf_vad = (uint32_t *)calloc((angle_range + 2 * step), sizeof(uint32_t));

    handle->currentBeamIndex = 0;
    handle->angleRetainNum = 1;
    handle->angleReset = (uint32_t *)calloc(handle->angleRetainNum, sizeof(uint32_t));
    handle->angleRetain = (int32_t *)calloc(handle->angleRetainNum, sizeof(int32_t));
    handle->sameSourceCount = (int32_t *)calloc(handle->angleRetainNum, sizeof(int32_t));
    handle->diffSourceCount = (int32_t *)calloc(handle->angleRetainNum, sizeof(int32_t));
    handle->angleDiff = (int *)calloc(handle->angleRetainNum, sizeof(int));

    handle->angleRetainMaxTime = (uint32_t)(10 / 0.016);

    handle->gccValThreshold = 0.1f;
    handle->angleNumThreshold = 20;
    handle->vadNumThreshold = 10;
    handle->weightThreshold = 0.1f;

    return;
}

void SoundLocater_GccPhat(SoundLocater *handle, complex float *X, complex float *Y,
                          uint32_t pair_id, float *angle, float *energy) {
    complex float *xcorr_interpl = handle->xcorr_interpl;
    float *gphat = handle->gphat;
    float *ifft = handle->ifft;

    uint32_t half_fftlen = handle->half_fftlen;
    uint32_t half_fftlen_interpl = handle->half_fftlen_interpl;

    for (int i = 0; i < half_fftlen; i++) {
        complex float xcorr = X[i] * conjf(Y[i]);
        xcorr = xcorr / cabsf(xcorr);
        handle->xcorr[pair_id * half_fftlen + i] =
            0.9f * handle->xcorr[pair_id * half_fftlen + i] + 0.1f * xcorr;
    }

    for (int i = 0; i < half_fftlen; i++) {
        xcorr_interpl[i] = handle->xcorr[pair_id * half_fftlen + i];
    }
    for (int i = half_fftlen; i < half_fftlen_interpl; i++) { xcorr_interpl[i] = 0; }

    uiv_ifft(handle->fft_lookup, (float *)(xcorr_interpl), ifft);

    for (int q = 0; q < handle->num_angle_samples; q++) {
        int index = handle->mapper[pair_id * handle->num_angle_samples + q];
        gphat[q] = ifft[index];
    }

    int max_index = 0;
    float max_gphat = gphat[0];
    for (int q = 1; q < handle->num_angle_samples; q++) {
        if (gphat[q] > max_gphat) {
            max_gphat = gphat[q];
            max_index = q;
        }
    }

    *angle = handle->candidate_angle[max_index];
    *energy = gphat[max_index];
}

void SoundLocater_ProjectAngle(SoundLocater *handle, float *theta_rad, float *theta_deg,
                               float *phi_rad, float *phi_deg) {

    float cos_theta0 = cosf(handle->theta_pair_rad[0]);
    float cos_theta1 = cosf(handle->theta_pair_rad[1]);
    float cos_theta2 = cosf(handle->theta_pair_rad[2]);

    float b[3] = {0.f};
    for (int i = 0; i < 3; i++) {
        b[i] = cos_theta0 * handle->basis[0][i];
        b[i] += cos_theta1 * handle->basis[1][i];
        b[i] += cos_theta2 * handle->basis[2][i];
    }

    float n[3] = {0.f, 0.f, 0.05f};
    for (int iter = 0; iter < 20; iter++) {
        float a[3] = {0.f};

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) { a[i] += n[i] * handle->project_matrix[i * 3 + j]; }
        }

        for (int i = 0; i < 3; i++) { n[i] = n[i] - (a[i] - b[i]) / handle->trace_matrix; }
    }

    *theta_rad = atan2f(n[1], n[0]);
    *theta_deg = *theta_rad * 180.f / M_PI;
    if (*theta_deg < 0) { *theta_deg = *theta_deg + 360.f; }

    *phi_rad = atan2f(n[2], sqrtf(n[0] * n[0] + n[1] * n[1]));
    *phi_deg = *phi_rad * 180.f / M_PI;
}

void SoundLocater_FindDoa(SoundLocater *handle, complex float *X, uint32_t *angle_deg,
                          float *energy) {
    handle->fn = handle->fn + 1;

    if (handle->nchannel >= 3) {
        complex float *ch0 = X;
        complex float *ch1 = X + handle->half_fftlen;
        complex float *ch2 = X + handle->half_fftlen * 2U;
        float angle0, angle1, angle2;
        float gccVal0, gccVal1, gccVal2;

        SoundLocater_GccPhat(handle, ch0, ch1, 0, &angle0, &gccVal0);
        SoundLocater_GccPhat(handle, ch0, ch2, 1, &angle1, &gccVal1);
        SoundLocater_GccPhat(handle, ch1, ch2, 2, &angle2, &gccVal2);

        handle->theta_pair_rad[0] = angle0;
        handle->theta_pair_rad[1] = angle1;
        handle->theta_pair_rad[2] = angle2;

        handle->theta_pair_deg[0] = angle0 * 180.f / M_PI;
        handle->theta_pair_deg[1] = angle1 * 180.f / M_PI;
        handle->theta_pair_deg[2] = angle2 * 180.f / M_PI;

        /* printf("angle deg: %f\t%f\n", handle->theta_pair_deg[0], handle->theta_pair_deg[1]); */

        SoundLocater_ProjectAngle(handle, &handle->theta_rad, &handle->theta_deg, &handle->phi_rad,
                                  &handle->phi_deg);
        printf("theta deg: %f\n", handle->theta_deg);

        handle->energy = gccVal0;
        *energy = gccVal0;
        *angle_deg = (uint32_t)roundf(handle->theta_deg);

    } else {
        complex float *ch0 = X;
        complex float *ch1 = X + handle->half_fftlen;
        float angle, gccVal;

        SoundLocater_GccPhat(handle, ch0, ch1, 0, &angle, &gccVal);

        handle->theta_rad = angle;
        handle->energy = gccVal;
        // FIXME: make sure angle range: 0 ~ 359
        *angle_deg = (uint32_t)roundf(angle * 180.f / M_PI);
        *angle_deg = min(359, max(0, *angle_deg));
        *energy = gccVal;
    }
}

void SoundLocater_ClusterAngle(SoundLocater *handle, float gccValThreshold, uint32_t *angleCluster,
                               float *weightMax, uint32_t *angleNum, uint32_t *vadNum) {
    float *weightBuf = handle->weightBuf;
    uint32_t *numBuf = handle->numBuf;
    uint32_t *numBuf_vad = handle->numBuf_vad;
    uint32_t angle_range = (handle->nchannel >= 3) ? 360 : 180;
    uint32_t step = handle->angleStep;

    for (int i = 0; i < angle_range + 2 * step; i++) {
        weightBuf[i] = 0;
        numBuf[i] = 0;
        numBuf_vad[i] = 0;
    }

    for (int i = 0; i < handle->angleBufLen; i++) {
        uint32_t angle = handle->angleBuf[i];
        float value = max(handle->gccValueBuf[i] - gccValThreshold, 0);

        for (uint32_t iter = angle; iter < angle + 2 * step; iter++) {
            weightBuf[iter] = weightBuf[iter] + value * value;
            numBuf[iter] = numBuf[iter] + 1;
            numBuf_vad[iter] = numBuf_vad[iter] + (handle->vadBuf[i] == 1);
        }
    }

    if (360 == angle_range) {
        for (uint32_t iter = 0; iter < step; iter++) {
            weightBuf[angle_range + iter] = weightBuf[angle_range + iter] + weightBuf[iter];
            weightBuf[step + iter] = weightBuf[step + iter] + weightBuf[angle_range + step + iter];

            numBuf[angle_range + iter] = numBuf[angle_range + iter] + numBuf[iter];
            numBuf[step + iter] = numBuf[step + iter] + numBuf[angle_range + step + iter];

            numBuf_vad[angle_range + iter] = numBuf_vad[angle_range + iter] + numBuf_vad[iter];
            numBuf_vad[step + iter] =
                numBuf_vad[step + iter] + numBuf_vad[angle_range + step + iter];
        }
    }

    float weightMax_ = weightBuf[step];
    int max_index = step;

    for (int i = step + 1; i < angle_range + step; i++) {
        if (weightBuf[i] > weightMax_) {
            max_index = i;
            weightMax_ = weightBuf[i];
        }
    }

    *weightMax = weightMax_;
    *angleNum = numBuf[max_index];
    *vadNum = numBuf_vad[max_index];
    *angleCluster = (weightMax_ == 0.0) ? ANGLE_UNVALID : (max_index - step);

    return;
}

void SoundLocater_ReplaceAngle(SoundLocater *handle, int32_t angleClusterNew,
                               uint32_t angleDistThreshold) {
    int *angleDiff = handle->angleDiff;
    int min_index = 0;
    int min_angle_dist = abs(angleClusterNew - handle->angleRetain[0]);
    min_angle_dist = min(min_angle_dist, 360 - min_angle_dist);

    if (angleClusterNew == ANGLE_UNVALID) return;

    for (int i = 1; i < handle->angleRetainNum; i++) {
        angleDiff[i] = abs(angleClusterNew - handle->angleRetain[i]);
        angleDiff[i] = min(angleDiff[i], 360 - angleDiff[i]);
        if (angleDiff[i] < min_angle_dist) {
            min_index = i;
            min_angle_dist = angleDiff[i];
        }
    }

    if (min_angle_dist < angleDistThreshold) {
        handle->currentBeamIndex = min_index;
        handle->angleRetain[min_index] = angleClusterNew;
    } else {
        int minRetainTime = handle->angleRetainMaxTime;
        int angleReplace = 0;

        for (int i = 0; i < handle->angleRetainNum; i++) {
            if (minRetainTime > (handle->sameSourceCount[i] - handle->diffSourceCount[i])) {
                minRetainTime = handle->sameSourceCount[i] - handle->diffSourceCount[i];
                angleReplace = i;
            }
        }

        handle->currentBeamIndex = angleReplace;
        handle->angleRetain[angleReplace] = angleClusterNew;
        handle->sameSourceCount[angleReplace] = 0;
        handle->diffSourceCount[angleReplace] = 0;
        handle->angleReset[angleReplace] = 0;
    }
}

void SoundLocater_RetainAngle(SoundLocater *handle, float *weightBuf, float weightThreshold) {
    for (int i = 0; i < handle->angleRetainNum; i++) {
        int destAngle = handle->angleRetain[i];
        int diffSourceCount = handle->diffSourceCount[i];
        int sameSourceCount = handle->sameSourceCount[i];
        float weight = 0.0f;

        if (destAngle != ANGLE_UNVALID) {
            if (handle->nchannel >= 3) {
                if (destAngle < 20) {
                    weight = amax_f32(&weightBuf[360 + destAngle - 20], 20 - destAngle, NULL);
                    weight = fmaxf(weight, amax_f32(&weightBuf[0], destAngle + 20, NULL));
                } else if (destAngle > 340) {
                    weight = amax_f32(&weightBuf[destAngle - 20], 360, NULL);
                    weight = fmaxf(weight, amax_f32(&weightBuf[0], destAngle + 20 - 360, NULL));
                } else {
                    weight = amax_f32(&weightBuf[destAngle - 20], 40, NULL);
                }
            } else {
                int angleMin = max(destAngle - 10, 0);
                int angleMax = min(destAngle + 10, 180);
                int length = angleMax - angleMin + 1;

                weight = amax_f32(&weightBuf[angleMin], length, NULL);
            }

            if (weight > weightThreshold) {
                diffSourceCount = 0;
                sameSourceCount = sameSourceCount + 1;
            } else {
                diffSourceCount = diffSourceCount + 1;
            }

            int threshold_min = (int)(0.160 / 0.016);
            sameSourceCount = min(sameSourceCount, handle->angleRetainMaxTime);
            int threshold = max(sameSourceCount, threshold_min);

            if (diffSourceCount > threshold) {
                destAngle = ANGLE_UNVALID;
                diffSourceCount = 0;
                sameSourceCount = 0;
            }

            handle->angleRetain[i] = destAngle;
            handle->diffSourceCount[i] = diffSourceCount;
            handle->sameSourceCount[i] = sameSourceCount;
        }
    }
}

uint32_t SoundLocater_InBeamDet(SoundLocater *handle, int *angleBuf, int angleRetain) {
    if (angleRetain == ANGLE_UNVALID) return 0;

    int delta_angle = min(abs(angleBuf[0] - angleRetain), 360 - abs(angleBuf[0] - angleRetain));

    handle->inbeam = 0;
    if (delta_angle < 15) { handle->inbeam = 1; }

    return handle->inbeam;
}

uint32_t SoundLocater_OutBeamDet(SoundLocater *handle, int *angleBuf, float *gccValueBuf,
                                 int angleRetain, int frame_delay) {
    int angle = angleBuf[frame_delay - 1];
    int delta_angle = abs(angle - angleRetain);

    if (delta_angle < 0) { delta_angle = 360 + delta_angle; }

    if (delta_angle > 60) {
        handle->outbeam = 1;

        int valid_count = 0;
        int valid_count2 = 0;
        int valid_count3 = 0;

        for (int i = 0; i < frame_delay + 20; i++) {
            int angleDiff =
                min(abs(angleBuf[i] - angleRetain), 360 - abs(angleBuf[i] - angleRetain));
            if (angleDiff < 10) {
                if (gccValueBuf[i] > 0.3) { valid_count++; }
                valid_count2++;
            }

            if (gccValueBuf[i] > 0.3) { valid_count3++; }
        }

        if ((valid_count > 1) || (valid_count2 > 3) || (valid_count3 > 4)) { handle->outbeam = 0; }

    } else {
        handle->outbeam = 0;
    }

    return handle->outbeam;
}

void SoundLocater_Cluster(SoundLocater *handle, uint32_t angle_deg, float gccVal, uint32_t vad,
                          int *inbeam, int *outbeam) {
    uint32_t bufLen = handle->angleBufLen;
    uint32_t angleCluster = 0, angleNum = 0, vadNum = 0;
    uint32_t angleDistThreshold = 10;
    float weightMax = 0.f;
    float gccValThreshold = handle->gccValThreshold;

    update_buffer_int32(handle->angleBuf, angle_deg, bufLen);
    update_buffer_int32(handle->vadBuf, vad, bufLen);
    update_buffer_f32(handle->gccValueBuf, gccVal, bufLen);

    SoundLocater_ClusterAngle(handle, gccValThreshold, &angleCluster, &weightMax, &angleNum,
                              &vadNum);
    handle->angleCluster = angleCluster;

    //    printf("angle Cluster: %d\n", handle->angleCluster);

    if ((weightMax > handle->weightThreshold) && (angleNum > handle->angleNumThreshold) &&
        (vadNum > handle->vadNumThreshold)) {
        SoundLocater_ReplaceAngle(handle, angleCluster, angleDistThreshold);
    }

    float weightThreshold = 0.25f;
    if (weightMax > weightThreshold) {
        SoundLocater_RetainAngle(handle, handle->weightBuf, weightThreshold);
    }

    uint32_t curBeamIdx = handle->currentBeamIndex;
    uint32_t angleRetain = handle->angleRetain[curBeamIdx];
    uint32_t targetAngle = (handle->free_tracking_mode) ? angleRetain : handle->target_angle;

    handle->inbeam = SoundLocater_InBeamDet(handle, handle->angleBuf, targetAngle);
    handle->outbeam =
        SoundLocater_OutBeamDet(handle, handle->angleBuf, handle->gccValueBuf, targetAngle, 0);

    *inbeam = handle->inbeam;
    *outbeam = handle->outbeam;

    return;
}

void SoundLocater_Release(SoundLocater *handle) {
    uiv_fft_destroy(handle->fft_lookup);

    free(handle->candidate_angle);
    free(handle->mapper);
    free(handle->xcorr);
    free(handle->theta_pair_rad);
    free(handle->theta_pair_deg);

    free(handle->xcorr_interpl);
    free(handle->gphat);
    free(handle->ifft);

    free(handle->micPos);
    free(handle->basis);
    free(handle->project_matrix);

    free(handle->vadBuf);
    free(handle->angleBuf);
    free(handle->gccValueBuf);
    free(handle->angleDiff);

    free(handle->angleReset);
    free(handle->angleRetain);
    free(handle->sameSourceCount);
    free(handle->diffSourceCount);

    free(handle->weightBuf);
    free(handle->numBuf);
    free(handle->numBuf_vad);
}

