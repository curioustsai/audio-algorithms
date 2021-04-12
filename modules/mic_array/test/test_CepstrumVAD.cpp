#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "CepstrumVAD.h"
#include "NoiseReduce.h"
#include "fftwrap.h"
#include "basic_op.h"
// #include "SoundLocater.h"
// #include "Beamformer.h"

using namespace ::testing;

TEST(CepstrumVADTest, ProcessNoise) {
    FILE* fptr = fopen("./data/sn_16kHz.bin", "rb");
    int fftlen = 512;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int sample_rate = 16000;
    float* freq_data = (float*) malloc(half_fftlen * 2 * sizeof(float));
    float* ref_power = (float*) malloc(half_fftlen * sizeof(float));

    CepstrumVAD stCepVAD;
    CepstrumVAD_Init(&stCepVAD, fftlen, sample_rate);
    std::vector<int> vad_all;
	vad_all.clear();

    int frame_cnt = 0;
    while (fread(freq_data, sizeof(float), half_fftlen * 2, fptr) == (size_t)(half_fftlen*2)) {
        uiv_cmplx_mag_squared_f32(freq_data, ref_power, half_fftlen);
        int vad = CepstrumVAD_Process(&stCepVAD, ref_power);
        vad_all.push_back(vad);
        frame_cnt++;
    }
	fclose(fptr);

    CepstrumVAD_Release(&stCepVAD);

    int vad_sum = 0;
    for (auto vad: vad_all) { vad_sum += vad; }
    ASSERT_LT(vad_sum, frame_cnt >> 1);
    // for (auto vad: vad_all) { ASSERT_EQ(vad, 0); }
}

TEST(CepstrumVADTest, ProcessSpeech) {
    FILE* fptr = fopen("./data/cepvad_16kHz.bin", "rb");
    int fftlen = 512;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int sample_rate = 16000;
    float* freq_data = (float*) malloc(half_fftlen * 2 * sizeof(float));
    float* ref_power = (float*) malloc(half_fftlen * sizeof(float));
    std::vector<int> vad_all;
	vad_all.clear();

    CepstrumVAD stCepVAD;
    CepstrumVAD_Init(&stCepVAD, fftlen, sample_rate);

    int frame_cnt = 0;
    while (fread(freq_data, sizeof(float), half_fftlen * 2, fptr) == (size_t)(half_fftlen*2)) {
        uiv_cmplx_mag_squared_f32(freq_data, ref_power, half_fftlen);
        int vad = CepstrumVAD_Process(&stCepVAD, ref_power);
        vad_all.push_back(vad);
        frame_cnt++;
    }

	fclose(fptr);
    CepstrumVAD_Release(&stCepVAD);

    int vad_sum = 0;
    for (auto vad: vad_all) { vad_sum += vad; }

    ASSERT_GT(vad_sum, frame_cnt >> 1);
}
