#include "audio_events.h"
#include <cmath>

using namespace ubnt::smartaudio;

inline float convertLvlThr(int levelThreshold, float high = -20, float low = -60) {
    return (float)levelThreshold / 100 * (low - high) + high;
}

void LoudnessDetector::Init(std::string model, float thresholdLoud, float thresholdQuiet) {
    _model = model;
    _thresholdLoud = thresholdLoud;
    _thresholdQuiet = thresholdQuiet;

    if (_model.compare("g4dome") == 0) {
        _dynRngHigh = -10;
        _dynRngLow = -50;
    } else if (_model.compare("g4pro") == 0) {
        _dynRngHigh = -25;
        _dynRngLow = -75;
    } else if (_model.compare("g4bullet") == 0) {
        _dynRngHigh = -25;
        _dynRngLow = -75;
    } else if (_model.compare("g3flex") == 0) {
        _dynRngHigh = -25;
        _dynRngLow = -75;
    }
}

void LoudnessDetector::Release() {}

AudioEventType LoudnessDetector::Detect(float* data, int numSamples) {
    AudioEventType detected = AUDIO_EVENT_NONE;
    float power = 0.f;

    for (int i = 0; i < numSamples; ++i) { power += (data[i] * data[i]); }
    power /= numSamples;
    _powerAvg = 0.9f * _powerAvg + 0.1f * power;
    _powerAvgdB = 10 * log10f(_powerAvg);

    if (_powerAvgdB > _thresholdLoud) {
        _loudCnt++;
        if (_loudCnt > _loudCntThr) {
            detected = AUDIO_EVENT_LOUD;
            _loudCnt = 0;
        }
    } else if (_powerAvgdB < _thresholdQuiet) {
        _quietCnt++;
        if (_quietCnt > _quietCntThr) {
            detected = AUDIO_EVENT_QUIET;
            _quietCnt = 0;
        }
    }

    return detected;
}

void LoudnessDetector::ResetStates() {
    _powerAvg = 0;
    _powerAvgdB = 0;
    _loudCnt = 0;
    _quietCnt = 0;
}

void LoudnessDetector::SetThresholdLoud(float thresholdLoud) { _thresholdLoud = thresholdLoud; }
void LoudnessDetector::SetThresholdQuiet(float thresholdQuiet) { _thresholdQuiet = thresholdQuiet; }
void LoudnessDetector::SetQuietCntThr(int quietCntThr) { _quietCntThr = quietCntThr; }
void LoudnessDetector::SetLevelThreshold(int levelThreshold) {
    _levelThreshold = levelThreshold;
    _thresholdLoud = convertLvlThr(levelThreshold, _dynRngHigh, _dynRngLow);

#ifdef AUDIO_ALGO_DEBUG
    printf(
        "Model: %s\n==================\nDynamic High : %2.2fdB\nDynamic Low : %2.2fdB\nThreshold : "
        "%2.2fdB\n\n",
        _model.c_str(), _dynRngHigh, _dynRngLow, _thresholdLoud);
#endif
}
void LoudnessDetector::SetModel(std::string model) { _model = model; }

float LoudnessDetector::GetThresholdLoud() const { return _thresholdLoud; }
float LoudnessDetector::GetThresholdQuiet() const { return _thresholdQuiet; }
float LoudnessDetector::GetPowerAvg() const { return _powerAvg; }
float LoudnessDetector::GetPowerAvgdB() const { return _powerAvgdB; }
int LoudnessDetector::GetQuietCntThr() const { return _quietCntThr; }
int LoudnessDetector::GetLevelThreshold() const { return _levelThreshold; }
