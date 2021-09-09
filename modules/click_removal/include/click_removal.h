/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <cstdint>
#include <memory>
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
    std::unique_ptr<float[]> dbgInfo;
#endif

private:
    std::unique_ptr<RingBuffer> _inBuffer;
    std::unique_ptr<SosFilter> _hpf4kHz;
    std::unique_ptr<SosFilter> _lpf4kHz;
    std::unique_ptr<FrameOverlap> _inFrame;
    std::unique_ptr<FrameOverlap> _inFrame4kHz;
    std::unique_ptr<float[]> _floatBuf;
    std::unique_ptr<float[]> _hop;

    int _frameSize{1024};
    int _subframeSize{1024};
    int _hopSize{512};
    int _detected{0};

    float _threshold_all{0.01};
    float _threshold_4kHz{0.005};
};

} // namespace ubnt
