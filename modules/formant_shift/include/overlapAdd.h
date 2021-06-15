/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include "utils.h"

namespace ubnt{

class OverlapAdd {
public:
    enum class WindowType {
        NONE,
        HANNING
    };
    OverlapAdd() = delete;
    OverlapAdd(unsigned int frameSize, WindowType inType, WindowType outType);
    ~OverlapAdd();
    int setInput(const float *input, unsigned int frameSize);
    int getInput(float *input, unsigned int frameSize);
    int setOutput(const float *output, unsigned int frameSize);
    int getOutput(float *output, unsigned int frameSize);
    
private:
    void init();
    void release();
    void setWindow(float *window, WindowType type);
    void hanning(float *window, unsigned int numSample);

    float *inWindow{nullptr};
    float *outWindow{nullptr};
    float *inputPool{nullptr};
    float *outputPool{nullptr};

    static constexpr unsigned int Capacity = 21;
    unsigned int halfBufferSize;
    unsigned int bufferSize;
    unsigned int bufPoolSize;
    unsigned int inPoolReadIdx{0U};
    unsigned int inPoolWriteIdx{0U};
    unsigned int inBufferedSize{0U};
    unsigned int outPoolReadIdx{0U};
    unsigned int outPoolWriteIdx{0U};
    unsigned int outBufferedSize{0U};
    WindowType inWindowType;
    WindowType outWindowType;
};

} // ubnt