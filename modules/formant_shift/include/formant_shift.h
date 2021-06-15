/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#ifndef __FORMANT_SHIFT_H__
#define __FORMANT_SHIFT_H__

#include <memory>
#include <algorithm>
#include <cstdint>

namespace ubnt {

class Pffft;
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
    float getShiftTone();
    int process(float* in, float *ori, float* out, unsigned int numSample);
    int process(int16_t* in, int16_t *ori, int16_t* out, unsigned int numSample);
private:
    std::unique_ptr<Pffft> fft;
    unsigned int sampleRate{48000U};
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

    static constexpr unsigned int MAX_BUFFER_SIZE = 16384;
    float in_buf_t[MAX_BUFFER_SIZE];
    float ori_buf_t[MAX_BUFFER_SIZE];
    float out_buf_t[MAX_BUFFER_SIZE];

    inline float awayFromZero(float input) {
        const float Epsilon = 1e-6f;
        return std::max<float>(input, Epsilon);
    }
};

} // ubnt
#endif // __FORMANT_SHIFT_H__
