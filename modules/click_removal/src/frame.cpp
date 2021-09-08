/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "frame.h"
#include <cmath>
#include <cstring>
#include <stdio.h>

namespace ubnt {

Frame::Frame() { reset(_frameSize); }

Frame::Frame(const int frameSize) : _frameSize(frameSize) { reset(_frameSize); };

bool Frame::reset(const int frameSize) {
    _frameSize = frameSize;

    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
    _data = new float[_frameSize]{0};

    return true;
}

Frame::~Frame() {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
}

bool Frame::updateFrame(const float* data, const int num) {
    if (num != _frameSize) {
        printf("Update Frame Failed\n");
        return false;
    }

    memcpy(_data, data, num * sizeof(float));

    return true;
}

bool Frame::getOutput(float* buf, const int num) {
    if (num != _frameSize) {
        printf("GetOuput Failed\n");
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

bool Frame::copyFrame(Frame& other) {
    if (other._frameSize != _frameSize) return false;

    if (_data == nullptr) { _data = new float[_frameSize]; }
    memcpy(_data, other._data, _frameSize * sizeof(float));

    return true;
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

bool FrameOverlap::reset(const int frameSize, const int overlapSize) {
    _frameSize = frameSize;
    _overlapSize = overlapSize;
    _hopSize = frameSize - _overlapSize;

    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
    _data = new float[_frameSize]{0};

    return true;
}

bool FrameOverlap::updateFrame(const float* data, const int num) {
    if (num != _hopSize) {
        printf("Update Frame Failed\n");
        return false;
    }

    memmove(_data, _data + _hopSize, _overlapSize * sizeof(float));
    memcpy(_data + _overlapSize, data, _hopSize * sizeof(float));

    return true;
}

bool FrameOverlap::getOutput(float* buf, const int num) {
    if (num != _hopSize) {
        printf("GetOuput Failed\n");
        return false;
    }

    memcpy(buf, _data, num * sizeof(float));

    return true;
}

} // namespace ubnt
