#include "audio_events.h"
#include <cmath>

using namespace ubnt::smartaudio;

void LoudnessDetector::Init(float thresholdLoud, float thresholdQuiet) {
    _thresholdLoud = thresholdLoud;
    _thresholdQuiet = thresholdQuiet;
    _powerAvg = 0;
}
AudioEventType LoudnessDetector::Detect(float* data, int numSamples) {
    AudioEventType detected = AUDIO_EVENT_NONE;
    float power = 0.f;

    for (int i = 0; i < numSamples; ++i) { power += (data[i] * data[i]); }
    power /= numSamples;
    _powerAvg = 0.9f * _powerAvg + 0.1f * power;
    _powerAvgdB = 10 * log10f(_powerAvg);

    if (_powerAvgdB > _thresholdLoud)
        detected = AUDIO_EVENT_LOUD;
    else if (_powerAvgdB < _thresholdQuiet)
        detected = AUDIO_EVENT_QUIET;

    return detected;
}
void LoudnessDetector::SetThresholdLoud(float thresholdLoud) { _thresholdLoud = thresholdLoud; }
float LoudnessDetector::GetThresholdLoud() const { return _thresholdLoud; }
void LoudnessDetector::SetThresholdQuiet(float thresholdQuiet) { _thresholdQuiet = thresholdQuiet; }
float LoudnessDetector::GetThresholdQuiet() const { return _thresholdQuiet; }
float LoudnessDetector::GetPowerAvg() const { return _powerAvg; }
float LoudnessDetector::GetPowerAvgdB() const { return _powerAvgdB; }
