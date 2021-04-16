#include <cmath>
#include <cstring>
#include "overlapAdd.h"
#include "utils.h"

using namespace ubnt;

OverlapAdd::OverlapAdd(
    unsigned int frameSize,
    WindowType inType,
    WindowType outType) :
    bufferSize(frameSize),
    bufPoolSize(frameSize * 2),
    inWindowType(inType),
    outWindowType(outType)
{
    init();
}

OverlapAdd::~OverlapAdd() {
    release();
}

void OverlapAdd::init() {
    if (bufferSize == 0) { release(); return; }

    if ((inWindow != nullptr) || 
        (outWindow != nullptr) ||
        (inputPool != nullptr) ||
        (outputPool != nullptr)) 
    {
            release();
    }

    inWindow = new float[bufPoolSize]();
    outWindow = new float[bufPoolSize]();
    setWindow(inWindow, inWindowType);
    setWindow(outWindow, outWindowType);

    inputPool = new float[bufPoolSize]();
    outputPool = new float[bufPoolSize]();
    inPoolReadIdx = 0U;
    inPoolWriteIdx = bufferSize;
    outPoolReadIdx = 0;
    outPoolWriteIdx = 0;
}

void OverlapAdd::release() {
    freeBuffer(&inWindow);
    freeBuffer(&outWindow);
    freeBuffer(&inputPool);
    freeBuffer(&outputPool);
}

void OverlapAdd::setWindow(float *window, WindowType type) {
    switch (type) {
    case WindowType::HANNING:
        hanning(window, bufPoolSize);
        break;
    default:
        for (unsigned int idx = 0; idx < bufPoolSize; idx++) {
            window[idx] = 1.0f;
        }
        break;
    }
}

void OverlapAdd::hanning(float *window, unsigned int numSample) {
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

int OverlapAdd::setInput(float *input, unsigned int frameSize) {
    if(bufferSize != frameSize) return -1;

    if (inPoolWriteIdx <= (bufPoolSize - bufferSize)) {
        memcpy(&(inputPool[inPoolWriteIdx]), input, sizeof(float) * bufferSize);
        inPoolWriteIdx = circIndex(inPoolWriteIdx  + bufferSize, bufPoolSize);
    }
    else {
        memcpy(&(inputPool[inPoolWriteIdx]), input, sizeof(float) * (bufPoolSize - inPoolWriteIdx));
        memcpy(inputPool, &(input[bufPoolSize - inPoolWriteIdx]), sizeof(float) * (bufferSize - (bufPoolSize - inPoolWriteIdx)));
        inPoolWriteIdx = circIndex(bufferSize - (bufPoolSize - inPoolWriteIdx), bufPoolSize);
    }
    return bufferSize;
}

int OverlapAdd::getInput(float *input, unsigned int frameSize) {
    if (bufPoolSize != frameSize) return -1;

    if (inWindowType != WindowType::NONE) {
        unsigned int idx = 0;
        unsigned int inIdx = inPoolReadIdx;
        while (idx < bufPoolSize) {
            input[idx] = inputPool[inIdx] * inWindow[idx];
            idx++;
            inIdx = circIndex(inIdx + 1, bufPoolSize);
        }
    }
    else {
        memcpy(input, &(inputPool[inPoolReadIdx]), sizeof(float) * (bufPoolSize - inPoolReadIdx));
        memcpy(&(input[bufPoolSize - inPoolReadIdx]), inputPool, sizeof(float) * inPoolReadIdx);
    }
    inPoolReadIdx = circIndex(inPoolReadIdx + bufferSize, bufPoolSize);

    return bufPoolSize;
}

int OverlapAdd::setOutput(float *output, unsigned int frameSize) {
    if (frameSize != bufPoolSize) return -1;

    unsigned int idx = 0;
    unsigned int outIdx = outPoolWriteIdx;
    while (idx < bufPoolSize) {
        float out = 
            (outWindowType == WindowType::NONE) ? output[idx] : output[idx] * outWindow[idx];

        if (idx < bufPoolSize - bufferSize) {
            outputPool[outIdx] += out;
        }
        else {
            outputPool[outIdx] = out;
        }
        idx++;
        outIdx = circIndex(outIdx + 1, bufPoolSize);
    }
    
    outPoolWriteIdx = circIndex(outPoolWriteIdx + bufPoolSize - bufferSize, bufPoolSize);

    return bufPoolSize;
}

int OverlapAdd::getOutput(float *output, unsigned int frameSize) {
    if (frameSize != bufferSize) return -1;

    if (outPoolReadIdx <= (bufPoolSize - bufferSize)) {
        memcpy(output, &(outputPool[outPoolReadIdx]), sizeof(float) * bufferSize);
        outPoolReadIdx = circIndex(outPoolReadIdx + bufferSize, bufPoolSize);
    }
    else {
        memcpy(output, &(outputPool[outPoolReadIdx]), 
            sizeof(float) * (bufPoolSize - outPoolReadIdx));
        memcpy(&(output[bufPoolSize - outPoolReadIdx]), outputPool,
            sizeof(float) * (bufferSize - (bufPoolSize - outPoolReadIdx)));
        outPoolReadIdx = circIndex(bufferSize - (bufPoolSize - outPoolReadIdx), bufPoolSize);
    }

    return bufferSize;
}