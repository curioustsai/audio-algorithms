/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once
#include <cmath>

namespace ubnt {

class Biquad {
public:
    /* Default constructor, call reset function to set coefs before use */
    Biquad() = default;

    /* Default destructor */
    ~Biquad() = default;

    /*
     * Reset biquad filter with coef[5]
     * b0 = coef[0], b1 = coef[1], b2 = coef[2], a0 = 1, a1 = coef[3], a2 = coef[4]
     */
    bool reset(const float* coef, const int num);

    /*
     * Direct Form II implementation, can be processed in-place  
     */
    void process(const float* input, float* output, const int num);
    void process(const int16_t *input, int16_t *output, const int num);

private:
    float _coef[5] = {0.f};
    float _state[2] = {0.f};
};

/*
 * Second order section filter (cascaded biquad)
 */
class SosFilter {
public:
    /* Default constructor, call reset function to set coefs before use */
    SosFilter() = default;

    /* Default destructor */
    ~SosFilter();

    /*
     * Reset coefs with 2d array
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
    void process(const int16_t* input, int16_t* output, const int num);

private:
    int _numCascade{0};
    Biquad** _biquads{nullptr};
};

} // namespace ubnt
