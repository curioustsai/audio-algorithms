/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "biquad.h"
#include <cstring>
#include <stdio.h>

namespace ubnt {

bool Biquad::Reset(const float *coef, const int num) {
    if (num != 5) {
        printf("the element of coef array should be 5");
        return false;
    }

    memcpy(_coef, coef, num * sizeof(float));
    return true;
}

void Biquad::Process(const float *input, float *output, const int num) {
    float b0 = _coef[0];
    float b1 = _coef[1];
    float b2 = _coef[2];
    float a1 = _coef[3];
    float a2 = _coef[4];

    for (int i = 0; i < num; i++) {
        float temp1 = a1 * _state[0];
        float temp2 = a2 * _state[1];
        float wn = (float)input[i] - temp1 - temp2;
        output[i] = b0 * wn + b1 * _state[0] + b2 * _state[1];

        _state[1] = _state[0];
        _state[0] = wn;
    }
}

void Biquad::Process(const int16_t *input, int16_t *output, const int num) {
    float b0 = _coef[0];
    float b1 = _coef[1];
    float b2 = _coef[2];
    float a1 = _coef[3];
    float a2 = _coef[4];

    for (int i = 0; i < num; i++) {
        float temp1 = a1 * _state[0];
        float temp2 = a2 * _state[1];
        float wn = (float)input[i] - temp1 - temp2;
        float temp = b0 * wn + b1 * _state[0] + b2 * _state[1];
        output[i] = (int16_t)(temp + 0.5f);

        _state[1] = _state[0];
        _state[0] = wn;
    }
}

SosFilter::~SosFilter() {
    if (_biquads) {
        for (int i = 0; i < _numCascade; i++) {
            if (_biquads[i]) {
                delete _biquads[i];
                _biquads[i] = nullptr;
            }
        }
        delete[] _biquads;
        _biquads = nullptr;
    }
}

bool SosFilter::Reset(const float coefs[][5], const int numCascade) {
    _numCascade = numCascade;
    _biquads = new Biquad *[numCascade];

    for (int i = 0; i < numCascade; i++) {
        _biquads[i] = new Biquad();
        _biquads[i]->Reset(coefs[i], 5);
    }

    return true;
}

void SosFilter::Process(const float *input, float *output, const int num) {
    _biquads[0]->Process(input, output, num);
    for (int i = 1; i < _numCascade; i++) { _biquads[i]->Process(output, output, num); }
}

void SosFilter::Process(const int16_t *input, int16_t *output, const int num) {
    _biquads[0]->Process(input, output, num);
    for (int i = 1; i < _numCascade; i++) { _biquads[i]->Process(output, output, num); }
}
} // namespace ubnt
