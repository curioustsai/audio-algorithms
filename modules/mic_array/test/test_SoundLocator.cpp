#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "SoundLocater.h"
#include "fftwrap.h"

using namespace ::testing;

TEST(SoundLocaterParamCtrl, MicArray) {
    int fftlen = 512;
    int sample_rate = 16000;
    int nchannel = 3;

    float micPos[9] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

    float retMicPos[9] = {0.0};

    SoundLocater stSoundLocater;
    SoundLocater_Init(&stSoundLocater, sample_rate, fftlen, nchannel);

    SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_SET_MICARRAY, micPos);
    SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_GET_MICARRAY, retMicPos);

    for (int i = 0; i < nchannel * 3; i++) { ASSERT_EQ(micPos[i], retMicPos[i]); }
}

TEST(SoundLocaterParamCtrl, Freetrack) {
    int fftlen = 512;
    int sample_rate = 16000;
    int nchannel = 3;

    uint32_t free_tracking_mode = 1;
    uint32_t ret_tracking_mode;

    SoundLocater stSoundLocater;
    SoundLocater_Init(&stSoundLocater, sample_rate, fftlen, nchannel);

    SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_SET_FREETRACK, &free_tracking_mode);
    SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_GET_FREETRACK, &ret_tracking_mode);
    ASSERT_EQ(free_tracking_mode, ret_tracking_mode);

    free_tracking_mode = 0;
    SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_SET_FREETRACK, &free_tracking_mode);
    SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_GET_FREETRACK, &ret_tracking_mode);

    ASSERT_EQ(free_tracking_mode, ret_tracking_mode);
}

TEST(SoundLocaterParamCtrl, TargetAngle) {
    int fftlen = 512;
    int sample_rate = 16000;
    int nchannel = 3;

    SoundLocater stSoundLocater;
    SoundLocater_Init(&stSoundLocater, sample_rate, fftlen, nchannel);

    for (int targetAngle = 0; targetAngle <= 360; targetAngle++) {
        int retTargetAngle;
        SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_SET_TARGETANGLE, &targetAngle);
        SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_GET_TARGETANGLE, &retTargetAngle);
        ASSERT_EQ(targetAngle, retTargetAngle);
    }
}

// TEST(SoundLocater, FindDOA) {
//     int fftlen = 512;
//     int half_fftlen = uiv_half_fftlen(fftlen);
//     int sample_rate = 16000;
//     int nchannel = 2;
//     float mic_dist = 0.050f;
//     float mic_pos[6] = {0, 0, 0, 0.050, 0, 0};
//     float angle = M_PI * 45.0 / 180.0;
//     float ret_energy;
//     uint32_t ret_angle;
//
//     SoundLocater stSoundLocater;
//     SoundLocater_Init(&stSoundLocater, sample_rate, fftlen, nchannel);
// 	SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_SET_MICARRAY, mic_pos);
//
//     float* X = (float*)malloc(sizeof(float) * half_fftlen * 2 * 2);
//     float* Y = X + half_fftlen * 2;
//     float tdoa = (((float)stSoundLocater.fs) / stSoundLocater.sound_speed) * mic_dist * cosf(angle);
//
//     for (int k = 0; k < half_fftlen; k++) {
//
//         angle = 2 * M_PI * ((float)k) * tdoa / ((float)fftlen);
//
//         float y_real = cosf(angle);
//         float y_imag = sinf(angle);
//
//         Y[k * 2 + 0] = y_real;
//         Y[k * 2 + 1] = y_imag;
//         X[k * 2 + 0] = 1.0f;
//         X[k * 2 + 1] = 0.0f;
//     }
//
//     SoundLocater_FindDoa(&stSoundLocater, (_Complex float*)X, &ret_angle, &ret_energy);
//     printf("ret_angle %d\n", ret_angle);
// }

// TEST(SoundLocaterTest, DOA) {
//     FILE* fptr = fopen("./data/doa_3mic_16kHz.bin", "rb");
//     FILE* fptr_speech = fopen("./data/speech_frame.bin", "rb");
//
//     int fftlen = 512;
//     int half_fftlen = uiv_half_fftlen(fftlen);
//     int sample_rate = 16000;
//     int nchannel = 3;
// 	std::vector<int> angleRetain_all;
// 	angleRetain_all.clear();
//
//     float* inputs_f = (float*)malloc(half_fftlen * 2 * nchannel * sizeof(float));
// 	float micPos[9] = {
// 		0.02f, 0.f, 0.f,
// 		-0.02f, 0.f, 0.f,
// 		0.f, 0.0523f, 0.f
// 	};
//
//     SoundLocater stSoundLocater;
//     SoundLocater_Init(&stSoundLocater, sample_rate, fftlen, nchannel);
// 	SoundLocater_ParamCtrl(&stSoundLocater, SOUNDLOCATOR_SET_MICARRAY, micPos);
//
//     int frame_cnt = 0;
//     while (fread(inputs_f, sizeof(float), half_fftlen * 2 * nchannel, fptr) ==
//            (size_t)(half_fftlen * 2 * nchannel)) {
//
//         uint32_t angle_deg;
//         float energy;
//         int inbeam, outbeam;
//         uint32_t speech_frame;
//
//         fread(&speech_frame, sizeof(uint32_t), 1, fptr_speech);
//
//         SoundLocater_FindDoa(&stSoundLocater, (_Complex float*)inputs_f, &angle_deg, &energy);
//         SoundLocater_Cluster(&stSoundLocater, angle_deg, energy, speech_frame, &inbeam, &outbeam);
//
// 		int index = stSoundLocater.currentBeamIndex;
// 		int angleRetain = stSoundLocater.angleRetain[index];
//
// 		std::cout<< angleRetain << std::endl;
//
// 		angleRetain_all.push_back(angleRetain);
//
//         frame_cnt++;
//     }
//
//     SoundLocater_Release(&stSoundLocater);
//
//     fclose(fptr);
//     fclose(fptr_speech);
// }
