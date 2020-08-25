/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

#include "audio_event_type.h"

namespace ubnt {
namespace smartaudio {

class LoudnessDetector {
public:
    void Init(float thresholdLoud, float thresholdQuiet);
    AudioEventType Detect(float* data, int numSample);
    void SetThresholdLoud(float thresholdLoud);
    float GetThresholdLoud() const;
    void SetThresholdQuiet(float thresholdQuiet);
    float GetThresholdQuiet() const;
    float GetPowerAvg() const;
    float GetPowerAvgdB() const;

private:
    float _powerAvg{0.0};
    float _powerAvgdB{0.0};
    float _thresholdLoud{0.0};
    float _thresholdQuiet{0.0};
};

} // namespace smartaudio
} // namespace ubnt
