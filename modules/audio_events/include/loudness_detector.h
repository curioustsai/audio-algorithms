#pragma once

#include "audio_event_type.h"

namespace ubnt {
namespace smartaudio {

class LoudnessDetector {
public:
	LoudnessDetector(float thresholdLoud, float thresholdQuiet);
    AudioEventType Detect(float* data, int numSample);
    void SetThresholdLoud(float thresholdLoud);
    float GetThresholdLoud() const;
    void SetThresholdQuiet(float thresholdQuiet);
    float GetThresholdQuiet() const;
    float GetPowerAvg() const;

private:
	float _powerAvg;
    float _thresholdLoud;
    float _thresholdQuiet;
    int _holdOff;

};

}  // namespace smartaudio
}  // namespace ubnt
