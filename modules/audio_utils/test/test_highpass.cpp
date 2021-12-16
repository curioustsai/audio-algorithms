#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "biquad.h"
#include "equalizer.h"
#include "sndfile.h"
#include "tone_generator.h"

using namespace ubnt;

TEST(SosFilter, Sweep_HPF1kHz) {
    SosFilter sosfilt;

    // hpf 1kHz for fs=48kHz
    // from scipy import signal
    // sos_hpf = signal.cheby1(4, 3, 1000/24000, 'high', output='sos')
    const float hpf_1kHz[2][5] = {{0.60030784, -1.20061568, 0.60030784, -1.68713702, 0.76275989},
                                  {1., -2., 1., -1.95698292, 0.97569048}};

    int frameSize = 1024;
    int numFrames = 500;
    int totalLength = frameSize * numFrames;
    int samplerate = 48000;
    int f1 = 0;
    int f2 = 24000;
    float amplitude = 0.95;

    float *inbuf = new float[totalLength];
    float *outbuf = new float[totalLength];

    SF_INFO info_in, info_out;
    info_in.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info_in.samplerate = samplerate;
    info_in.channels = 1;
    info_in.frames = 0;

    info_out.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info_out.samplerate = samplerate;
    info_out.channels = 1;
    info_out.frames = 0;

    generate_chirp(inbuf, totalLength, samplerate, f1, f2, amplitude);
    SNDFILE *infile = sf_open("./Sweep_HPF1kHz_before.wav", SFM_WRITE, &info_in);
    sf_write_float(infile, inbuf, totalLength);
    sf_close(infile);

    sosfilt.reset(hpf_1kHz, 2);
    sosfilt.process(inbuf, outbuf, totalLength);
    for (int i = 0; i < numFrames; ++i) {
        sosfilt.process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./Sweep_HPF1kHz_after.wav", SFM_WRITE, &info_out);
    sf_write_float(outfile, outbuf, totalLength);
    sf_close(outfile);

    delete[] inbuf;
    delete[] outbuf;
}

TEST(SosFilter, Sweep_HPF1kHz_int16) {
    SosFilter sosfilt;

    // hpf 1kHz for fs=48kHz
    // from scipy import signal
    // sos_hpf = signal.cheby1(4, 3, 1000/24000, 'high', output='sos')
    const float hpf_1kHz[2][5] = {{0.60030784, -1.20061568, 0.60030784, -1.68713702, 0.76275989},
                                  {1., -2., 1., -1.95698292, 0.97569048}};

    int frameSize = 1024;
    int numFrames = 500;
    int totalLength = frameSize * numFrames;
    int samplerate = 48000;
    int f1 = 0;
    int f2 = 24000;
    int16_t amplitude = 32000;

    int16_t *inbuf = new int16_t[totalLength];
    int16_t *outbuf = new int16_t[totalLength];

    SF_INFO info_in, info_out;
    info_in.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    info_in.samplerate = samplerate;
    info_in.channels = 1;
    info_in.frames = 0;

    info_out.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    info_out.samplerate = samplerate;
    info_out.channels = 1;
    info_out.frames = 0;

    generate_chirp_int16(inbuf, totalLength, samplerate, f1, f2, amplitude);
    SNDFILE *infile = sf_open("./Sweep_HPF1kHz_int16_before.wav", SFM_WRITE, &info_in);
    sf_write_short(infile, inbuf, totalLength);
    sf_close(infile);

    sosfilt.reset(hpf_1kHz, 2);
    sosfilt.process(inbuf, outbuf, totalLength);
    for (int i = 0; i < numFrames; ++i) {
        sosfilt.process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./Sweep_HPF1kHz_int16_after.wav", SFM_WRITE, &info_out);
    sf_write_short(outfile, outbuf, totalLength);
    sf_close(outfile);

    delete[] inbuf;
    delete[] outbuf;
}

