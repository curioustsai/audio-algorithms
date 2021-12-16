#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <string.h>

#include "equalizer.h"
#include "sndfile.h"
#include "tone_generator.h"

using namespace ubnt;

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

    for (int i = 0; i < numFrames; ++i) {
        eq->process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

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

    // EQ coefficient
    int f0 = 4000;
    float gain = -6;
    float Q = 0.5;
    eq = new Equalizer(f0, fs, gain, Q);

    for (int i = 0; i < numFrames; ++i) {
        eq->process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./EQ_4000Hz_gain-6.wav", SFM_WRITE, &info);
    sf_write_float(outfile, outbuf, bufSize);
    sf_close(outfile);

    delete eq;
    delete[] inbuf;
    delete[] outbuf;
}

TEST(Equalizer, MulitEQ) {
    const int NUM_EQ = 4;
    Equalizer **eq = new Equalizer *[NUM_EQ];

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

    SNDFILE *outfile = sf_open("./MultiEQ.wav", SFM_WRITE, &info);
    sf_write_float(outfile, outbuf, bufSize);
    sf_close(outfile);

    for (int i = 0; i < NUM_EQ; ++i) { delete eq[i]; }
    delete[] eq;
    delete[] inbuf;
    delete[] outbuf;
}

TEST(Equalizer, Peak4kHzSpeech) {
    Equalizer *eq;

    SF_INFO info;
    SNDFILE *infile = sf_open("./test_vector/speech.wav", SFM_READ, &info);

    int fs = info.samplerate;
    int bufSize = info.frames;
    int frameSize = 1024;
    int numFrames = bufSize / frameSize;

    int16_t *inbuf = new int16_t[bufSize] {0};
    int16_t *outbuf = new int16_t[bufSize] {0};

    // EQ coefficient
    int f0 = 4000;
    float gain = 6;
    float Q = 0.5;
    eq = new Equalizer(f0, fs, gain, Q);

    for (int i = 0; i < numFrames; ++i) {
        sf_read_short(infile, &inbuf[i*frameSize], frameSize);
        eq->process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE* outfile = sf_open("./speech_4000Hz_gain6.wav", SFM_WRITE, &info);
    sf_write_short(outfile, outbuf, bufSize);
    sf_close(outfile);

    delete eq;
    delete[] inbuf;
    delete[] outbuf;
}

TEST(Equalizer, 4kTone) {
    Equalizer *eq;

    int fs = 48000;
    int numFrames = 500;
    int frameSize = 1024;
    int amp = 16834;
    int bufSize = numFrames * frameSize;

    int16_t *inbuf = new int16_t[bufSize];
    int16_t *outbuf = new int16_t[bufSize];

    generate_sine_int16(inbuf, bufSize, fs, 4000, amp);

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    info.samplerate = fs;
    info.channels = 1;
    info.frames = 0;

    SNDFILE *outfile = sf_open("./tone_4kHz.wav", SFM_WRITE, &info);
    sf_write_short(outfile, inbuf, bufSize);
    sf_close(outfile);

    // EQ coefficient
    int f0 = 4000;
    float gain = -6;
    float Q = 0.5;
    eq = new Equalizer(f0, fs, gain, Q);

    for (int i = 0; i < numFrames; ++i) {
        eq->process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    outfile = sf_open("./tone_4kHz_supp6.wav", SFM_WRITE, &info);
    sf_write_short(outfile, outbuf, bufSize);
    sf_close(outfile);

    delete eq;
    delete[] inbuf;
    delete[] outbuf;
}
