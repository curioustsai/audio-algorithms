#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "equalizer.h"
#include "sndfile.h"

using namespace ubnt;

void generate_sine(float *buf, int length, int sample_rate = 48000, int fc = 1000,
                   float amplitude = 0.5) {
    for (int i = 0; i < length; i++) { buf[i] = amplitude * sinf(2 * M_PI * fc * i / sample_rate); }
}

void generate_chirp(float *buf, int length, int sample_rate = 48000, int f1 = 1000, int f2 = 2000,
                    float amplitude = 0.5) {
    float M = (float)length / sample_rate;
    for (int i = 0; i < length; ++i) {
        float t = (float)i / sample_rate;
        buf[i] = amplitude * sinf(2 * M_PI * (f1 * t + (f2 - f1) * t * t / (2.0 * M)));
    }
}

TEST(Equalizer, Peak) {
    Equalizer *eq;

    int fs = 48000;
    int numFrames = 500;
    int frameSize = 1024;
    int bufSize = numFrames * frameSize;
    float *inbuf = new float[bufSize];
    float *outbuf = new float[bufSize];

    // generate sweeping tones signal
    float amp = powf(10.0, -20.0 / 20.0);
    generate_chirp(inbuf, bufSize, fs, 0, 24000, amp);

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info.samplerate = fs;
    info.channels = 1;
    info.frames = 0;

    SNDFILE *outfile = sf_open("./sweep_tone.wav", SFM_WRITE, &info);
    sf_write_float(outfile, inbuf, bufSize);
    sf_close(outfile);

    // EQ coefficient
    int f0 = 4000;
    float gain = 6;
    float Q = 0.5;
    eq = new Equalizer(f0, fs, gain, Q);
    eq->process(inbuf, outbuf, bufSize);

    outfile = sf_open("./EQ_4000Hz_gain6.wav", SFM_WRITE, &info);
    sf_write_float(outfile, outbuf, bufSize);
    sf_close(outfile);

    delete eq;
    delete[] inbuf;
    delete[] outbuf;
}

TEST(Equalizer, Notch) {
    Equalizer *eq;

    int fs = 48000;
    int numFrames = 500;
    int frameSize = 1024;
    int bufSize = numFrames * frameSize;
    float *inbuf = new float[bufSize];
    float *outbuf = new float[bufSize];

    // generate sweeping tones signal
    float amp = powf(10.0, -20.0 / 20.0);
    generate_chirp(inbuf, bufSize, fs, 0, 24000, amp);

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info.samplerate = fs;
    info.channels = 1;
    info.frames = 0;

    SNDFILE *outfile = sf_open("./sweep_tone.wav", SFM_WRITE, &info);
    sf_write_float(outfile, inbuf, bufSize);
    sf_close(outfile);

    // EQ coefficient
    int f0 = 4000;
    float gain = -6;
    float Q = 0.5;
    eq = new Equalizer(f0, fs, gain, Q);
    eq->process(inbuf, outbuf, bufSize);

    outfile = sf_open("./EQ_4000Hz_gain-6.wav", SFM_WRITE, &info);
    sf_write_float(outfile, outbuf, bufSize);
    sf_close(outfile);

    delete eq;
    delete[] inbuf;
    delete[] outbuf;
}

TEST(Equalizer, MulitEQ) {
    const int NUM_EQ = 4;
    Equalizer **eq = new Equalizer*[NUM_EQ];

    int fs = 48000;
    int numFrames = 500;
    int frameSize = 1024;
    int bufSize = numFrames * frameSize;
    float *inbuf = new float[bufSize];
    float *outbuf = new float[bufSize];

    // generate sweeping tones signal
    float amp = powf(10.0, -20.0 / 20.0);
    generate_chirp(inbuf, bufSize, fs, 0, 24000, amp);

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info.samplerate = fs;
    info.channels = 1;
    info.frames = 0;

    SNDFILE *outfile = sf_open("./sweep_tone.wav", SFM_WRITE, &info);
    sf_write_float(outfile, inbuf, bufSize);
    sf_close(outfile);

    // EQ coefficient
    // int f0 = 4000;
    float gain = -6;
    eq[0] = new Equalizer(1000, fs, gain, 0.1);
    eq[1] = new Equalizer(2000, fs, gain, 0.05);
    eq[2] = new Equalizer(3000, fs, gain, 0.033);
    eq[3] = new Equalizer(4000, fs, gain, 0.025);

    eq[0]->process(inbuf, outbuf, bufSize);
    eq[1]->process(outbuf, outbuf, bufSize);
    eq[2]->process(outbuf, outbuf, bufSize);
    eq[3]->process(outbuf, outbuf, bufSize);

    outfile = sf_open("./MultiEQ.wav", SFM_WRITE, &info);
    sf_write_float(outfile, outbuf, bufSize);
    sf_close(outfile);

    for (int i = 0; i < NUM_EQ; ++i) {
       delete eq[i]; 
    }
    delete[] eq;
    delete[] inbuf;
    delete[] outbuf;
}
