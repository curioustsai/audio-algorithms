/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once
#include <cmath>

namespace ubnt {

class Gain {
public:
    Gain() = delete;

    Gain(float gaindB) { _gain = powf(10, gaindB / 20.f); };

    ~Gain() = default;

    void Process(float* buffer, const int num) {
        for (int i = 0; i < num; ++i) { buffer[i] *= _gain; }
    };
    void Process(int16_t* buffer, const int num) {
        for (int i = 0; i < num; ++i) {
            float temp = buffer[i] * _gain;
            buffer[i] = (int16_t)(temp + 0.5f);
        }
    }

private:
    float _gain{1.0f};
};

} // namespace ubnt
