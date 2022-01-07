#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "ring_buffer.h"

using namespace ubnt;

TEST(RingBuffer, IndenticalFrame) {
    RingBuffer<float> buf;
    int frameSize = 1024;
    float *input = new float[frameSize];
    float *output = new float[frameSize];
    for (int i = 0; i < frameSize; i++) { input[i] = i; }

    buf.putFrame(input, frameSize);
    buf.getFrame(output, frameSize);

    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(input[i], output[i]); }

    delete[] input;
    delete[] output;
}

TEST(RingBuffer, SetDelay) {
    RingBuffer<float> buf;
    int frameSize = 1024;

    float *input = new float[frameSize];
    float *output = new float[frameSize];
    for (int i = 0; i < frameSize; i++) { input[i] = i; }

    buf.setDelay(1024);
    buf.putFrame(input, frameSize);
    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], 0); }
    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(input[i], output[i]); }

    delete[] input;
    delete[] output;
}

TEST(RingBuffer, Cirulate) {
    int frameSize = 1024;
    RingBuffer<float> buf;
    buf.resetCapacity(frameSize * 10);

    float *input = new float[frameSize];
    float *output = new float[frameSize];

    buf.setDelay(frameSize * 9 + 100);
    while (buf.getFrame(output, frameSize)) {
        for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], 0); }
    }

    buf.getFrame(output, 100);
    for (int i = 0; i < 100; i++) { ASSERT_EQ(output[i], 0); }

    // buf.showInfo();

    for (int i = 0; i < frameSize; i++) { input[i] = (float)i; }
    buf.putFrame(input, frameSize);
    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], i); }
    // buf.showInfo();

    delete[] input;
    delete[] output;
}

TEST(RingBuffer, Cirulate2) {
    int frameSize = 1024;
    RingBuffer<float> buf;
    buf.resetCapacity(frameSize * 10);

    float *input = new float[frameSize];
    float *output = new float[frameSize];

    // fill in 9216 + 100 zeros
    buf.setDelay(frameSize * 9 + 100);

    // pop out 9216 zeros
    while (buf.getFrame(output, frameSize)) {
        for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], 0); }
    }

    // fill in 1024 samples, total 1024 + 100 samples remain
    for (int i = 0; i < frameSize; i++) { input[i] = (float)i; }
    buf.putFrame(input, frameSize);
    ASSERT_EQ(buf.getInUseSamples(), 100 + frameSize);

    // pop out 100 zeros, remain 1024 samples in use
    buf.getFrame(output, 100);
    for (int i = 0; i < 100; i++) { ASSERT_EQ(output[i], 0); }
    ASSERT_EQ(buf.getInUseSamples(), frameSize);

    // fill in 1024 samples, remain 1024 + 1024 samples in use
    for (int i = 0; i < frameSize; i++) { input[i] = (float)(i + frameSize); }
    buf.putFrame(input, frameSize);

    // pop out 1024 samples, remain 1024 samples in use
    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], i); }

    // pop last 1024 samples
    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], i + frameSize); }
    ASSERT_EQ(buf.getInUseSamples(), 0);

    delete[] input;
    delete[] output;
}

TEST(RingBuffer, IndenticalFrameInt16) {
    int frameSize = 1024;
    RingBuffer<int16_t> buf(frameSize, 2);
    int16_t *input = new int16_t[frameSize];
    int16_t *output = new int16_t[frameSize];
    for (int i = 0; i < frameSize; i++) { input[i] = i; }

    buf.putFrame(input, frameSize);
    buf.getFrame(output, frameSize);

    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(input[i], output[i]); }

    delete[] input;
    delete[] output;
}

TEST(RingBuffer, CirulateInt16) {
    int frameSize = 1024;
    RingBuffer<int16_t> buf(frameSize, 10);
    int16_t *input = new int16_t[frameSize];
    int16_t *output = new int16_t[frameSize];

    for (int i = 0; i < frameSize; i++) { input[i] = i; }
    buf.setDelay(frameSize * 9);
    buf.putFrame(input, frameSize);

    for (int i = 0; i < 9; ++i) {
        buf.getFrame(output, frameSize);
        for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], 0); }
    }

    buf.putFrame(input, frameSize);

    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(input[i], output[i]); }

    buf.getFrame(output, frameSize);
    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(input[i], output[i]); }

    delete[] input;
    delete[] output;
}

