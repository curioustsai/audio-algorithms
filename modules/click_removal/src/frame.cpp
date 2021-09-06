/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "frame.h"
#include <cmath>
#include <cstring>
#include <iostream>

Frame::Frame() { reset(_frameSize); }

Frame::Frame(const int frameSize) : _frameSize(frameSize) { reset(_frameSize); };

void Frame::reset(const int frameSize) {
    _frameSize = frameSize;

    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
    _data = new float[_frameSize] {0};
}

Frame::~Frame() {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
}

bool Frame::updateFrame(const float* data, const int num) {
    if (num != _frameSize) {
        std::cout << "Update Frame Failed" << std::endl;
        return false;
    }

    memcpy(_data, data, num * sizeof(float));

    return true;
}

bool Frame::getOutput(float* buf, const int num) {
    if (num != _frameSize) {
        std::cout << "GetOuput Failed" << std::endl;
        return false;
    }

    memcpy(buf, _data, num * sizeof(float));

    return true;
}

float Frame::getPowerMean() {
    float mean = 0;
    for (int i = 0; i < _frameSize; i++) { mean += (_data[i] * _data[i]); }
    mean /= _frameSize;

    return mean;
}

float Frame::getPowerdB() {
    float mean = 0;
    for (int i = 0; i < _frameSize; i++) { mean += (_data[i] * _data[i]); }

    mean /= _frameSize;

    return 10 * log10f(mean);
}

FrameOverlap::FrameOverlap() {}

FrameOverlap::FrameOverlap(const int frameSize, const int overlapSize) {
    _frameSize = frameSize;
    _overlapSize = overlapSize;
    reset(_frameSize, _overlapSize);
}

FrameOverlap::~FrameOverlap() {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
}

void FrameOverlap::reset(const int frameSize, const int overlapSize) {
    _frameSize = frameSize;
    _overlapSize = overlapSize;
    _hopSize = frameSize - _overlapSize;

    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
    _data = new float[_frameSize] {0};
}

bool FrameOverlap::updateFrame(const float* data, const int num) {
    if (num != _hopSize) {
        std::cout << "Update Frame Failed" << std::endl;
        return false;
    }

    memmove(_data, _data + _hopSize, _overlapSize * sizeof(float));
    memcpy(_data + _overlapSize, data, _hopSize * sizeof(float));

    return true;
}

bool FrameOverlap::getOutput(float* buf, const int num) {
    if (num != _hopSize) {
        std::cout << "GetOuput Failed" << std::endl;
        return false;
    }

    memcpy(buf, _data, num * sizeof(float));

    return true;
}

// void FrameOverlap::hanning(float *window, unsigned int framSize) {
//     const float PI_2 = M_PI + M_PI;
//     const float denom = 1.0f / static_cast<float>(framSize - 1);
//     const unsigned int halfSample = framSize >> 1;
//     for (unsigned int i = 0; i < halfSample; i++) {
//         window[i] = 0.5f * (1.0f - cos(PI_2 * static_cast<float>(i) * denom));
//         window[framSize - i - 1] = window[i];
//     }
//     // If the numSample is odd number, the center index of window should be calculated
//     if ((framSize & 1) == 1) {
//         window[halfSample + 1] = 0.5f;
//     }
// }
