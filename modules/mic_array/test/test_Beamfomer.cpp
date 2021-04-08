#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "Beamformer.h"
#include "fftwrap.h"

using namespace ::testing;

TEST(Beamfomer, Init) {
    int fftlen = 512;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int nchannel = 3;

    Beamformer stBeamformer;
    Beamformer_Init(&stBeamformer, fftlen, nchannel);

    ASSERT_EQ(stBeamformer.half_fftlen, half_fftlen);
    ASSERT_EQ(stBeamformer.nchannel, nchannel);
}

TEST(Beamformer, UpdateSpeechMatrix) {
    int fftlen = 4;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int nchannel = 3;
    int speech_status = 1;

    Beamformer stBeamformer;
    Beamformer_Init(&stBeamformer, fftlen, nchannel);
    float *X_itr = (float *)malloc(half_fftlen * 2 * nchannel * sizeof(float));

    for (int k = 0; k < half_fftlen * 2 * nchannel; k++) { X_itr[k] = k; }

    // initialized X_itr
    Beamformer_UpdateSpeechMatrix(&stBeamformer, (_Complex float *)X_itr, speech_status);

    for (int k = 0; k < half_fftlen; k++) {
        float *Ryy = (float *)&stBeamformer.speechRyy[k * nchannel * nchannel];
        float *ptr_x = (float *)&X_itr[nchannel * 2 * k];

        for (int i = 0; i < nchannel; i++) {
            for (int j = i + 1; j < nchannel; j++) {
                float real = Ryy[2 * (i * nchannel + j)];
                float imag = Ryy[2 * (i * nchannel + j) + 1];

                float xi_real = ptr_x[2 * i];
                float xi_imag = ptr_x[2 * i + 1];
                float xj_real = ptr_x[2 * j];
                float xj_imag = ptr_x[2 * j + 1];

                // xi * conj(xj)
                float real_expect = xi_real * xj_real + xi_imag * xj_imag;
                float imag_expect = xi_imag * xj_real - xi_real * xj_imag;

                real_expect *= 0.5f;
                imag_expect *= 0.5f;

                if (i == j) { real_expect += (0.5f * 4096.f); }

                ASSERT_EQ(real, real_expect) << "k: " << k << " i: " << i << " j: " << j;
                ASSERT_EQ(imag, imag_expect) << "k: " << k << " i: " << i << " j: " << j;
            }
        }
    }

    free(X_itr);
}

TEST(Beamformer, UpdateNoiseMatrix) {
    int fftlen = 4;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int nchannel = 3;
    int noise_status = 1;

    Beamformer stBeamformer;
    Beamformer_Init(&stBeamformer, fftlen, nchannel);
    float *X_itr = (float *)malloc(half_fftlen * 2 * nchannel * sizeof(float));
    float *spp = (float *)malloc(half_fftlen * sizeof(float));

    for (int k = 0; k < half_fftlen * 2 * nchannel; k++) { X_itr[k] = k; }
    for (int k = 0; k < half_fftlen; k++) { spp[k] = 0.0f; }

    // initialized X_itr
    Beamformer_UpdateNoiseMatrix(&stBeamformer, (_Complex float *)X_itr, noise_status, spp);

    for (int k = 0; k < half_fftlen; k++) {
        float *Rvv = (float *)&stBeamformer.noiseRvv[k * nchannel * nchannel];
        float *ptr_x = (float *)&X_itr[nchannel * 2 * k];

        for (int i = 0; i < nchannel; i++) {
            for (int j = i + 1; j < nchannel; j++) {
                float real = Rvv[2 * (i * nchannel + j)];
                float imag = Rvv[2 * (i * nchannel + j) + 1];

                float xi_real = ptr_x[2 * i];
                float xi_imag = ptr_x[2 * i + 1];
                float xj_real = ptr_x[2 * j];
                float xj_imag = ptr_x[2 * j + 1];

                // xi * conj(xj)
                float real_expect = xi_real * xj_real + xi_imag * xj_imag;
                float imag_expect = xi_imag * xj_real - xi_real * xj_imag;

                real_expect *= 0.5f;
                imag_expect *= 0.5f;

                if (i == j) { real_expect += (0.5f * 4096.f); }

                ASSERT_EQ(real, real_expect) << "k: " << k << " i: " << i << " j: " << j;
                ASSERT_EQ(imag, imag_expect) << "k: " << k << " i: " << i << " j: " << j;
            }
        }
    }

    free(X_itr);
}

// TEST(Beamformer, UpdateSteeringVector) {}
