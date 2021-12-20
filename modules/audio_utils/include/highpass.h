/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once
#include "biquad.h"

namespace ubnt {

class HighPassFilter : public SosFilter {

public:
    enum class SampleRate { Fs_8kHz = 0, Fs_16kHz, Fs_32kHz, Fs_48kHz, Fs_ALL };
    enum class CutoffFreq { Fc_100Hz = 0, Fc_200Hz, Fc_300Hz, Fc_400Hz, Fc_500Hz, Fc_ALL };

    static constexpr int numSampleRate = 4;
    static constexpr int numCutoffFreq = 5;
    static constexpr int numBiquad = 2;
    static constexpr int numCoefs = 5;

    const float hpfCoefs[numSampleRate][numCutoffFreq][numBiquad][numCoefs] = {
        {// fs=8kHz, fc=100Hz
         {{0.64343135, -1.28686269, 0.64343135, -1.81987148, 0.84876895},
          {1., -2., 1., -1.97853641, 0.98531211}},

         // fs=8kHz, fc=200Hz
         {{0.5789906, -1.1579812, 0.5789906, -1.61837391, 0.72401014},
          {1., -2., 1., -1.94408931, 0.97094115}},

         // fs=8kHz, fc=300Hz
         {{0.51683305, -1.03366609, 0.51683305, -1.40702142, 0.62365275},
          {1., -2., 1., -1.89719293, 0.95697835}},

         // fs=8kHz, fc=400Hz
         {{0.45835291, -0.91670582, 0.45835291, -1.19423247, 0.5448359},
          {1., -2., 1., -1.83845374, 0.94350776}},

         // fs=8kHz, fc=500Hz
         {{0.40431432, -0.80862865, 0.40431432, -0.98576028, 0.48447206},
          {1., -2., 1., -1.76853864, 0.93060594}}},

        // fs=16kHz, fc=100Hz
        {{{0.67585365, -1.3517073, 0.67585365, -1.91345234, 0.9209974},
          {1., -2., 1., -1.99092138, 0.99262241}},

         // fs=16kHz, fc=200Hz
         {{0.64343135, -1.28686269, 0.64343135, -1.81987148, 0.84876895},
          {1., -2., 1., -1.97853641, 0.98531211}},

         // fs=16kHz, fc=300Hz
         {{0.61104256, -1.22208511, 0.61104256, -1.72099856, 0.78318312},
          {1., -2., 1., -1.96290425, 0.97808114}},

         // fs=16kHz, fc=400Hz
         {{0.5789906, -1.1579812, 0.5789906, -1.61837391, 0.72401014},
          {1., -2., 1., -1.94408931, 0.97094115}},

         // fs=16kHz, fc=500Hz
         {{0.54752227, -1.09504454, 0.54752227, -1.51333467, 0.67094936},
          {1., -2., 1., -1.92216093, 0.9639033}}},

        // fs=32kHz, fc=100Hz
        {{{0.69196614, -1.38393228, 0.69196614, -1.95772169, 0.95964837},
          {1., -2., 1., -1.99587747, 0.99630356}},

         // fs=32kHz, fc=200Hz
         {{0.67585365, -1.3517073, 0.67585365, -1.91345234, 0.9209974},
          {1., -2., 1., -1.99092138, 0.99262241}},

         // fs=32kHz, fc=300Hz
         {{0.65965959, -1.31931918, 0.65965959, -1.86742763, 0.88404198},
          {1., -2., 1., -1.98513862, 0.98895809}},

         // fs=32kHz, fc=400Hz
         {{0.64343135, -1.28686269, 0.64343135, -1.81987148, 0.84876895},
          {1., -2., 1., -1.97853641, 0.98531211}},

         // fs=32kHz, fc=500Hz
         {{0.62721243, -1.25442485, 0.62721243, -1.77099547, 0.8151581},
          {1., -2., 1., -1.97112232, 0.98168597}}},

        // fs=48kHz, fc=100Hz
        {{{0.69731009, -1.39462019, 0.69731009, -1.97204765, 0.97290995},
          {1., -2., 1., -1.99734459, 0.99753409}},

         // fs=48kHz, fc=200Hz
         {{0.68660746, -1.37321492, 0.68660746, -1.94317459, 0.94657583},
          {1., -2., 1., -1.99431772, 0.99507474}},

         // fs=48kHz, fc=300Hz
         {{0.67585365, -1.3517073, 0.67585365, -1.91345234, 0.9209974},
          {1., -2., 1., -1.99092138, 0.99262241}},

         // fs=48kHz, fc=400Hz
         {{0.66506367, -1.33012734, 0.66506367, -1.88295017, 0.89617272},
          {1., -2., 1., -1.98715762, 0.99017756}},

         // fs=48kHz, fc=500Hz
         {{0.65425175, -1.30850349, 0.65425175, -1.85173504, 0.87209823},
          {1., -2., 1., -1.98302857, 0.98774065}}}};

    HighPassFilter() = delete;
    HighPassFilter(SampleRate fs, CutoffFreq fc) {
        Reset(hpfCoefs[static_cast<int>(fs)][static_cast<int>(fc)], numBiquad);
    };
    ~HighPassFilter() = default;
};

} // namespace ubnt

