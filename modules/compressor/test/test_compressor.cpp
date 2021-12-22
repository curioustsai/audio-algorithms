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
    float knee = 1.f;

    // compressor 1 (normal)
    float threshold = -24.f;
    float ratio = 2.f;

    // compressor 2 (aggressive)
    float threshold_agg = -12.f;
    float ratio_agg = 4.f;

    // expander
    float threshold_expander = -60.f;
    float ratio_expander = 4.f;

    float attack = 0.003f;
    float release = 0.250f;

    sf_compressor_state_st compressor_st;
    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, knee, threshold, ratio,
                  threshold_agg, ratio_agg, threshold_expander, ratio_expander, attack, release);

    int duration = 5;
    int total_length = sample_rate * duration;
    int fc = 1000;
    int frame_size = 1024;
    int chunks = total_length / frame_size;

    float* outbuf = new float[total_length];
    std::vector<float> amplitudes;
    for (float dB = -93; dB <= 0; dB += 3) { amplitudes.push_back(powf(10, 0.05 * dB)); }
    std::vector<float> answers = {-68.08, -67.33, -66.58, -65.83, -65.08, -64.33, -63.58, -62.83,
                                  -62.08, -61.33, -60.58, -59.83, -56.95, -53.95, -50.96, -47.98,
                                  -44.98, -42.00, -39.00, -36.00, -33.00, -30.00, -27.00, -24.00,
                                  -22.21, -20.70, -19.19, -17.68, -16.58, -15.77, -15.01, -14.26};

    /*
     * Test without pre-gain & post-gain
     */
    std::vector<float> input_data;
    std::vector<float> output_data;

    for (size_t k = 0; k < amplitudes.size(); k++) {
        float amp = amplitudes[k];

        float* inbuf = generate_sine(sample_rate, duration, fc, amp);
        for (int i = 0; i < chunks; i++) {
            sf_compressor_process(&compressor_st, frame_size, &inbuf[i * frame_size],
                                  &outbuf[i * frame_size]);
        }

        // compare, start from 3s
        // float dBFS_input = calculate_dBFS(inbuf, total_length, 4 * sample_rate, 4.5 * sample_rate);
        float dBFS_output =
            calculate_dBFS(outbuf, total_length, 4 * sample_rate, 4.5 * sample_rate);
        // printf("dBFS_input: %f\tdBFS_output: %f\tanswer:%f\n", dBFS_input, dBFS_output, answers[k]);
        // printf("dBFS_input: %f\tdBFS_output: %f\n", dBFS_input, dBFS_output);

        ASSERT_LT(fabsf(dBFS_output - answers[k]), 1);
        std::vector<float> temp(inbuf, inbuf + duration * sample_rate);
        input_data.insert(input_data.end(), temp.begin(), temp.end());

        std::vector<float> temp2(outbuf, outbuf + duration * sample_rate);
        output_data.insert(output_data.end(), temp2.begin(), temp2.end());

        delete[] inbuf;
    }

    // output_pcm(input_data.data(), sample_rate * duration * amplitudes.size(), "./amp.pcm");
    // output_pcm(output_data.data(), sample_rate * duration * amplitudes.size(), "./output.pcm");

    /*
     * Test with pre-gain
     */
    pregain = 6.f;
    postgain = 0.f;
    knee = 1.f;
    // compressor 1 (normal)
    threshold = -24.f;
    ratio = 2.f;
    // compressor 2 (aggressive)
    threshold_agg = -12.f;
    ratio_agg = 4.f;
    // expander
    threshold_expander = -60.f;
    ratio_expander = 4.f;

    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, knee, threshold, ratio,
                  threshold_agg, ratio_agg, threshold_expander, ratio_expander, attack, release);
    answers.clear();

    answers = {-66.58, -65.83, -65.08, -64.33, -63.58, -62.83, -62.08, -61.33,
               -60.58, -59.83, -56.95, -53.95, -50.96, -47.98, -44.98, -42.00,
               -39.00, -36.00, -33.00, -30.00, -27.00, -24.00, -22.21, -20.70,
               -19.19, -17.68, -16.58, -15.77, -15.01, -14.26, -13.50, -12.74};

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
    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, knee, threshold, ratio,
                  threshold_agg, ratio_agg, threshold_expander, ratio_expander, attack, release);
    // printf("pre-gain: %f\tpost-gain: %f\n", pregain, postgain);
    answers.clear();
    answers = {-62.08, -61.33, -60.58, -59.83, -59.08, -58.33, -57.58, -56.83,
               -56.08, -55.33, -54.58, -53.83, -50.95, -47.95, -44.96, -41.98,
               -38.98, -36.00, -33.00, -30.00, -27.00, -24.00, -21.00, -18.00,
               -16.21, -14.70, -13.19, -11.68, -10.58, -9.77,  -9.01,  -8.26};

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
