/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "ring_buffer.h"
#include <cstring>
#include <stdio.h>

namespace ubnt {

RingBuffer::RingBuffer() {
    _capacity = 15360;
    resetCapacity(_capacity);
}

RingBuffer::RingBuffer(const int capacity) : _capacity(capacity) { resetCapacity(_capacity); }
RingBuffer::~RingBuffer() {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
}

bool RingBuffer::resetCapacity(const int capacity) {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
    _data = new float[capacity]{0};
    if (_data == nullptr) return false;

    _capacity = capacity;
    return true;
}

bool RingBuffer::setDelay(const int delaySamples) {
    float* buffer = new float[delaySamples]{0};
    putFrame(buffer, delaySamples);
    delete[] buffer;

    return true;
}

int RingBuffer::getInUseSamples() { return _inUseLength; }

bool RingBuffer::putFrame(const float* dataFrame, const int num) {
    // overwrite cases, ignore and return false
    if ((_capacity - _inUseLength) < num) { return false; }

    // safe case
    if (_inUseEnd + num < _capacity) {
        memcpy(_data + _inUseEnd, dataFrame, num * sizeof(float));
        _inUseLength += num;
        _inUseEnd += num;
    } else {
        int len1 = _capacity - _inUseEnd;
        int len2 = num - len1;
        memcpy(_data + _inUseEnd, dataFrame, len1 * sizeof(float));
        memcpy(_data, dataFrame + len1, len2 * sizeof(float));

        _inUseLength += num;
        _inUseEnd = len2;
    }

    return true;
}

int RingBuffer::getFrame(float* dataFrame, const int num) {
    // not enough data for output
    if (_inUseLength < num) { return 0; }

    // successfully get data
    if (_inUseStart + num < _capacity) {
        memcpy(dataFrame, _data + _inUseStart, num * sizeof(float));
        _inUseLength -= num;
        _inUseStart += num;
    } else {
        int len1 = _capacity - _inUseStart;
        int len2 = num - len1;
        memcpy(dataFrame, _data + _inUseStart, len1 * sizeof(float));
        memcpy(dataFrame + len1, _data, len2 * sizeof(float));
        _inUseLength -= num;
        _inUseStart = len2;
    }
    return num;
}

void RingBuffer::showInfo() {
    printf("capacity: %d\nIn Use Lenght: %d\nStart index: %d\nEnd Index: %d\n\n", _capacity,
           _inUseLength, _inUseStart, _inUseEnd);
}

} // namespace ubnt
