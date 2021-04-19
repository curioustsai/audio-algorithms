#include <cmath>
#include <cstring>
#include "formantInterpolate.h"

namespace ubnt {
FormantInterpolate::FormantInterpolate(unsigned int numSample) {
    bufferSize = numSample;
    inSpectrum = new float[bufferSize]();
}

FormantInterpolate::~FormantInterpolate() {
    if (inSpectrum != nullptr) {
        delete[] inSpectrum;
    }
}

int FormantInterpolate::process(float *spectrum, const float shiftTone, unsigned int numSample) {
    // The fourier transformed data length should be even number
    if ((numSample & 1) == 1) return -1;
    if (numSample != bufferSize) return -1;

    if (this->shiftTone != shiftTone) {
        this->shiftTone = shiftTone;
        shiftRatio = powf(2.0f, -shiftTone * semiToneScale);
    }

    memcpy(inSpectrum, spectrum, sizeof(float) * numSample);

    const unsigned int freqSize = numSample >> 1;
    for (unsigned int idx = 0; idx <= freqSize; idx++) {
        const float pos = (float)idx * shiftRatio;
        const unsigned int inIdx = static_cast<int>(floorf(pos));
        // The highest frequency value is saved at index 1
        const int i = (idx == freqSize) ? 1 : idx * 2;
        const int inI = (inIdx >= freqSize) ? 1 : inIdx * 2;

        if ((inIdx > numSample) || (inIdx == numSample && pos > inIdx)) {
            spectrum[i] = 0;
        }
        else {
            if (inIdx == pos) {
                spectrum[i] = inSpectrum[inI];
            }
            else {
                const float currRatio = pos - (float)inIdx;
                spectrum[i] = inSpectrum[inI] + 
                    currRatio * (inSpectrum[inI + 2] - inSpectrum[inI]);
            }
        }
    }

    return numSample;
}

} // ubnt