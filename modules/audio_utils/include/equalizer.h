/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once
#include "biquad.h"
#include <cmath>

namespace ubnt {

class Equalizer : public Biquad {
public:
    Equalizer() = delete;
    Equalizer(int f0, int fs, float gain, float Q);
    ~Equalizer() = default;

private:
    int _f0{1000};
    int _fs{48000};
    float _gain{6.0};
    float _Q{1.0};
};

} // namespace ubnt
