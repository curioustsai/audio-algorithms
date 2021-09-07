/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

class Biquad {
public:
    Biquad();
    ~Biquad();
    /*
     * Init biquad filter with coef[5]
     * b0 = coef[0], b1 = coef[1], b2 = coef[2], a0 = 1, a1 = coef[3], a2 = coef[4]
     */
    bool reset(const float* coef, const int num);
    /*
     * Process can be in-place, Direct Form II implementation
     */
    void process(const float* input, float* output, const int num);

private:
    float _coef[5] = {0.f};
    float _state[2] = {0.f};
};

class SosFilter {
public:
    /*
     * Initialize with 2d coefs array
     * coefs[5][numCascade] = { 
     * {0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.4169284},
     * {1, -2, 1, -1.62913992, 0.9105507}
     * };
     */
    bool reset(const float coefs[][5], const int numCascade);

    /*
     * Process with cascaded biquad filters
     */
    void process(const float* input, float* output, const int num);

private:
    int _numCascade;
    Biquad** _biquads;
};

