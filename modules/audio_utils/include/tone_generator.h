#pragma once

#include <cmath>
#include <cstdint>

void generate_sine(float *buf, int length, int sample_rate, int fc, float amplitude);
void generate_sine_int16(int16_t *buf, int length, int sample_rate, int fc, int16_t amplitude);

void generate_chirp(float *buf, int length, int sample_rate, int f1, int f2, float amplitude);
void generate_chirp_int16(int16_t *buf, int length, int sample_rate, int f1, int f2,
                          int16_t amplitude);
