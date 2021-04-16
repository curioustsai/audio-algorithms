#pragma once

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
    float *inWindow{nullptr};
    float *outWindow{nullptr};
    unsigned int bufferSize;
    unsigned int bufPoolSize;
    float *inputPool{nullptr};
    float *outputPool{nullptr};

    unsigned int inPoolReadIdx{0U};
    unsigned int inPoolWriteIdx{0U};
    unsigned int outPoolReadIdx{0U};
    unsigned int outPoolWriteIdx{0U};

    void init();
    void release();

    WindowType inWindowType;
    WindowType outWindowType;
    void setWindow(float *window, WindowType type);
    void hanning(float *window, unsigned int numSample);
    
};

} // ubnt