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
    _data.reset(new float[_frameSize]{0});

    return true;
}

Frame::~Frame() {}

bool Frame::updateFrame(const float* data, const int num) {
    if (num != _frameSize) {
        printf("Update Frame Failed\n");
        return false;
    }

    memcpy(_data.get(), data, num * sizeof(float));

    return true;
}

bool Frame::updateFrame(const Frame* data) { return updateFrame(data->ptr(), data->frameSize()); }

bool Frame::getFrame(float* buf, const int num) {
    if (num != _frameSize) {
        printf("GetOuput Failed\n");
        return false;
    }

    memcpy(buf, _data.get(), num * sizeof(float));
    return true;
}

bool Frame::getFrame(Frame* data) { return getFrame(data->ptr(), data->frameSize()); }

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

bool Frame::copyFrame(Frame* other) {
    if (other->frameSize() != _frameSize) return false;

    if (_data == nullptr) { _data.reset(new float[_frameSize]); }
    memcpy(_data.get(), other->ptr(), _frameSize * sizeof(float));

    return true;
}

FrameOverlap::FrameOverlap() {}

FrameOverlap::FrameOverlap(const int frameSize, const int overlapSize) {
    _frameSize = frameSize;
    _overlapSize = overlapSize;
    reset(_frameSize, _overlapSize);
}

FrameOverlap::~FrameOverlap() {}

bool FrameOverlap::reset(const int frameSize) { return reset(frameSize, frameSize / 2); }
bool FrameOverlap::reset(const int frameSize, const int overlapSize) {
    _frameSize = frameSize;
    _overlapSize = overlapSize;
    _hopSize = frameSize - _overlapSize;

    _data.reset(new float[_frameSize]{0});

    return true;
}

bool FrameOverlap::updateHop(const float* data, const int num) {
    if (num != _hopSize) {
        printf("Update Hop Failed\n");
        return false;
    }

    memmove(_data.get(), _data.get() + _hopSize, _overlapSize * sizeof(float));
    memcpy(_data.get() + _overlapSize, data, _hopSize * sizeof(float));

    return true;
}

bool FrameOverlap::updateHop(const Frame* data) {
    return updateHop(data->ptr(), data->frameSize());
}

bool FrameOverlap::getHop(float* buf, const int num) {
    if (num != _hopSize) {
        printf("GetOuput Failed\n");
        return false;
    }

    memcpy(buf, _data.get(), num * sizeof(float));

    return true;
}

bool FrameOverlap::getHop(Frame* data) {
    return getFrame(data->ptr(), data->frameSize());
}

} // namespace ubnt
