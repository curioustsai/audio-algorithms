/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

namespace ubnt {

class FormantInterpolate {
public:
    FormantInterpolate()=delete;
    FormantInterpolate(unsigned int numSample);
    ~FormantInterpolate();
    int process(float *spectrum, const float shiftTone, unsigned int numSample);
private:
    float *inSpectrum{nullptr};
    static constexpr float semiToneScale{1.0f / 12.0f};
    unsigned int bufferSize{0};
    float shiftTone{0.0f};
    float shiftRatio{1.0f};
};

} // ubnt