/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "equalizer.h"
#include <cmath>
#include <cstring>
#include <stdio.h>

namespace ubnt {

Equalizer::Equalizer(int f0, int fs, float gain, float Q) : _f0(f0), _fs(fs), _gain(gain), _Q(Q) {
    float amp = sqrt(powf(10, (gain / 20)));
    float w0 = 2 * M_PI * f0 / fs;
    float alpha = sinf(w0) / (2 * Q);

    float b0 = 1 + alpha * amp;
    float b1 = -2 * cosf(w0);
    float b2 = 1 - alpha * amp;
    float a0 = 1 + alpha / amp;
    float a1 = -2 * cosf(w0);
    float a2 = 1 - alpha / amp;

    // PeakEQ coefficient
    float a0_inv = 1 / a0;
    float ba[5] = {b0 * a0_inv, b1 * a0_inv, b2 * a0_inv, a1 * a0_inv, a2 * a0_inv};

    Reset(ba, 5);
}

} // namespace ubnt
