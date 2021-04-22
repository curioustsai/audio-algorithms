/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#ifndef __OVERLAPADD_H__
#define __OVERLAPADD_H__
#include "utils.h"

namespace ubnt{

class OverlapAdd {
public:
    enum class WindowType {
        NONE,
        HANNING
    };
    OverlapAdd() = delete;
    ~OverlapAdd();
    OverlapAdd(unsigned int frameSize, WindowType inType, WindowType outType);
    int setInput(float *input, unsigned int frameSize);
    int getInput(float *input, unsigned int frameSize);
    int setOutput(float *output, unsigned int frameSize);
    int getOutput(float *output, unsigned int frameSize);
    
private:
    const unsigned int Capacity = 20;
    float *inWindow{nullptr};
    float *outWindow{nullptr};
    unsigned int halfBufferSize;
    unsigned int bufferSize;
    unsigned int bufPoolSize;
    float *inputPool{nullptr};
    float *outputPool{nullptr};

    unsigned int inPoolReadIdx{0U};
    unsigned int inPoolWriteIdx{0U};
    unsigned int inBufferedSize{0U};
    unsigned int outPoolReadIdx{0U};
    unsigned int outPoolWriteIdx{0U};
    unsigned int outBufferedSize{0U};

    void init();
    void release();

    WindowType inWindowType;
    WindowType outWindowType;
    void setWindow(float *window, WindowType type);
    void hanning(float *window, unsigned int numSample);
    
};

} // ubnt

#endif // __OVERLAPADD_H__
