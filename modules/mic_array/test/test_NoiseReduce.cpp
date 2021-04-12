#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "CepstrumVAD.h"
#include "NoiseReduce.h"
#include "basic_op.h"
#include "fftwrap.h"

using namespace ::testing;

TEST(NoiseReduceTest, ProcessNoise) {
    FILE* fptr = fopen("./data/sn_16kHz.bin", "rb");
    int fftlen = 512;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int sample_rate = 16000;
    int bPostFilt = 0;
    uint32_t cep_vad = 0;

    float* freq_data = (float*)malloc(half_fftlen * 2 * sizeof(float));
    float* ref_power = (float*)malloc(half_fftlen * sizeof(float));

    NoiseReduce stNoiseEst;
    NoiseReduce_Init(&stNoiseEst, sample_rate, fftlen, bPostFilt);

    std::vector<int> speech_frame;
    std::vector<int> noise_frame;
	speech_frame.clear();
	noise_frame.clear();

    int frame_cnt = 0;
    while (fread(freq_data, sizeof(float), half_fftlen * 2, fptr) == (size_t)(half_fftlen * 2)) {
        uiv_cmplx_mag_squared_f32(freq_data, ref_power, half_fftlen);
        NoiseReduce_EstimateNoise(&stNoiseEst, ref_power, frame_cnt, cep_vad);
        NoiseReduce_SnrVAD(&stNoiseEst);

        speech_frame.push_back(stNoiseEst.speech_frame);
        noise_frame.push_back(stNoiseEst.noise_frame);

        frame_cnt++;
    }
	fclose(fptr);

    int speech_count = 0, noise_count = 0;
    for (auto speech : speech_frame) { speech_count += speech; }
    for (auto noise : noise_frame) { noise_count += noise; }

    ASSERT_EQ(speech_count, 0);
    ASSERT_EQ(noise_count, frame_cnt);
}

TEST(NoiseReduceTest, ProcessSpeech) {
    FILE* fptr = fopen("./data/speech_16kHz.bin", "rb");
    int fftlen = 512;
    int half_fftlen = uiv_half_fftlen(fftlen);
    int sample_rate = 16000;
    int bPostFilt = 0;
    uint32_t cep_vad = 0;

    float* freq_data = (float*)malloc(half_fftlen * 2 * sizeof(float));
    float* ref_power = (float*)malloc(half_fftlen * sizeof(float));

    CepstrumVAD stCepVAD;
    NoiseReduce stNoiseEst;

    CepstrumVAD_Init(&stCepVAD, fftlen, sample_rate);
    NoiseReduce_Init(&stNoiseEst, sample_rate, fftlen, bPostFilt);

    std::vector<int> speech_frame;
    std::vector<int> noise_frame;
	speech_frame.clear();
	noise_frame.clear();

    int frame_cnt = 0;
    while (fread(freq_data, sizeof(float), half_fftlen * 2, fptr) == (size_t)(half_fftlen * 2)) {
        uiv_cmplx_mag_squared_f32(freq_data, ref_power, half_fftlen);
        cep_vad = CepstrumVAD_Process(&stCepVAD, ref_power);
        NoiseReduce_EstimateNoise(&stNoiseEst, ref_power, frame_cnt, cep_vad);
        NoiseReduce_SnrVAD(&stNoiseEst);

        speech_frame.push_back(stNoiseEst.speech_frame);
        noise_frame.push_back(stNoiseEst.noise_frame);

        frame_cnt++;
    }

	fclose(fptr);
    CepstrumVAD_Release(&stCepVAD);
    NoiseReduce_Release(&stNoiseEst);

    int speech_count = 0, noise_count = 0;
    for (auto speech : speech_frame) { speech_count += speech; }
    for (auto noise : noise_frame) { noise_count += noise; }

	/* 
	 * first 35 frames are noise, and last 30 frames are speech
	 * total 65 frames
	 */
    ASSERT_GT(speech_count, 0);
    ASSERT_LT(speech_count, 30);
    ASSERT_GT(noise_count, 0);
    ASSERT_LT(noise_count, 35);
}
