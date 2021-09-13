/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <memory>
namespace ubnt {

class Frame {
public:
    Frame();
    Frame(const int frameSize);
    ~Frame();

    virtual bool reset(const int frameSize);
    virtual bool updateFrame(const float* data, const int num);
    virtual bool updateFrame(const Frame* data);
    virtual bool getFrame(float* buf, const int num);
    virtual bool getFrame(Frame* data);

    float* ptr() const { return _data.get(); };
    int frameSize() const { return _frameSize; };

    float getPowerMean();
    float getPowerdB();
    bool copyFrame(Frame* other);

protected:
    std::unique_ptr<float[]> _data{nullptr};
    int _frameSize{1024};
};

class FrameOverlap : public Frame {
public:
    FrameOverlap();
    FrameOverlap(const int frameSize, const int overlapSize);
    ~FrameOverlap();

    bool reset(const int frameSize) override;

    bool reset(const int frameSize, const int overlapSize);
    bool updateHop(const float* data, const int num);
    bool updateHop(const Frame* data);
    bool getHop(float* buf, const int num);
    bool getHop(Frame*);

    /* windowing */
    // createWindow()
    // applyWindow()
    // getWindowedFrame()

    // int overlapAdd(float *data, int num)
    // {
    //
    // }
    //

private:
    // std::unique_ptr<float[]> _window;
    // std::unique_ptr<float[]> _windowedFrame;
    int _overlapSize{512};
    int _hopSize{512};
};

} // namespace ubnt
