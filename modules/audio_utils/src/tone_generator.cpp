#include "tone_generator.h"

void generate_sine(float *buf, int length, int sample_rate, int fc, float amplitude) {
    for (int i = 0; i < length; i++) { buf[i] = amplitude * sinf(2 * M_PI * fc * i / sample_rate); }
}

void generate_sine_int16(int16_t *buf, int length, int sample_rate, int fc, int16_t amplitude) {
    for (int i = 0; i < length; i++) {
        float temp = (float)amplitude * sinf(2 * M_PI * fc * i / sample_rate);
        buf[i] = (int16_t)(temp + 0.5f);
    }
}

void generate_chirp(float *buf, int length, int sample_rate, int f1, int f2, float amplitude) {
    float M = (float)length / sample_rate;
    for (int i = 0; i < length; ++i) {
        float t = (float)i / sample_rate;
        buf[i] = amplitude * sinf(2 * M_PI * (f1 * t + (f2 - f1) * t * t / (2.0 * M)));
    }
}

void generate_chirp_int16(int16_t *buf, int length, int sample_rate, int f1, int f2, int16_t amplitude) {
    float M = (float)length / sample_rate;
    for (int i = 0; i < length; ++i) {
        float t = (float)i / sample_rate;
        float temp = amplitude * sinf(2 * M_PI * (f1 * t + (f2 - f1) * t * t / (2.0 * M)));
        buf[i] = (int16_t)(temp + 0.5f);
    }
}
