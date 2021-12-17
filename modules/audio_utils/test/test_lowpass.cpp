#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "biquad.h"
#include "equalizer.h"
#include "lowpass.h"
#include "sndfile.h"
#include "tone_generator.h"

using namespace ubnt;

TEST(SosFilter, Sweep_LPF4kHz) {
    SosFilter sosfilt;

    // lpf 4kHz for fs=48kHz
    // from scipy import signal
    // sos_hpf = signal.cheby1(4, 3, 4000/24000, 'low', output='sos')
    const float lpf_4kHz[2][5] = {
        {5.17335477e-04, 1.03467095e-03, 5.17335477e-04, -1.75391386e+00, 8.03975970e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.68424486e+00, 9.17796589e-01}};

    int frameSize = 1024;
    int numFrames = 500;
    int totalLength = frameSize * numFrames;
    int samplerate = 48000;
    int f1 = 0;
    int f2 = 24000;
    float amplitude = 0.95;

    float *inbuf = new float[totalLength];
    float *outbuf = new float[totalLength];

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info.samplerate = samplerate;
    info.channels = 1;
    info.frames = 0;

    generate_chirp(inbuf, totalLength, samplerate, f1, f2, amplitude);
    SNDFILE *infile = sf_open("./Sweep_LPF4kHz_before.wav", SFM_WRITE, &info);
    sf_write_float(infile, inbuf, totalLength);
    sf_close(infile);

    sosfilt.reset(lpf_4kHz, 2);
    sosfilt.process(inbuf, outbuf, totalLength);
    for (int i = 0; i < numFrames; ++i) {
        sosfilt.process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./Sweep_LPF4kHz_after.wav", SFM_WRITE, &info);
    sf_write_float(outfile, outbuf, totalLength);
    sf_close(outfile);

    delete[] inbuf;
    delete[] outbuf;
}

TEST(SosFilter, Sweep_LPF4kHz_int16) {
    SosFilter sosfilt;

    // lpf 4kHz for fs=48kHz
    // from scipy import signal
    // sos_hpf = signal.cheby1(4, 3, 4000/24000, 'low', output='sos')
    const float lpf_4kHz[2][5] = {
        {5.17335477e-04, 1.03467095e-03, 5.17335477e-04, -1.75391386e+00, 8.03975970e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.68424486e+00, 9.17796589e-01}};

    int frameSize = 1024;
    int numFrames = 500;
    int totalLength = frameSize * numFrames;
    int samplerate = 48000;
    int f1 = 0;
    int f2 = 24000;
    int16_t amplitude = 32000;

    int16_t *inbuf = new int16_t[totalLength];
    int16_t *outbuf = new int16_t[totalLength];

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    info.samplerate = samplerate;
    info.channels = 1;
    info.frames = 0;

    generate_chirp_int16(inbuf, totalLength, samplerate, f1, f2, amplitude);
    SNDFILE *infile = sf_open("./Sweep_LPF4kHz_int16_before.wav", SFM_WRITE, &info);
    sf_write_short(infile, inbuf, totalLength);
    sf_close(infile);

    sosfilt.reset(lpf_4kHz, 2);
    sosfilt.process(inbuf, outbuf, totalLength);
    for (int i = 0; i < numFrames; ++i) {
        sosfilt.process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./Sweep_LPF4kHz_int16_after.wav", SFM_WRITE, &info);
    sf_write_short(outfile, outbuf, totalLength);
    sf_close(outfile);

    delete[] inbuf;
    delete[] outbuf;
}

TEST(LowPassFilter, IterativeFilters) {
    int frameSize = 1024;
    int numFrames = 500;
    int totalLength = frameSize * numFrames;
    int16_t amplitude = 16384;

    int16_t *inbuf = new int16_t[totalLength];
    int16_t *outbuf = new int16_t[totalLength];

    using SampleRate = ubnt::LowPassFilter::SampleRate;
    using CutoffFreq = ubnt::LowPassFilter::CutoffFreq;

    std::string strCutoffFreq[] = {"4kHz", "5kHz", "6kHz", "7kHz"};
    std::string strSampleRate[] = {"16kHz", "32kHz", "48kHz"};
    int samplerateTable[] = {16000, 32000, 48000};

    for (SampleRate fs = SampleRate::Fs_16kHz; fs != SampleRate::Fs_ALL;
         fs = static_cast<SampleRate>(static_cast<int>(fs) + 1)) {

        int index_fs = static_cast<int>(fs);
        int samplerate = samplerateTable[index_fs];
        int f1 = 0;
        int f2 = samplerate / 2;

        SF_INFO info;
        info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
        info.samplerate = samplerate;
        info.channels = 1;
        info.frames = 0;

        generate_chirp_int16(inbuf, totalLength, samplerate, f1, f2, amplitude);

        std::string infileName = "./LPF_" + strSampleRate[index_fs] + "_before.wav";
        SNDFILE *infile = sf_open(infileName.c_str(), SFM_WRITE, &info);
        sf_write_short(infile, inbuf, totalLength);
        sf_close(infile);

        for (CutoffFreq fc = CutoffFreq::Fc_4kHz; fc != CutoffFreq::Fc_ALL;
             fc = static_cast<CutoffFreq>(static_cast<int>(fc) + 1)) {

            int index_fc = static_cast<int>(fc);
            LowPassFilter *lpf = new LowPassFilter(fs, fc);
            lpf->process(inbuf, outbuf, totalLength);
            for (int i = 0; i < numFrames; ++i) {
                lpf->process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
            }

            std::string outfileName =
                "./LPF_" + strSampleRate[index_fs] + "_fc_" + strCutoffFreq[index_fc] + ".wav";
            SNDFILE *outfile = sf_open(outfileName.c_str(), SFM_WRITE, &info);
            sf_write_short(outfile, outbuf, totalLength);
            sf_close(outfile);
            delete lpf;
        }
    }

    delete[] inbuf;
    delete[] outbuf;
}
