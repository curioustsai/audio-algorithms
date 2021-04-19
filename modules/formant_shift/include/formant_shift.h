/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#ifndef __FORMANT_SHIFT_H__
#define __FORMANT_SHIFT_H__

#include "pffftwrap.h"
#include "overlapAdd.h"
#include "formantInterpolate.h"

namespace ubnt {

class FormantShift {
public:
    FormantShift()=default;
    ~FormantShift()=default;
    void init();
    void release();
    void setShiftTone(float shiftTone);
    float getShiftTone();
    int process(float* in, float *ori, float* out, unsigned int numSample);
private:
    Pffft fft;
    unsigned int bufferSize{0U};
    unsigned int processSize{0U};
    
    void spectralSmooth(float *inSpectrum, float *outSpectrum, unsigned int frameSize);
    const float spectralSmoothRatio{1.0f/16.0f};
    float *logSpectrum{nullptr};
    float *cepstrum{nullptr};

    // formant shift amount in semi-tones
    FormantInterpolate *oriFormantInterpo{nullptr};
    float shiftTone{0.0f};

    float *inBuffer{nullptr};
    float *inFrequency{nullptr};
    float *inSpectrum{nullptr};
    float *oriBuffer{nullptr};
    float *oriSpectrum{nullptr};
    float *outFrequency{nullptr};
    float *outBuffer{nullptr};
    OverlapAdd *inOLA{nullptr};
    OverlapAdd *oriOLA{nullptr};
};

} // ubnt
#endif // __FORMANT_SHIFT_H__
