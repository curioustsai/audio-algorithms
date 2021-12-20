#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "gain.h"
#include "sndfile.h"
#include "tone_generator.h"

using namespace ubnt;

float calculate_dBFS(float *buffer, int bufSize) {
    float average = 0;
    for (int i = 0; i < bufSize; ++i) { average += fabsf(buffer[i]); }
    return 20 * log10f(average / bufSize);
};

float calculate_dBFS_int16(int16_t *buffer, int bufSize) {
    float average = 0;
    for (int i = 0; i < bufSize; ++i) { average += fabsf(buffer[i] / 32768.f); }
    return 20 * log10f(average / bufSize);
};

TEST(Gain, Process_f32) {
    int frameSize = 1024;
    int numFrames = 100;
    int bufSize = frameSize * numFrames;
    int sampeleRate = 48000;
    int fc = 1000;
    float amplitude = 0.25;

    float *buffer = new float[bufSize];

    generate_sine(buffer, bufSize, sampeleRate, fc, amplitude);

    float dBFS_before = calculate_dBFS(buffer, bufSize);

    Gain gain(-6.0f);
    gain.Process(buffer, bufSize);

    float dBFS_after = calculate_dBFS(buffer, bufSize);
    ASSERT_LT(fabs(dBFS_before - 6.0 - dBFS_after), 0.5);

    delete[] buffer;
}

TEST(Gain, Process_int16) {
    int frameSize = 1024;
    int numFrames = 100;
    int bufSize = frameSize * numFrames;
    int sampeleRate = 48000;
    int fc = 1000;
    int16_t amplitude = 8192;

    int16_t *buffer = new int16_t[bufSize];

    generate_sine_int16(buffer, bufSize, sampeleRate, fc, amplitude);

    float dBFS_before = calculate_dBFS_int16(buffer, bufSize);

    Gain gain(6.0f);
    gain.Process(buffer, bufSize);

    float dBFS_after = calculate_dBFS_int16(buffer, bufSize);
    ASSERT_LT(fabs(dBFS_before + 6.0 - dBFS_after), 0.5);

    delete[] buffer;
}

