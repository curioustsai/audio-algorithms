/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

namespace ubnt {

class RingBuffer;
class SosFilter;
class FrameOverlap;

class ClickRemoval {
public:
    ClickRemoval() = delete;
    ClickRemoval(const int frameSize, const int subframeSize, const float threshold_all,
                 const float threshold_4kHz);
    ~ClickRemoval();
    int process(const float* input, float* output, const int num);

#ifdef AUDIO_ALGO_DEBUG
    float* dbgInfo{nullptr};
#endif

private:
    RingBuffer* _inBuffer;
    SosFilter* _hpf4kHz;
    SosFilter* _lpf4kHz;
    FrameOverlap* _inFrame;
    FrameOverlap* _inFrame4kHz;

    int _frameSize{1024};
    int _subframeSize{1024};
    int _hopSize{512};
    int _detected{0};

    float* _hop{nullptr};
    float _threshold_all{0.01};
    float _threshold_4kHz{0.005};
};

} // namespace ubnt
