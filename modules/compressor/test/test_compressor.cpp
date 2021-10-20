#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "compressor.h"

float* generate_sine(int sample_rate = 48000, float duration = 5, int fc = 1000,
                     float amplitude = 0.5) {
    int total_length = sample_rate * duration;
    float* buf = new float[total_length];
    for (int i = 0; i < total_length; i++) {
        buf[i] = amplitude * sinf(2 * M_PI * fc * i / sample_rate);
    }

    return buf;
}

void output_pcm(float* buf, int length, const char* path) {
    FILE* fptr = fopen(path, "wb");
    short* buf_s16 = new short[length];

    for (int i = 0; i < length; i++) { buf_s16[i] = roundf(buf[i] * 32768); }

    fwrite(buf_s16, sizeof(short), length, fptr);
    fclose(fptr);

    delete[] buf_s16;
}

float calculate_rms(float* buf, int length, int startpoint = 0, int endpoint = 0) {
    if (endpoint == 0) endpoint = length;

    float rms = 0.f;
    for (int i = startpoint; i < endpoint; i++) { rms += (buf[i] * buf[i]); }
    rms /= (endpoint - startpoint);
    return 10 * log10f(rms);
}

float calculate_dBFS(float* buf, int length, int startpoint = 0, int endpoint = 0) {
    if (endpoint == 0) endpoint = length;

    float maxf = 0;
    for (int i = startpoint; i < endpoint; i++) { maxf = std::max(maxf, fabsf(buf[i])); }
    return 20 * log10f(maxf);
}

TEST(DRC, Process) {
    int sample_rate = 48000;

    float pregain = 0.f;
    float postgain = 0.f;
    float threshold = -12.f;
    float knee = 3.f;
    float ratio = 12.f;
    float attack = 0.003f;
    float release = 0.250f;

    sf_compressor_state_st compressor_st;
    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, threshold, knee, ratio, attack,
                  release);

    int duration = 5;
    int total_length = sample_rate * duration;
    int fc = 1000;
    int frame_size = 1024;
    int chunks = total_length / frame_size;

    float* outbuf = new float[total_length];
    std::vector<float> amplitudes;
    for (float dB = -60; dB <= 0; dB += 3) { amplitudes.push_back(powf(10, 0.05 * dB)); }
    std::vector<float> answers = {
        -60.0f, -57.0f, -54.0f, -51.0f,        -48.0f,        -45.0f,        -42.0f,
        -39.0f, -36.0f, -33.0f, -30.0f,        -27.0f,        -24.0f,        -21.0f,
        -18.0f, -15.0f, -12.0f, -10.84154926f, -10.59154926f, -10.34154926f, -10.09154926f};
    /*
     * Test without pre-gain & post-gain
     */
    // iterate different amplitude levels
    // printf("pre-gain: %f\tpost-gain: %f\n", pregain, postgain);
    for (size_t k = 0; k < amplitudes.size(); k++) {
        float amp = amplitudes[k];

        float* inbuf = generate_sine(sample_rate, duration, fc, amp);
        for (int i = 0; i < chunks; i++) {
            sf_compressor_process(&compressor_st, frame_size, &inbuf[i * frame_size],
                                  &outbuf[i * frame_size]);
        }

        // compare, start from 3s
        // float dBFS_input = calculate_dBFS(inbuf, total_length, 3 * sample_rate, 4 * sample_rate);
        float dBFS_output = calculate_dBFS(outbuf, total_length, 3 * sample_rate, 4 * sample_rate);
        // printf("dBFS_input: %f\tdBFS_output: %f\tanswer:%f\n", dBFS_input, dBFS_output, answers[k]);

        ASSERT_LT(fabsf(dBFS_output - answers[k]), 1);

        delete[] inbuf;
    }

    /*
     * Test with pre-gain
     */
    pregain = 6.f;
    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, threshold, knee, ratio, attack,
                  release);
    // iterate different amplitude levels
    // printf("pre-gain: %f\tpost-gain: %f\n", pregain, postgain);
    answers.clear();
    answers = {
        -54.0, -51.0,        -48.0,        -45.0,        -42.0,        -39.0,       -36.0,
        -33.0, -30.0,        -27.0,        -24.0,        -21.0,        -18.0,       -15.0,
        -12.0, -10.84154926, -10.59154926, -10.34154926, -10.09154926, -9.84154926, -9.59154926,
    };

    for (size_t k = 0; k < amplitudes.size(); k++) {
        float amp = amplitudes[k];
        float* inbuf = generate_sine(sample_rate, duration, fc, amp);
        for (int i = 0; i < chunks; i++) {
            sf_compressor_process(&compressor_st, frame_size, &inbuf[i * frame_size],
                                  &outbuf[i * frame_size]);
        }

        // compare, start from 3s
        // float dBFS_input = calculate_dBFS(inbuf, total_length, 3 * sample_rate, 4 * sample_rate);
        float dBFS_output = calculate_dBFS(outbuf, total_length, 3 * sample_rate, 4 * sample_rate);
        // printf("dBFS_input: %f\tdBFS_output: %f\n", dBFS_input, dBFS_output);

        ASSERT_LT(fabsf(dBFS_output - answers[k]), 1);

        delete[] inbuf;
    }

    /*
     * Test with post-gain
     */
    pregain = 0.f;
    postgain = 6.0f;
    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, threshold, knee, ratio, attack,
                  release);
    // printf("pre-gain: %f\tpost-gain: %f\n", pregain, postgain);
    answers.clear();
    answers = {-54.0, -51.0, -48.0, -45.0,       -42.0,       -39.0,       -36.0,
               -33.0, -30.0, -27.0, -24.0,       -21.0,       -18.0,       -15.0,
               -12.0, -9.0,  -6.0,  -4.84154926, -4.59154926, -4.34154926, -4.09154926};

    // iterate different amplitude levels
    for (size_t k = 0; k < amplitudes.size(); k++) {
        float amp = amplitudes[k];
        float* inbuf = generate_sine(sample_rate, duration, fc, amp);
        for (int i = 0; i < chunks; i++) {
            sf_compressor_process(&compressor_st, frame_size, &inbuf[i * frame_size],
                                  &outbuf[i * frame_size]);
        }
        // compare, start from 3s
        // float dBFS_input = calculate_dBFS(inbuf, total_length, 3 * sample_rate, 4 * sample_rate);
        float dBFS_output = calculate_dBFS(outbuf, total_length, 3 * sample_rate, 4 * sample_rate);
        // printf("dBFS_input: %f\tdBFS_output: %f\n", dBFS_input, dBFS_output);

        ASSERT_LT(fabsf(dBFS_output - answers[k]), 1);

        delete[] inbuf;
    }

#if 0
    std::string inpath = "./sine_1k_test.pcm";
    output_pcm(inbuf, 48000 * 5, inpath.c_str());
    std::string outpath = "./sine_1k_result.pcm";
    output_pcm(outbuf, total_length, outpath.c_str());
#endif

    delete[] outbuf;
}

