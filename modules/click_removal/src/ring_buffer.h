/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <memory>
#include "frame.h"

namespace ubnt {

class RingBuffer {
public:
    /* Constructor, create with default capacity 15360 samples */
    RingBuffer();

    /* Constructor, create with capacity size */
    RingBuffer(const int capacity);

    /* Destructor, delete _data buffer */
    ~RingBuffer();

    /* Resize with capacity */
    bool resetCapacity(const int capacity);

    /*  Put zeros into buffer */
    bool setDelay(const int delaySamples);

    /* Copy new frame into ring buffer */
    bool putFrame(const float* newFrame, const int num);

    /* Copy new frame into ring buffer */
    bool putFrame(Frame* newFrame);

    /* Copy frame into buffer */
    int getFrame(float* buffer, const int num);

    /* Copy frame into Frame */
    int getFrame(Frame* outFrame);

    /* Get samples in use */
    int getInUseSamples();

    /* print member info */
    void showInfo();

private:
    float* _data{nullptr};
    int _capacity{15360};
    int _inUseStart{0};
    int _inUseEnd{0};
    int _inUseLength{0};
};

} // namespace ubnt
