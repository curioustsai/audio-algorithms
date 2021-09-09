/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <cstdint>
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

    int process(float* buf, const int num);
    int process(int16_t* buf, const int num);

    void threshold_all(const int threshold_all) { _threshold_all = threshold_all; }
    int threshold_all() const { return _threshold_all; }

    void threshold_4kHz(const int threshold_4kHz) { _threshold_4kHz = threshold_4kHz; }
    int threshold_4kHz() const { return _threshold_4kHz; }

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

    float* _floatBuf{nullptr};
    float* _hop{nullptr};
    float _threshold_all{0.01};
    float _threshold_4kHz{0.005};
};

} // namespace ubnt
