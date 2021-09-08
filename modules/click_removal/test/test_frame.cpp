#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <cmath>

#include "frame.h"

using namespace ubnt;

/* TEST for Frame */

TEST(Frame, Reset) {
    Frame frame;
    int frameSize = 2048;
    float *output = new float[frameSize];

    frame.reset(frameSize);
    frame.getOutput(output, frameSize);

    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(output[i], 0); }

    delete[] output;
}

TEST(Frame, UdpateFrame) {
    Frame frame{1024};
    int frameSize = 1024;
    float *inbuf = new float[frameSize];
    float *outbuf = new float[frameSize];
    for (int i = 0; i < frameSize; i++) { inbuf[i] = i; }
    frame.updateFrame(inbuf, frameSize);
    frame.getOutput(outbuf, frameSize);

    for (int i = 0; i < frameSize; i++) { ASSERT_EQ(inbuf[i], outbuf[i]); }

    delete[] inbuf;
    delete[] outbuf;
}

TEST(Frame, PowerMean) {
    Frame frame{1024};
    int frameSize = 1024;
    float *inbuf = new float[frameSize];
    for (int i = 0; i < frameSize; i++) { inbuf[i] = i; }
    frame.updateFrame(inbuf, frameSize);

    float powerMean = 0.f;
    for (int i = 0; i < frameSize; i++) { powerMean += (i * i); }
    powerMean /= frameSize;

    ASSERT_EQ(frame.getPowerMean(), powerMean);

    delete[] inbuf;
}

TEST(Frame, PowerdB) {
    Frame frame{1024};
    int frameSize = 1024;
    float *inbuf = new float[frameSize];
    for (int i = 0; i < frameSize; i++) { inbuf[i] = i; }
    frame.updateFrame(inbuf, frameSize);

    float powerdB = 0.f;
    for (int i = 0; i < frameSize; i++) { powerdB += (i * i); }
    powerdB = 10 * log10f(powerdB / frameSize);

    ASSERT_EQ(frame.getPowerdB(), powerdB);

    delete[] inbuf;
}

/* TEST for FrameOverlap */

TEST(FrameOverlap, Reset) {
    // 50% overlap
    int frameSize = 2048;
    int overlapSize = 1024;
    int hopSize = frameSize - overlapSize;
    FrameOverlap frame;
    float *inbuf = new float[hopSize]{0};
    float *outbuf = new float[hopSize]{0};

    frame.reset(frameSize, overlapSize);

    for (int i = 0; i < hopSize; i++) { inbuf[i] = i; }
    frame.updateFrame(inbuf, hopSize);
    frame.getOutput(outbuf, hopSize);
    for (int i = 0; i < hopSize; i++) { ASSERT_EQ(outbuf[i], 0); }

    for (int i = 0; i < hopSize; i++) { inbuf[i] = i + hopSize; }
    frame.updateFrame(inbuf, hopSize);
    frame.getOutput(outbuf, hopSize);
    for (int i = 0; i < hopSize; i++) { ASSERT_EQ(outbuf[i], i); }

    delete[] inbuf;
    delete[] outbuf;
}

TEST(FrameOverlap, GetPowerMean) {
    // 50% overlap
    int frameSize = 2048;
    int overlapSize = 1024;
    int hopSize = frameSize - overlapSize;
    FrameOverlap frame;
    float *inbuf = new float[hopSize]{0};

    frame.reset(frameSize, overlapSize);

    for (int i = 0; i < hopSize; i++) { inbuf[i] = i; }
    frame.updateFrame(inbuf, hopSize);

    float powerMean = 0.f;
    for (int i = 0; i < hopSize; i++) { powerMean += (i * i); }
    powerMean /= frameSize;

    ASSERT_EQ(frame.getPowerMean(), powerMean);

    delete[] inbuf;
}

TEST(FrameOverlap, GetPowerdB) {
    // 50% overlap
    int frameSize = 2048;
    int overlapSize = 1024;
    int hopSize = frameSize - overlapSize;
    FrameOverlap frame;
    float *inbuf = new float[hopSize]{0};

    frame.reset(frameSize, overlapSize);

    for (int i = 0; i < hopSize; i++) { inbuf[i] = i; }
    frame.updateFrame(inbuf, hopSize);

    float powerMean = 0.f;
    for (int i = 0; i < hopSize; i++) { powerMean += (i * i); }
    powerMean = 10 * log10f(powerMean / frameSize);

    ASSERT_EQ(frame.getPowerdB(), powerMean);

    delete[] inbuf;
}
