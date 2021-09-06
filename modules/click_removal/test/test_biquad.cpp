#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "biquad.h"

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

    biquad.reset(ba, 5);
    biquad.process(inbuf, outbuf, 16);

    for (int i = 0; i < frameSize; i++) { ASSERT_LT(fabs(outbuf[i] - ans[i]), 1e-3); }

    delete[] inbuf;
    delete[] outbuf;
}

TEST(SosFilter, Process) {
    SosFilter sosfilt;
    // const float ba[2][5] = {{0.32483446, -1.29933784, 1.94900676, -1.29933784, 0.32483446},
    //                         {1., -2.28624977, 2.39800304, -1.27756663, 0.3796345}};
    const float ba[2][5] = {{0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.41692846},
                            {1., -2., 1., -1.62913992, 0.9105507}};
    int frameSize = 16;
    float *inbuf = new float[frameSize];
    float *outbuf = new float[frameSize];
    float ans[] = {0.,          0.32483446,  0.09298379, -0.2415354, -0.36018799, -0.24880247,
                   -0.04897011, 0.11620218,  0.20197575, 0.21500525, 0.174264,    0.09675092,
                   0.00131773,  -0.08798588, -0.1468685, -0.15983416};

    sosfilt.reset(ba, 2);
    for (int i = 0; i < frameSize; i++) { inbuf[i] = i; }
    sosfilt.process(inbuf, outbuf, frameSize);

    for (int i = 0; i < frameSize; i++) {
        // printf("%d\t%f\t%f\n", i, outbuf[i], ans[i]);
        ASSERT_LT(fabs(outbuf[i] - ans[i]), 1e-3);
    }

    delete[] inbuf;
    delete[] outbuf;
}
