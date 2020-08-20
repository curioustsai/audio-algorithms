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
    float _powerAvg;
    float _powerAvgdB;
    float _thresholdLoud;
    float _thresholdQuiet;
};

} // namespace smartaudio
} // namespace ubnt
