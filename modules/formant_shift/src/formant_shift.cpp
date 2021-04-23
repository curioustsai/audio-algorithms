/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <climits>
#include <cassert>

#include "formant_shift.h"
#include "utils.h"

using namespace ubnt;

FormantShift::FormantShift() {
    init();
}
FormantShift::~FormantShift() {
    release();
}

/** Filter out high frequency vibration in spectrum by using cepstrum
 * @param inSpectrum The spectrum of signal generated by pffft ordered transform
 * @param outSpectrum The smoothed spectrum which is desired
 * @param frameSize The length of inSpectrum and outSpectrum buffer
 * 
 * @return NULL No return value
 */
void FormantShift::spectralSmooth(float *inSpectrum, float *outSpectrum, unsigned int frameSize) {
    logSpectrum[0] = logf(awayFromZero(inSpectrum[0]));
    logSpectrum[1] = logf(awayFromZero(inSpectrum[1]));
    for (unsigned int i = 2; i < frameSize; i+=2) {
        logSpectrum[i] = logf(awayFromZero(inSpectrum[i]));
        logSpectrum[i + 1] = 0.0f;
    }
    fft.ifftOrder(logSpectrum, cepstrum, frameSize);

    // Cut high-frequency part in cepstrum domain
    const unsigned int fCutIndex = 
        static_cast<unsigned int>(round(static_cast<float>(frameSize) * spectralSmoothRatio));
    memset(cepstrum + fCutIndex, 0, sizeof(float) * (frameSize - fCutIndex*2 + 1));
    fft.fftOrder(cepstrum, logSpectrum, frameSize);

    // Calculate otuput spectrum by exponential of logSpectrum
    outSpectrum[0] = exp(logSpectrum[0]);
    outSpectrum[1] = exp(logSpectrum[1]);
    for (unsigned int i = 2; i < frameSize; i+=2) {
        outSpectrum[i] = exp(logSpectrum[i]);
        outSpectrum[i + 1] = 0.0f;
    }
    
    return;
}
/** Generate signal delay of input signals. It would put desired peroid of
 * silence in front of input, which could protect the system from output exhaust.
 * This function should be called after formantShift init() and before process().
 * @param delayInSample The desired delay in samples, which can't be larger than 20480 by default.
 * @return  No return value
 */
void FormantShift::setDelay(unsigned int delayInSample) {
    if ((inOLA == nullptr) || (oriOLA == nullptr)) return;
    float *silence = new float[delayInSample]();
    inOLA->setInput(silence, delayInSample);
    oriOLA->setInput(silence, delayInSample);
}

/** Set the value which original spectrum would be shifted.
 * @param shiftTone The value that original spectrum would be shifted (in semi-tone)
 * @return  No return value
 */
void FormantShift::setShiftTone(float shiftTone) {
    this->shiftTone = shiftTone;
}

float FormantShift::getShiftTone() {
    return shiftTone;
}

void FormantShift::init() {
    bufferSize = DefaultBufferSize;
    processSize = bufferSize * 2;

    fft.init(processSize, Pffft::Transform::REAL);

    // Buffer for spectrum smoothing
    cepstrum = new float[processSize]();
    logSpectrum = new float[processSize]();

    oriFormantInterpo = new FormantInterpolate(processSize);

    // Buffer for formant shift
    inBuffer = new float[processSize]();
    inFrequency = new float[processSize]();
    inSpectrum = new float[processSize]();
    oriBuffer = new float[processSize]();
    oriSpectrum = new float[processSize]();
    outFrequency = new float[processSize]();
    outBuffer = new float[processSize]();

    inOLA = new OverlapAdd(bufferSize, 
                            OverlapAdd::WindowType::NONE,
                            OverlapAdd::WindowType::HANNING);

    oriOLA = new OverlapAdd(bufferSize, 
                            OverlapAdd::WindowType::NONE,
                            OverlapAdd::WindowType::HANNING);
}

void FormantShift::release() {
    fft.release();

    // Buffer for spectrum smoothing
    freeBuffer(&cepstrum);
    freeBuffer(&logSpectrum);

    // Buffer for formant shift
    freeBuffer(&inBuffer);
    freeBuffer(&inFrequency);
    freeBuffer(&inSpectrum);
    freeBuffer(&oriBuffer);
    freeBuffer(&oriSpectrum);
    freeBuffer(&outFrequency);
    freeBuffer(&outBuffer);
    
    if (oriFormantInterpo != nullptr) {
        delete oriFormantInterpo;
        oriFormantInterpo = nullptr;
    }
    
    if (inOLA != nullptr) {
        delete inOLA; inOLA = nullptr;
    }

    if (oriOLA != nullptr) {
        delete oriOLA; oriOLA = nullptr;
    }
}

int FormantShift::process(float* in, float *ori, float* out, unsigned int numSample) {
    // Ring buffer process. Put buffer in and get windowed buffer out
    // with twice the buffer length.
    inOLA->setInput(in, numSample);
    oriOLA->setInput(ori, numSample); 

    while ((inOLA->getInput(inBuffer, processSize) > 0) 
        && (oriOLA->getInput(oriBuffer, processSize) > 0)
    ) {
        // Calculate frequency & spectrum of inBuffer(pitch shifted signal) and oriBuffer(original signal)
        fft.fftOrder(inBuffer, inFrequency, processSize);
        fft.fftOrder(oriBuffer, oriSpectrum, processSize);

        // Get spectrum from frequency
        Pffft::getSpectrum(inFrequency, inSpectrum, processSize);
        Pffft::getSpectrum(oriSpectrum, oriSpectrum, processSize);

        // Get smoothed formant from spectrum, the output also saved in spectrum variables
        spectralSmooth(inSpectrum, inSpectrum, processSize);
        spectralSmooth(oriSpectrum, oriSpectrum, processSize);
        oriFormantInterpo->process(oriSpectrum, shiftTone, processSize);

        // Correct formant of the pitch shifted signal by morphing it into the formant of original signal
        outFrequency[0] = inFrequency[0] * oriSpectrum[0] / std::max<float>(inSpectrum[0], 0.000001f);
        outFrequency[1] = inFrequency[1] * oriSpectrum[1] / std::max<float>(inSpectrum[1], 0.000001f);
        for (unsigned int fIdx = 2; fIdx < processSize; fIdx += 2) {
            const float coef = oriSpectrum[fIdx] / std::max<float>(inSpectrum[fIdx], 0.000001f);
            outFrequency[fIdx] = inFrequency[fIdx] * coef;
            outFrequency[fIdx + 1] = inFrequency[fIdx + 1] * coef; 
        }

        // Copy the previous half output, which is saved at the later half of outBuffer
        fft.ifftOrder(outFrequency, outBuffer, processSize);
        inOLA->setOutput(outBuffer, processSize);
    }

    int outSample = inOLA->getOutput(out, numSample);

    return outSample;
}

int FormantShift::process(int16_t* in, int16_t *ori, int16_t* out, unsigned int numSample) {
    assert(numSample <= MAX_BUFFER_SIZE);

    const float normalizeCoef = 1.0f / (float)SHRT_MAX;

    for (unsigned int idx = 0; idx < numSample; idx++) {
        in_buf_t[idx] = (float)in[idx] * normalizeCoef;
        ori_buf_t[idx] = (float)ori[idx] * normalizeCoef;
    }

    process(in_buf_t, ori_buf_t, out_buf_t, numSample);

    for (unsigned int idx = 0; idx < numSample; idx++) {
        float out_f = out_buf_t[idx] * (float)SHRT_MAX;
        if (out_f > (float)SHRT_MAX) {
            out[idx] = (float)SHRT_MAX;
        }
        else if (out_f < (float)SHRT_MIN) {
            out[idx] = SHRT_MIN;
        }
        else {
            out[idx] = (int16_t)(out_f);
        }
    }

    return numSample;
}