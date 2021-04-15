/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once
#include <cstdint>

namespace ubnt {

class FormantShift {
public:
    FormantShift()=default;
    ~FormantShift()=default;
    void setShiftTone(float shiftTone);
    float getShiftTone();
    void init();
    void release();
    int32_t process(float* in, float* out, int32_t numSample);
private:
    // formant shift amount in semi-tones
    float shiftTone{0.0f};
};

}