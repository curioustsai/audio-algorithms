/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include <cmath>
#include <cstring>
#include "overlapAdd.h"

using namespace ubnt;

OverlapAdd::OverlapAdd(
    unsigned int frameSize,
    WindowType inType,
    WindowType outType) :
    halfBufferSize(frameSize),
    bufferSize(frameSize * 2),
    bufPoolSize(frameSize * Capacity),
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

    inWindow = new float[bufferSize]();
    outWindow = new float[bufferSize]();
    setWindow(inWindow, inWindowType);
    setWindow(outWindow, outWindowType);

    inputPool = new float[bufPoolSize]();
    outputPool = new float[bufPoolSize]();
    inPoolReadIdx = 0U;
    inPoolWriteIdx = halfBufferSize;
    inBufferedSize = halfBufferSize;
    outPoolReadIdx = 0;
    outPoolWriteIdx = 0;
    outBufferedSize = 0;
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
        hanning(window, bufferSize);
        break;
    default:
        for (unsigned int idx = 0; idx < bufferSize; idx++) {
            window[idx] = 1.0f;
        }
        break;
    }
}

void OverlapAdd::hanning(float *window, unsigned int numSample) {
    const float PI_2 = M_PI + M_PI;
    const float denom = 1.0f / static_cast<float>(numSample - 1);
    const unsigned int halfSample = numSample >> 1;
    for (unsigned int i = 0; i < halfSample; i++) {
        window[i] = 0.5f * (1.0f - cos(PI_2 * static_cast<float>(i) * denom));
        window[numSample - i - 1] = window[i];
    }
    // If the numSample is odd number, the center index of window should be calculated
    if ((numSample & 1) == 1) {
        window[halfSample + 1] = 0.5f;
    }
}

int OverlapAdd::setInput(float *input, unsigned int frameSize) {
    if (inBufferedSize + frameSize > bufPoolSize) {
        return -1;
    }

    if (inPoolWriteIdx <= (bufPoolSize - frameSize)) {
        memcpy(&(inputPool[inPoolWriteIdx]), input, sizeof(float) * frameSize);
        inPoolWriteIdx = circIndex(inPoolWriteIdx  + frameSize, bufPoolSize);
    }
    else {
        memcpy(&(inputPool[inPoolWriteIdx]), input, sizeof(float) * (bufPoolSize - inPoolWriteIdx));
        memcpy(inputPool, &(input[bufPoolSize - inPoolWriteIdx]), sizeof(float) * (frameSize - (bufPoolSize - inPoolWriteIdx)));
        inPoolWriteIdx = circIndex(frameSize - (bufPoolSize - inPoolWriteIdx), bufPoolSize);
    }
    inBufferedSize += frameSize;
    return frameSize;
}

int OverlapAdd::getInput(float *input, unsigned int frameSize) {
    if (bufferSize != frameSize) return -1;
    if (inBufferedSize < frameSize) return -1;

    if (inWindowType != WindowType::NONE) {
        unsigned int idx = 0;
        unsigned int inIdx = inPoolReadIdx;
        while (idx < bufferSize) {
            input[idx] = inputPool[inIdx] * inWindow[idx];
            idx++;
            inIdx = circIndex(inIdx + 1, bufPoolSize);
        }
    }
    else {
        if (inPoolReadIdx + bufferSize <= bufPoolSize) {
            memcpy(input, &(inputPool[inPoolReadIdx]), sizeof(float) * bufferSize);
        }
        else {
            memcpy(input, &(inputPool[inPoolReadIdx]),
                sizeof(float) * (bufPoolSize - inPoolReadIdx));
            memcpy(&(input[bufPoolSize - inPoolReadIdx]), inputPool,
                sizeof(float) * (bufferSize - (bufPoolSize - inPoolReadIdx)));
        }
    }
    inPoolReadIdx = circIndex(inPoolReadIdx + halfBufferSize, bufPoolSize);
    inBufferedSize -= halfBufferSize;

    return bufferSize;
}

int OverlapAdd::setOutput(float *output, unsigned int frameSize) {
    if (frameSize != bufferSize) return -1;
    if (outBufferedSize + frameSize > bufPoolSize) return -1;

    unsigned int idx = 0;
    unsigned int outIdx = outPoolWriteIdx;
    while (idx < bufferSize) {
        float out = 
            (outWindowType == WindowType::NONE) ? output[idx] : output[idx] * outWindow[idx];

        if (idx < halfBufferSize) {
            outputPool[outIdx] += out;
        }
        else {
            outputPool[outIdx] = out;
        }
        idx++;
        outIdx = circIndex(outIdx + 1, bufPoolSize);
    }
    outPoolWriteIdx = circIndex(outPoolWriteIdx + halfBufferSize, bufPoolSize);
    outBufferedSize += halfBufferSize;

    return bufferSize;
}

int OverlapAdd::getOutput(float *output, unsigned int frameSize) {
    if (outBufferedSize < frameSize) return -1;

    if (outPoolReadIdx <= (bufPoolSize - frameSize)) {
        memcpy(output, &(outputPool[outPoolReadIdx]), sizeof(float) * frameSize);
        outPoolReadIdx = circIndex(outPoolReadIdx + frameSize, bufPoolSize);
    }
    else {
        memcpy(output, &(outputPool[outPoolReadIdx]), 
            sizeof(float) * (bufPoolSize - outPoolReadIdx));
        memcpy(&(output[bufPoolSize - outPoolReadIdx]), outputPool,
            sizeof(float) * (frameSize - (bufPoolSize - outPoolReadIdx)));
        outPoolReadIdx = circIndex(frameSize - (bufPoolSize - outPoolReadIdx), bufPoolSize);
    }
    outBufferedSize -= frameSize;

    return frameSize;
}
