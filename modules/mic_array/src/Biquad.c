/**
 * Biquad filter
 * Coefficient: 2nd Order Butterworth
 * import numpy as np 
 * import scipy
 * from scipy.io import wavefile
 * 
 * fs, data = wavfile.read('./test.wav')
 * sos = signal.butter(2, 70, 'hp', fs=16000, output='sos')
 * filtered = signal.sosfilt(sos, data)
 * wavfile.write("output.wav", fs, filtered)
 *
 * H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 */


#include "Biquad.h"

void Biquad_Init(BiquadFilter *handle) {
    // HPF coffericent for cutoff frequency (-3dB) 70Hz
    handle->coef[0] = 0.98075006;
    handle->coef[1] = -1.96150012;
    handle->coef[2] = 0.98075006;
    handle->coef[3] = -1.96112953;
    handle->coef[4] = 0.96187072;

    handle->state[0] = 0;
    handle->state[1] = 0;
}

/*
 * Direct Form II, Biquad Implementation
 * H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 */
void Biquad_Process(BiquadFilter *bqfilter, float *input, float *output, int num) {
    float b0 = bqfilter->coef[0];
    float b1 = bqfilter->coef[1];
    float b2 = bqfilter->coef[2];
    float a1 = bqfilter->coef[3];
    float a2 = bqfilter->coef[4];

    for (int i = 0; i < num; i++) {
        float wn = (float)input[i] - a1 * bqfilter->state[0] - a2 * bqfilter->state[1];
        float yn = b0 * wn + b1 * bqfilter->state[0] + b2 * bqfilter->state[1];

        output[i] = yn;
        /* output[i] = roundf(yn); */
        bqfilter->state[1] = bqfilter->state[0];
        bqfilter->state[0] = wn;
    }
}
