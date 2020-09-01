/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

#include "audio_event_type.h"
#include <string>

namespace ubnt {
namespace smartaudio {

class LoudnessDetector {
public:
    LoudnessDetector() = default;
    ~LoudnessDetector() = default;
    void Init(std::string model, float thresholdLoud, float thresholdQuiet);
    AudioEventType Detect(float* data, int numSample);
    void SetLevelThreshold(int levelThreshold);
    void SetThresholdLoud(float thresholdLoud);
    void SetThresholdQuiet(float thresholdQuiet);
    void SetQuietCntThr(int quietCntThr);
    void SetModel(std::string model);

    float GetThresholdLoud() const;
    float GetThresholdQuiet() const;
    float GetPowerAvg() const;
    float GetPowerAvgdB() const;
    int GetQuietCntThr() const;
    int GetLevelThreshold() const;

private:
    std::string _model{"g4dome"};
    float _dynRngHigh{-10};
    float _dynRngLow{-70};

    float _powerAvg{0.0};
    float _powerAvgdB{0.0};
    float _thresholdLoud{-20.0};
    float _thresholdQuiet{-90.0};
    int _levelThreshold{50};

    int _loudCnt{0};
    int _loudCntThr{38};
    int _quietCnt{0};
    int _quietCntThr{1000};
};

} // namespace smartaudio
} // namespace ubnt
