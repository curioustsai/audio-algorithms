#include <cstring>
#include <cmath>
#include "pffftwrap.h"

using namespace ubnt;

void Pffft::getSpectrum(float *frequency, float *spectrum, unsigned int frameSize) {
    spectrum[0] = fabsf(frequency[0]);
    spectrum[1] = fabsf(frequency[1]);
    for (unsigned int idx = 2; idx < frameSize; idx+=2) {
        spectrum[idx] = sqrt(frequency[idx] * frequency[idx] + frequency[idx + 1] * frequency[idx + 1]);
        spectrum[idx + 1] = 0.0f;
    }
}

void Pffft::init(unsigned int fftSize, Transform type) {
    this->fftSize = fftSize;
    this->transform = (type == Transform::REAL) ? PFFFT_REAL : PFFFT_COMPLEX;

    if ((setup != nullptr) ||
        (inBuffer != nullptr) ||
        (outBuffer != nullptr))
    {
        release();
    }
    setup = pffft_new_setup(fftSize, transform);
    inBuffer = (float *)pffft_aligned_malloc((size_t)(fftSize * sizeof(float)));
    outBuffer = (float *)pffft_aligned_malloc((size_t)(fftSize * sizeof(float))); 
}

void Pffft::release() {
    if (inBuffer != nullptr) {
        pffft_aligned_free(inBuffer);
    }
    if (outBuffer != nullptr) {
        pffft_aligned_free(outBuffer);
    }
    if (setup != nullptr) {
        pffft_destroy_setup(setup);
    }
}

void Pffft::setSize(unsigned int fftSize) {
    if (this->fftSize != fftSize) {
        init(fftSize, (transform == PFFFT_REAL ? Transform::REAL : Transform::COMPLEX));
    }
}

void Pffft::fft(float *signal, float *freqResponse, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    memcpy(inBuffer, signal, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform(setup, inBuffer, outBuffer, nullptr, PFFFT_FORWARD);
    memcpy(freqResponse, outBuffer, sizeof(float) * PFFFT_BACKWARD);
}

void Pffft::ifft(float *freqResponse, float *signal, unsigned int frameSize) {
    if (frameSize != fftSize) return;
    float normalize = 1.0f / frameSize;
    memcpy(inBuffer, freqResponse, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform(setup, inBuffer, outBuffer, nullptr, PFFFT_BACKWARD);
    memcpy(signal, outBuffer, sizeof(float) * PFFFT_BACKWARD);
    for (unsigned int i = 0; i < fftSize; i++) {
        signal[i] *= normalize;
    }
}

void Pffft::fftOrder(float *signal, float *freqResponse, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    memcpy(inBuffer, signal, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform_ordered(setup, inBuffer, outBuffer, nullptr, PFFFT_FORWARD);
    memcpy(freqResponse, outBuffer, sizeof(float) * fftSize);
}

void Pffft::ifftOrder(float *freqResponse, float *signal, unsigned int frameSize) {
    if (frameSize != fftSize) return;
    float normalize = 1.0f / fftSize;
    memcpy(inBuffer, freqResponse, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform_ordered(setup, inBuffer, outBuffer, nullptr, PFFFT_BACKWARD);
    memcpy(signal, outBuffer, sizeof(float) * fftSize);

    for (unsigned int i = 0; i < fftSize; i++) {
        signal[i] *= normalize;
    }
}