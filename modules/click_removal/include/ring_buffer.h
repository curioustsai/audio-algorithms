/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

class RingBuffer {
public:
    RingBuffer();
    RingBuffer(const int capacity);
    ~RingBuffer();

    bool resetCapacity(const int capacity);
    bool setDelay(const int delaySamples);
    bool putFrame(const float* newFrame, const int num);
    int getFrame(float* newFrame, const int num);
    int getInUseSamples();
    void showInfo();

private:
    float* _data{nullptr};
    int _capacity{15360};
    int _inUseStart{0};
    int _inUseEnd{0};
    int _inUseLength{0};
};
