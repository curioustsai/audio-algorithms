/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <memory>
#include <algorithm>
#include <cstdint>
#include "pffftwrap.h"
namespace ubnt {

class FormantInterpolate;
class OverlapAdd; 

class FormantShift {
public:
    FormantShift() = delete;
    FormantShift(unsigned int sampleRate);
    ~FormantShift();
    void init(unsigned int sampleRate);
    void release();
    void setDelay(unsigned int delayInSample);
    void setShiftTone(float shiftTone);
    float getShiftTone() const;
    int process(const float* in, const float *ori, float* out, unsigned int numSample);
    int process(const int16_t* in, const int16_t *ori, int16_t* out, unsigned int numSample);
private:
    void spectralSmooth(const float *inSpectrum, float *outSpectrum, unsigned int frameSize);
    inline float awayFromZero(float input) {
        static constexpr float Epsilon = 1e-6f;
        return std::max<float>(input, Epsilon);
    }

    Pffft fft;
    FormantInterpolate *oriFormantInterpo{nullptr};
    OverlapAdd *inOLA{nullptr};
    OverlapAdd *oriOLA{nullptr};

    float *inBuffer{nullptr};
    float *inFrequency{nullptr};
    float *inSpectrum{nullptr};
    float *oriBuffer{nullptr};
    float *oriSpectrum{nullptr};
    float *outFrequency{nullptr};
    float *outBuffer{nullptr};
    float *logSpectrum{nullptr};
    float *cepstrum{nullptr};

    static constexpr unsigned int MAX_BUFFER_SIZE = 16384;
    unsigned int sampleRate{48000U};
    unsigned int bufferSize{0U};
    unsigned int processSize{0U};    
    float shiftTone{0.0f};
    float in_buf_t[MAX_BUFFER_SIZE];
    float ori_buf_t[MAX_BUFFER_SIZE];
    float out_buf_t[MAX_BUFFER_SIZE];
};

} // ubnt