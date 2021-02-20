#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "CepstrumVAD.h"
// #include "NoiseReduce.h"
// #include "SoundLocater.h"
// #include "Beamformer.h"

using namespace ::testing;

TEST(CepstrumVADTest, Process) {
    // FILE* fptr = fopen("cepstrum.bin", "rb");
    int fftlen = 512;
    int sample_rate = 16000;
    float freq_data[512];
    // fread(freq_data, sizeof(float), 256, fptr);

    for (int i = 0; i < 512; i++) { 
        freq_data[i] = 0;
    }

    CepstrumVAD stCepVAD;
    CepstrumVAD_Init(&stCepVAD, fftlen, sample_rate);
    int vad = CepstrumVAD_Process(&stCepVAD, freq_data);
    CepstrumVAD_Release(&stCepVAD);

    ASSERT_EQ(vad, 0) << "finished test" << std::endl;
}

// TEST(NoiseReduceTest, Process) {
//     NoiseReduce stNoiseEst;
//     int fftlen = 512;
//     int sample_rate = 16000;
//     int bPostFilt = 0;
//     float power[512];
//     int frame_cnt = 0;
//     uint32_t cep_vad = 0;
//
//     NoiseReduce_Init(&stNoiseEst, sample_rate, fftlen, bPostFilt);
//     NoiseReduce_EstimateNoise(&stNoiseEst, power, frame_cnt, cep_vad);
//     NoiseReduce_SnrVAD(&stNoiseEst);
// }

// TEST(Beamformer, Process) { 
// }
//
// TEST(SoundLocater, Process) { 
// }

