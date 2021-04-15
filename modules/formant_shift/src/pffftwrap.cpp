#include <cstring>
#include "pffftwrap.h"

using namespace ubnt;

void Pffft::init(unsigned int fftSize, Transform type) {
    this->fftSize = fftSize;
    this->transform = (type == Transform::REAL) ? PFFFT_REAL : PFFFT_COMPLEX;

    if (setup != nullptr) {
        pffft_destroy_setup(setup);
    }
    setup = pffft_new_setup(fftSize, transform);

    if (inBuffer != nullptr) {
        pffft_aligned_free(inBuffer); inBuffer = nullptr;
    }
    inBuffer = (float *)pffft_aligned_malloc((size_t)(fftSize * sizeof(float)));

    if (outBuffer != nullptr) {
        pffft_aligned_free(outBuffer); outBuffer = nullptr;
    }
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

    memcpy(inBuffer, freqResponse, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform(setup, inBuffer, outBuffer, nullptr, PFFFT_BACKWARD);
    memcpy(signal, outBuffer, sizeof(float) * PFFFT_BACKWARD);
}

void Pffft::fftOrder(float *signal, float *freqResponse, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    memcpy(inBuffer, signal, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform_ordered(setup, inBuffer, outBuffer, nullptr, PFFFT_FORWARD);
    memcpy(freqResponse, outBuffer, sizeof(float) * PFFFT_BACKWARD);
}

void Pffft::ifftOrder(float *freqResponse, float *signal, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    memcpy(inBuffer, freqResponse, sizeof(float) * frameSize);
    memset(outBuffer, 0, sizeof(float) * frameSize);

    pffft_transform_ordered(setup, inBuffer, outBuffer, nullptr, PFFFT_BACKWARD);
    memcpy(signal, outBuffer, sizeof(float) * PFFFT_BACKWARD);
}