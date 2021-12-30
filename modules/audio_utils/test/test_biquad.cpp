#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "biquad.h"
#include "equalizer.h"
#include "sndfile.h"
#include "tone_generator.h"

using namespace ubnt;

TEST(Biquad, Process) {
    Biquad biquad;
    int frameSize = 16;
    float *inbuf = new float[frameSize];
    float *outbuf = new float[frameSize];

    for (int i = 0; i < frameSize; i++) { inbuf[i] = i; }

    float ba[5] = {0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.4169284};

    float ans[] = {0.00000000e+00,  3.24834460e-01,  2.13451923e-01,  4.82864962e-03,
                   -8.58212156e-02, -5.84071673e-02, -2.59862282e-03, 2.26440262e-02,
                   1.59630523e-02,  1.04854130e-03,  -5.96644303e-03, -4.35777513e-03,
                   -3.75957416e-04, 1.56983489e-03,  1.18830129e-03,  1.26335735e-04};

    biquad.Reset(ba, 5);
    biquad.Process(inbuf, outbuf, 16);

    for (int i = 0; i < frameSize; i++) { ASSERT_LT(fabs(outbuf[i] - ans[i]), 1e-3); }

    delete[] inbuf;
    delete[] outbuf;
}

TEST(SosFilter, Process) {
    SosFilter sosfilt;
    const float ba[2][5] = {{0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.41692846},
                            {1., -2., 1., -1.62913992, 0.9105507}};
    int frameSize = 16;
    float *inbuf = new float[frameSize];
    float *outbuf = new float[frameSize];
    float ans[16] = {0.,          0.32483446,  0.09298379, -0.2415354, -0.36018799, -0.24880247,
                     -0.04897011, 0.11620218,  0.20197575, 0.21500525, 0.174264,    0.09675092,
                     0.00131773,  -0.08798588, -0.1468685, -0.15983416};

    sosfilt.Reset(ba, 2);
    for (int i = 0; i < frameSize; i++) { inbuf[i] = i; }
    sosfilt.Process(inbuf, outbuf, frameSize);

    for (int i = 0; i < frameSize; i++) {
        // printf("%d\t%f\t%f\n", i, outbuf[i], ans[i]);
        ASSERT_LT(fabs(outbuf[i] - ans[i]), 1e-3);
    }

    delete[] inbuf;
    delete[] outbuf;
}

TEST(SosFilter, HPF1kHz) {
    SosFilter sosfilt;

    // hpf 1kHz for fs=48kHz
    // from scipy import signal
    // sos_hpf = signal.cheby1(4, 3, 1000/24000, 'high', output='sos')
    const float hpf_1kHz[2][5] = {{0.60030784, -1.20061568, 0.60030784, -1.68713702, 0.76275989},
                                  {1., -2., 1., -1.95698292, 0.97569048}};

    int frameSize = 16;
    float *inbuf = new float[frameSize];
    float *outbuf = new float[frameSize];
    float ans[16] = {0.,          0.60030784,  0.98697809,  1.1713336,   1.178229,    1.04111067,
                     0.79757528,  0.48564276,  0.14085483,  -0.20577818, -0.52902836, -0.80999444,
                     -1.03606497, -1.20043981, -1.30134908, -1.34109836};

    for (int i = 0; i < 16; i++) { inbuf[i] = (float)i; }
    sosfilt.Reset(hpf_1kHz, 2);
    sosfilt.Process(inbuf, outbuf, 16);

    for (int i = 0; i < frameSize; i++) {
        printf("%d\t%f\t%f\n", i, outbuf[i], ans[i]);
        ASSERT_LT(fabs(outbuf[i] - ans[i]), 1e-3);
    }

    delete[] inbuf;
    delete[] outbuf;
}

TEST(SosFilter, LPF1kHz) {
    SosFilter sosfilt;

    // lpf 1kHz for fs=48kHz
    // from scipy import signal
    // sos_lpf = signal.cheby1(4, 3, 1000/24000, 'low', output='sos')
    const float lpf_1kHz[2][5] = {
        {2.21649547e-06, 4.43299093e-06, 2.21649547e-06, -1.94427324e+00, 9.47549838e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.96271306e+00, 9.78001501e-01}};

    int frameSize = 16;
    float *inbuf = new float[frameSize];
    float *outbuf = new float[frameSize];
    float ans[16] = {0.00000000e+00, 2.21649547e-06, 2.19587902e-05, 1.10746881e-04,
                     3.85872611e-04, 1.05866521e-03, 2.45871894e-03, 5.05380838e-03,
                     9.46529467e-03, 1.64789054e-02, 2.70508494e-02, 4.23093039e-02,
                     6.35513889e-02, 9.22358102e-02, 1.29971422e-01, 1.78502021e-01};

    for (int i = 0; i < 16; i++) { inbuf[i] = (float)i; }
    sosfilt.Reset(lpf_1kHz, 2);
    sosfilt.Process(inbuf, outbuf, 16);

    for (int i = 0; i < frameSize; i++) {
        printf("%d\t%f\t%f\n", i, outbuf[i], ans[i]);
        ASSERT_LT(fabs(outbuf[i] - ans[i]), 1e-3);
    }

    delete[] inbuf;
    delete[] outbuf;
}
