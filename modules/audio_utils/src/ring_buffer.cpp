/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "ring_buffer.h"
#include "ubnt_logger/ubnt_logger.h"
#include <cstring>
#include <stdio.h>

namespace ubnt {

template <typename T>
RingBuffer<T>::RingBuffer() {
    _capacity = 15360;
    resetCapacity(_capacity);
}

template <typename T>
RingBuffer<T>::RingBuffer(const int frameSize, const int num) {
    _capacity = frameSize * num;
    resetCapacity(_capacity);
}

template <typename T>
RingBuffer<T>::~RingBuffer() {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
}

template <typename T>
bool RingBuffer<T>::resetCapacity(const int capacity) {
    _data = new T[capacity];
    memset(_data, 0, sizeof(T) * capacity);
    if (_data == nullptr) return false;

    _capacity = capacity;
    return true;
}

template <typename T>
bool RingBuffer<T>::setDelay(const int delaySamples) {
    T* buffer = new T[delaySamples];
    memset(buffer, 0, sizeof(T) * delaySamples);
    putFrame(buffer, delaySamples);
    delete[] buffer;

    return true;
}

template <typename T>
int RingBuffer<T>::getInUseSamples() {
    return _inUseLength;
}

template <typename T>
bool RingBuffer<T>::putFrame(const T* dataFrame, const int num) {
    // overwrite cases, ignore and return false
    if ((_capacity - _inUseLength) < num) { return false; }

    // safe case
    if (_inUseEnd + num < _capacity) {
        memcpy(_data + _inUseEnd, dataFrame, num * sizeof(T));
        _inUseLength += num;
        _inUseEnd += num;
    } else {
        int len1 = _capacity - _inUseEnd;
        int len2 = num - len1;
        memcpy(_data + _inUseEnd, dataFrame, len1 * sizeof(T));
        memcpy(_data, dataFrame + len1, len2 * sizeof(T));

        _inUseLength += num;
        _inUseEnd = len2;
    }

    return true;
}

template <typename T>
int RingBuffer<T>::getFrame(T* dataFrame, const int num) {
    // not enough data for output
    if (_inUseLength < num) { return 0; }

    // successfully get data
    if (_inUseStart + num < _capacity) {
        memcpy(dataFrame, _data + _inUseStart, num * sizeof(T));
        _inUseLength -= num;
        _inUseStart += num;
    } else {
        int len1 = _capacity - _inUseStart;
        int len2 = num - len1;
        memcpy(dataFrame, _data + _inUseStart, len1 * sizeof(T));
        memcpy(dataFrame + len1, _data, len2 * sizeof(T));
        _inUseLength -= num;
        _inUseStart = len2;
    }
    return num;
}

template <typename T>
void RingBuffer<T>::showInfo() {
    INFO("capacity: %d\nIn Use Lenght: %d\nStart index: %d\nEnd Index: %d\n\n", _capacity,
         _inUseLength, _inUseStart, _inUseEnd);
}

} // namespace ubnt
