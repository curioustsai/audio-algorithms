#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "comfort_noise.h"
#include "sndfile.h"

using namespace ubnt;


TEST(ComfortNoise, Process) {
    int frameSize = 1024;
    int numFrames = 10;
    // int samplerate = 48000;
    int bufSize = frameSize * numFrames;
    int16_t *buffer = new int16_t[bufSize]{0};
    int16_t maxCngLevel = 128;

    ComfortNoise* cng = new ComfortNoise(maxCngLevel);
    cng->Process(buffer, bufSize);

    // SF_INFO sfinfo;
    // sfinfo.samplerate = samplerate;
    // sfinfo.channels = 1;
    // sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    // SNDFILE *sndfile = sf_open("./cng.wav", SFM_WRITE, &sfinfo);
    // sf_write_short(sndfile, buffer, bufSize);

    delete[] buffer;
    delete cng;
}

