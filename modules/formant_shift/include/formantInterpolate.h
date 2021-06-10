/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#ifndef __FORMANT_INTERPOLATE_H__
#define __FORMANT_INTERPOLATE_H__

namespace ubnt {

class FormantInterpolate {
public:
    FormantInterpolate()=delete;
    FormantInterpolate(unsigned int numSample);
    ~FormantInterpolate();
    int process(float *spectrum, const float shiftTone, unsigned int numSample);
private:
    float *inSpectrum{nullptr};
    unsigned int bufferSize{0};
    const float semiToneScale{1.0f / 12.0f};
    float shiftTone{0.0f};
    float shiftRatio{1.0f};
};

} // ubnt

#endif // __FORMANT_INTERPOLATE_H__
