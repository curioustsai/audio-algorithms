/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include <cstring>
#include <cmath>
#include <cassert>
#include "pffftwrap.h"
#include "pffft.h"

using namespace ubnt;

void Pffft::getSpectrum(const float *frequency, float *spectrum, unsigned int frameSize) {
    spectrum[0] = fabsf(frequency[0]);
    spectrum[1] = fabsf(frequency[1]);
    for (unsigned int idx = 2; idx < frameSize; idx+=2) {
        spectrum[idx] = sqrt(frequency[idx] * frequency[idx] + frequency[idx + 1] * frequency[idx + 1]);
        spectrum[idx + 1] = 0.0f;
    }
}

void Pffft::init(unsigned int fftSize, Transform type) {
    assert(IsValidFftSize(fftSize, transform));

    this->fftSize = fftSize;
    transform = type;

    if ((setup != nullptr) ||
        (inBuffer != nullptr) ||
        (outBuffer != nullptr))
    {
        release();
    }
    setup = pffft_new_setup(fftSize, (transform == Transform::REAL ? PFFFT_REAL : PFFFT_COMPLEX));
    size_t Nbyte = fftSize * sizeof(float) * (transform == Transform::REAL? 1 : 2);
    inBuffer = (float *)pffft_aligned_malloc(Nbyte);
    outBuffer = (float *)pffft_aligned_malloc(Nbyte); 
}

void Pffft::release() {
    if (inBuffer != nullptr) {
        pffft_aligned_free(inBuffer);
    }
    if (outBuffer != nullptr) {
        pffft_aligned_free(outBuffer);
    }
    if (setup != nullptr) {
        pffft_destroy_setup(reinterpret_cast<PFFFT_Setup*>(setup));
    }
}

void Pffft::setSize(unsigned int fftSize) {
    if (this->fftSize != fftSize) {
        init(fftSize, transform);
    }
}

/** Copied from WebRTC utility folder and change fft transform 
 *  structure to ubnt::Pffft::Transform
 */
bool Pffft::IsValidFftSize(size_t fft_size, Transform fft_type) {
  if (fft_size == 0) {
    return false;
  }
  // PFFFT only supports transforms for inputs of length N of the form
  // N = (2^a)*(3^b)*(5^c) where b >=0 and c >= 0 and a >= 5 for the real FFT
  // and a >= 4 for the complex FFT.
  constexpr int kFactors[] = {2, 3, 5};
  int factorization[] = {0, 0, 0};
  int n = static_cast<int>(fft_size);
  for (int i = 0; i < 3; ++i) {
    while (n % kFactors[i] == 0) {
      n = n / kFactors[i];
      factorization[i]++;
    }
  }
  int a_min = (fft_type == Transform::REAL) ? 5 : 4;
  return factorization[0] >= a_min && n == 1;
}

void Pffft::fft(const float *signal, float *freqResponse, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    size_t bufferSize = frameSize * (transform == Transform::REAL ? 1 : 2);
    memcpy(inBuffer, signal, sizeof(float) * bufferSize);
    memset(outBuffer, 0, sizeof(float) * bufferSize);

    pffft_transform(reinterpret_cast<PFFFT_Setup*>(setup), inBuffer, outBuffer, nullptr, PFFFT_FORWARD);
    memcpy(freqResponse, outBuffer, sizeof(float) * bufferSize);
}

void Pffft::ifft(const float *freqResponse, float *signal, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    size_t bufferSize = frameSize * (transform == Transform::REAL ? 1 : 2);
    float normalize = 1.0f / frameSize;
    memcpy(inBuffer, freqResponse, sizeof(float) * bufferSize);
    memset(outBuffer, 0, sizeof(float) * bufferSize);

    pffft_transform(reinterpret_cast<PFFFT_Setup*>(setup), inBuffer, outBuffer, nullptr, PFFFT_BACKWARD);
    memcpy(signal, outBuffer, sizeof(float) * bufferSize);
    for (unsigned int i = 0; i < bufferSize; i++) {
        signal[i] *= normalize;
    }
}

void Pffft::fftOrder(const float *signal, float *freqResponse, unsigned int frameSize) {
    if (frameSize != fftSize) return;

    size_t bufferSize = frameSize * (transform == Transform::REAL ? 1 : 2);
    memcpy(inBuffer, signal, sizeof(float) * bufferSize);
    memset(outBuffer, 0, sizeof(float) * bufferSize);

    pffft_transform_ordered(reinterpret_cast<PFFFT_Setup*>(setup), inBuffer, outBuffer, nullptr, PFFFT_FORWARD);
    memcpy(freqResponse, outBuffer, sizeof(float) * bufferSize);
}

void Pffft::ifftOrder(const float *freqResponse, float *signal, unsigned int frameSize) {
    if (frameSize != fftSize) return;
    
    size_t bufferSize = frameSize * (transform == Transform::REAL ? 1 : 2);
    float normalize = 1.0f / fftSize;
    memcpy(inBuffer, freqResponse, sizeof(float) * bufferSize);
    memset(outBuffer, 0, sizeof(float) * bufferSize);

    pffft_transform_ordered(reinterpret_cast<PFFFT_Setup*>(setup), inBuffer, outBuffer, nullptr, PFFFT_BACKWARD);
    memcpy(signal, outBuffer, sizeof(float) * bufferSize);

    for (unsigned int i = 0; i < bufferSize; i++) {
        signal[i] *= normalize;
    }
}
