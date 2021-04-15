
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "formant_shift.h"

using namespace ubnt;

/** Filter out high frequency vibration in spectrum by using cepstrum
 * @param inSpectrum The spectrum of signal generated by pffft ordered transform
 * @param outSpectrum The smoothed spectrum which is desired
 * @param frameSize The length of inSpectrum and outSpectrum buffer
 * 
 * @return NULL No return value
 */
void FormantShift::spectralSmooth(float *inSpectrum, float *outSpectrum, unsigned int frameSize) {
    logSpectrum[0] = logf(inSpectrum[0]);
    logSpectrum[1] = logf(inSpectrum[1]);
    for (unsigned int i = 2; i < frameSize; i+=2) {
        logSpectrum[i] = log(inSpectrum[i]);
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

void FormantShift::setShiftTone(float shiftTone) {
    this->shiftTone = shiftTone;
    return;
}

float FormantShift::getShiftTone() {
    return shiftTone;
}

void FormantShift::init() {
    if (bufferSize == 0U) return;
    processSize = bufferSize * 2;

    fft.init(processSize, Pffft::Transform::REAL);

    // Buffer for spectrum smoothing
    cepstrum = new float[processSize]();
    logSpectrum = new float[processSize]();

    // Buffer for windowing
    window = new float[processSize]();
    hanning(window, processSize);

    // Buffer for formant shift
    inBuffer = new float[processSize]();
    inBufferWin = new float[processSize]();
    inFrequency = new float[processSize]();
    inSpectrum = new float[processSize]();
    oriBuffer = new float[processSize]();
    oriBufferWin = new float[processSize]();
    oriSpectrum = new float[processSize]();
    outFrequency = new float[processSize]();
    outBuffer = new float[processSize]();
}

void FormantShift::release() {
    fft.release();

    // Buffer for spectrum smoothing
    freeBuffer(&cepstrum);
    freeBuffer(&logSpectrum);
    
    // Buffer for windowing
    freeBuffer(&window);

    // Buffer for formant shift
    freeBuffer(&inBuffer);
    freeBuffer(&inBufferWin);
    freeBuffer(&inFrequency);
    freeBuffer(&inSpectrum);
    freeBuffer(&oriBuffer);
    freeBuffer(&oriBufferWin);
    freeBuffer(&oriSpectrum);
    freeBuffer(&outFrequency);
    freeBuffer(&outBuffer);
}

int FormantShift::process(float* in, float *ori, float* out, unsigned int numSample) {
    // If input buffer size changes, the corresponding fft size and buffer
    // should also be reallocated.
    if (numSample != bufferSize) {
        bufferSize = numSample;
        release(); init();
    }

    // Very simple ring buffer behavior
    memcpy(inBuffer, inBuffer + bufferSize, sizeof(float) * bufferSize);
    memcpy(inBuffer + bufferSize, in, sizeof(float) * bufferSize);
    memcpy(oriBuffer, oriBuffer + bufferSize, sizeof(float) * bufferSize);
    memcpy(oriBuffer + bufferSize, ori, sizeof(float) * bufferSize);

    // inBuffer windowing
    for (unsigned int idx = 0; idx < processSize; idx++) {
        inBufferWin[idx] = inBuffer[idx] * window[idx];
        oriBufferWin[idx] = oriBuffer[idx] * window[idx];
    }

    // Calculate frequency & spectrum of inBuffer(pitch shifted signal) and oriBuffer(original signal)
    fft.fftOrder(inBufferWin, inFrequency, processSize);
    fft.fftOrder(oriBufferWin, oriSpectrum, processSize);

    // Get spectrum from frequency
    inSpectrum[0] = fabsf(inFrequency[0]);
    inSpectrum[1] = fabsf(inFrequency[1]);
    oriSpectrum[0] = fabsf(oriSpectrum[0]);
    oriSpectrum[1] = fabsf(oriSpectrum[1]);
    for (unsigned int idx = 2; idx < processSize; idx+=2) {
        inSpectrum[idx] = sqrt(inFrequency[idx] * inFrequency[idx] + inFrequency[idx + 1] * inFrequency[idx + 1]);
        inSpectrum[idx + 1] = 0.0f;
        oriSpectrum[idx] = sqrt(oriSpectrum[idx] * oriSpectrum[idx] + oriSpectrum[idx + 1] * oriSpectrum[idx + 1]);
        oriSpectrum[idx + 1] = 0.0f;
    }

    // Get smoothed formant from spectrum, the output also saved in spectrum variables
    spectralSmooth(inSpectrum, inSpectrum, processSize);
    spectralSmooth(oriSpectrum, oriSpectrum, processSize);

    // Calculate output frequency response
    outFrequency[0] = inFrequency[0] * oriSpectrum[0] / std::max<float>(inSpectrum[0], 0.000001f);
    outFrequency[1] = inFrequency[1] * oriSpectrum[1] / std::max<float>(inSpectrum[1], 0.000001f);
    for (unsigned int fIdx = 2; fIdx < processSize; fIdx += 2) {
        const float coef = oriSpectrum[fIdx] / std::max<float>(inSpectrum[fIdx], 0.000001f);
        outFrequency[fIdx] = inFrequency[fIdx] * coef;
        outFrequency[fIdx + 1] = inFrequency[fIdx + 1] * coef; 
    }

    // Copy the previous half output, which is saved at the later half of outBuffer
    memcpy(out, outBuffer + bufferSize, sizeof(float) * bufferSize);
    fft.ifftOrder(outFrequency, outBuffer, processSize);
    for (unsigned int idx = 0; idx < bufferSize; idx++) {
        out[idx] += outBuffer[idx];
    }
    return bufferSize;
}

void FormantShift::hanning(float *window, unsigned int numSample) {
    const float PI_2 = M_PI + M_PI;
    const float denom = 1.0f / static_cast<float>(numSample - 1);
    const int halfSample = numSample >> 1;
    for (unsigned int i = 0; i < numSample; i++) {
        window[i] = 0.5f * (1.0f - cos(PI_2 * static_cast<float>(i) * denom));
        window[numSample - i - 1] = window[i];
    }
    // If the numSample is odd number, the center index of window should be calculated
    if ((numSample & 1) == 1) {
        window[halfSample + 1] = 0.5f;
    }
}