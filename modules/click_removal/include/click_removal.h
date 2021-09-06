/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include "biquad.h"
#include "frame.h"
#include "ring_buffer.h"

class ClickRemoval {
public:
    ClickRemoval() = delete;
    ClickRemoval(const int frameSize, const int subframeSize, const float threshold_all,
                 const float threshold_4kHz);
    ~ClickRemoval();
    int process(const float *input, float *output, const int num);

#ifdef AUDIO_ALGO_DEBUG
    float *dbgInfo; 
#endif

private:
    RingBuffer _rawBuffer{1024};
    SosFilter _hpf4kHz;
    FrameOverlap _inFrame{1024, 512};
    FrameOverlap _inFrame4kHz{1024, 512};

    int _frameSize{1024};
    int _subframeSize{256};
    int _hopSize{128};
    int _detected{0};

    float _threshold_all{0.01};
    float _threshold_4kHz{0.005};
    float _framePowerSmooth{0};
};

