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
    virtual ~Frame();

    virtual bool reset(const int frameSize);
    virtual bool updateFrame(const float* dataBuf, const int num);
    virtual bool updateFrame(const Frame* dataFrame);
    virtual bool getFrame(float* buf, const int num);
    virtual bool getFrame(Frame* dataFrame);

    float* data() const { return _data; };
    int frameSize() const { return _frameSize; };

    float getPowerMean();
    float getPowerdB();
    bool copyFrame(Frame* other);

protected:
    float* _data{nullptr};
    int _frameSize{1024};
};

class FrameOverlap : public Frame {
public:
    FrameOverlap();
    FrameOverlap(const int frameSize, const int overlapSize);
    ~FrameOverlap();

    bool reset(const int frameSize) override;

    bool reset(const int frameSize, const int overlapSize);
    bool updateHop(const float* dataBuf, const int num);
    bool updateHop(const Frame* dataFrame);
    bool getHop(float* buf, const int num);
    bool getHop(Frame*);

private:
    int _overlapSize{512};
    int _hopSize{512};

};

} // namespace ubnt
