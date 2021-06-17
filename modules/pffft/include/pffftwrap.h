/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */
#pragma once

namespace ubnt {

class Pffft {
public:
    enum class Transform {
        REAL,
        COMPLEX,
    };
    void init(unsigned int fftSize, Transform type);
    void release();
    void setSize(unsigned int fftSize);
    void fft(float *signal, float *freqResponse, unsigned int frameSize);
    void ifft(float *freqResponse, float *signal, unsigned int frameSize);
    void fftOrder(float *signal, float *freqResponse, unsigned int frameSize);
    void ifftOrder(float *freqResponse, float *signal, unsigned int frameSize);
    static void getSpectrum(float *freqeuncy, float *spectrum, unsigned int frameSize);

private:
    void *setup{nullptr};
    Transform transform{Transform::REAL};
    unsigned int fftSize{0};
    float *inBuffer{nullptr};
    float *outBuffer{nullptr};

    bool IsValidFftSize(size_t fft_size, Transform fft_type);
};
}