/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

class Frame {
public:
    Frame();
    Frame(const int frameSize);
    ~Frame();

    void reset(const int frameSize);
    bool updateFrame(const float* data, const int num);
    bool getOutput(float* buf, const int num);
    float getPowerMean();
    float getPowerdB();

protected:
    float* _data{nullptr};
    int _frameSize{1024};
};

class FrameOverlap : public Frame {
public:
    FrameOverlap();
    FrameOverlap(const int frameSize, const int overlapSize);
    ~FrameOverlap();

    void reset(const int frameSize, const int overlapSize);
    bool updateFrame(const float* data, const int num);
    bool getOutput(float* buf, const int num);

    // void applyWindow();
    // void initHanning();

private:
    float *hanning{nullptr};
    float *windowed{nullptr};

    int _overlapSize{512};
    int _hopSize{512};
};
