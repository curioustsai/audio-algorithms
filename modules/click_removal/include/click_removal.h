/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <cstdint>
#include <memory>
namespace ubnt {

class RingBuffer;
class SosFilter;
class Frame;
class FrameOverlap;

class ClickRemoval {
public:
    ClickRemoval() = delete;
    ClickRemoval(const int frameSize, const int subframeSize, const float threshold_all,
                 const float threshold_4kHz);
    ~ClickRemoval();

    int process(float* buf, const int num);
    int process(int16_t* buf, const int num);

    void threshold_all(const float threshold_all);
    float threshold_all() const;

    void threshold_4kHz(const float threshold_4kHz);
    float threshold_4kHz() const;

#ifdef AUDIO_ALGO_DEBUG
    int dbgChannels{3};
    std::unique_ptr<float[]> dbgInfo;
#endif

private:
    void initHann(Frame* window);
    bool applyWindow(const Frame* frame, const Frame* window, Frame* windowed);
    void overlapAdd(Frame* previous, Frame* current, float* buf);

    RingBuffer* _inBuffer{nullptr};
    RingBuffer* _outBuffer{nullptr};
    SosFilter* _hpf4kHz{nullptr};
    SosFilter* _removeFilter{nullptr};

    FrameOverlap* _currFrame{nullptr};
    FrameOverlap* _prevFrame{nullptr};
    FrameOverlap* _outFrame{nullptr};
    Frame* _hop{nullptr};
    Frame* _frameHP{nullptr};
    Frame* _frameBP{nullptr};
    Frame* _frameLP{nullptr};
    Frame* _hann{nullptr};
    Frame* _windowed{nullptr};

    float* _floatBuf{nullptr};

    int _frameSize{1024};
    int _subframeSize{1024};
    int _hopSize{512};
    int _detected{0};

    float _threshold_all{0.01};
    float _threshold_4kHz{0.005};
};

} // namespace ubnt
