/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

namespace ubnt {

class Frame {
public:
    Frame();
    Frame(const int frameSize);
    ~Frame();

    bool reset(const int frameSize);
    bool updateFrame(const float* data, const int num);
    bool getOutput(float* buf, const int num);
    float getPowerMean();
    float getPowerdB();
    bool copyFrame(Frame& other);

protected:
    float* _data{nullptr};
    int _frameSize{1024};
};

class FrameOverlap : public Frame {
public:
    FrameOverlap();
    FrameOverlap(const int frameSize, const int overlapSize);
    ~FrameOverlap();

    bool reset(const int frameSize, const int overlapSize);
    bool updateFrame(const float* data, const int num);
    bool getOutput(float* buf, const int num);

private:
    int _overlapSize{512};
    int _hopSize{512};
};

} // namespace ubnt
