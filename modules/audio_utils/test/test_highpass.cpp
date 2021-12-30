#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "biquad.h"
#include "equalizer.h"
#include "highpass.h"
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

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    info.samplerate = samplerate;
    info.channels = 1;
    info.frames = 0;

    generate_chirp(inbuf, totalLength, samplerate, f1, f2, amplitude);
    SNDFILE *infile = sf_open("./Sweep_HPF1kHz_before.wav", SFM_WRITE, &info);
    sf_write_float(infile, inbuf, totalLength);
    sf_close(infile);

    sosfilt.Reset(hpf_1kHz, 2);
    sosfilt.Process(inbuf, outbuf, totalLength);
    for (int i = 0; i < numFrames; ++i) {
        sosfilt.Process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./Sweep_HPF1kHz_after.wav", SFM_WRITE, &info);
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

    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    info.samplerate = samplerate;
    info.channels = 1;
    info.frames = 0;

    generate_chirp_int16(inbuf, totalLength, samplerate, f1, f2, amplitude);
    SNDFILE *infile = sf_open("./Sweep_HPF1kHz_int16_before.wav", SFM_WRITE, &info);
    sf_write_short(infile, inbuf, totalLength);
    sf_close(infile);

    sosfilt.Reset(hpf_1kHz, 2);
    sosfilt.Process(inbuf, outbuf, totalLength);
    for (int i = 0; i < numFrames; ++i) {
        sosfilt.Process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
    }

    SNDFILE *outfile = sf_open("./Sweep_HPF1kHz_int16_after.wav", SFM_WRITE, &info);
    sf_write_short(outfile, outbuf, totalLength);
    sf_close(outfile);

    delete[] inbuf;
    delete[] outbuf;
}

TEST(HighPassFilter, IterativeFilters) {
    int frameSize = 1024;
    int numFrames = 500;
    int totalLength = frameSize * numFrames;
    int f1 = 0;
    int f2 = 4000;
    int16_t amplitude = 16384;

    int16_t *inbuf = new int16_t[totalLength];
    int16_t *outbuf = new int16_t[totalLength];

    using SampleRate = ubnt::HighPassFilter::SampleRate;
    using CutoffFreq = ubnt::HighPassFilter::CutoffFreq;

    std::string strCutoffFreq[] = {"100Hz", "150Hz", "200Hz", "250Hz", "300Hz", "400Hz", "500Hz"};
    std::string strSampleRate[] = {"8kHz", "16kHz", "32kHz", "48kHz"};
    int samplerateTable[] = {8000, 16000, 32000, 48000};

    for (SampleRate fs = SampleRate::Fs_8kHz; fs != SampleRate::Fs_ALL;
         fs = static_cast<SampleRate>(static_cast<int>(fs) + 1)) {

        int index_fs = static_cast<int>(fs);
        int samplerate = samplerateTable[index_fs];

        SF_INFO info;
        info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
        info.samplerate = samplerate;
        info.channels = 1;
        info.frames = 0;

        generate_chirp_int16(inbuf, totalLength, samplerate, f1, f2, amplitude);

        std::string infileName = "./HPF_" + strSampleRate[index_fs] + "_before.wav";
        SNDFILE *infile = sf_open(infileName.c_str(), SFM_WRITE, &info);
        sf_write_short(infile, inbuf, totalLength);
        sf_close(infile);

        for (CutoffFreq fc = CutoffFreq::Fc_100Hz; fc != CutoffFreq::Fc_ALL;
             fc = static_cast<CutoffFreq>(static_cast<int>(fc) + 1)) {

            int index_fc = static_cast<int>(fc);
            HighPassFilter *hpf = new HighPassFilter(fs, fc);
            hpf->Process(inbuf, outbuf, totalLength);
            for (int i = 0; i < numFrames; ++i) {
                hpf->Process(&inbuf[i * frameSize], &outbuf[i * frameSize], frameSize);
            }

            std::string outfileName =
                "./HPF_" + strSampleRate[index_fs] + "_fc_" + strCutoffFreq[index_fc] + ".wav";
            SNDFILE *outfile = sf_open(outfileName.c_str(), SFM_WRITE, &info);
            sf_write_short(outfile, outbuf, totalLength);
            sf_close(outfile);
            delete hpf;
        }
    }

    delete[] inbuf;
    delete[] outbuf;
}
