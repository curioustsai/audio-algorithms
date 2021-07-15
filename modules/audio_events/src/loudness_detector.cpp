#include "audio_events.h"
#include <cmath>
#include <stdio.h>

using namespace ubnt::smartaudio;

void LoudnessDetector::Init(std::string model, float thresholdLoud, float thresholdQuiet) {
    _model = std::move(model);
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
    } else {
        _dynRngHigh = -10;
        _dynRngLow = -70;
    }
}

void LoudnessDetector::Release() {}

AudioEventType LoudnessDetector::Detect(float* data, int numSamples) {
    AudioEventType detected = AUDIO_EVENT_NONE;
    float power = 0.f;
    int level;

    for (int i = 0; i < numSamples; ++i) { power += (data[i] * data[i]); }
    power /= numSamples;
    _powerAvg = 0.9f * _powerAvg + 0.1f * power;
    _powerAvgdB = 10 * log10f(_powerAvg);

    level = (int)(100.0 * (_powerAvgdB - _dynRngLow) / (float)(_dynRngHigh - _dynRngLow));
    _level = std::min(100, std::max(level, 0));

    if (_level > _thresholdLoud) {
        _loudCnt++;
        _quietCnt = 0;
        if (_loudCnt > _loudCntThr) {
            detected = AUDIO_EVENT_LOUD;
            _loudCnt = 0;
        }
    } else if (_level <= _thresholdQuiet) {
        _quietCnt++;
        _loudCnt = 0;
        if (_quietCnt > _quietCntThr) {
            detected = AUDIO_EVENT_QUIET;
            _quietCnt = 0;
        }
    } else {
        _idleCnt++;
        if (_idleCnt > _idleCntThr) {
            _idleCnt = 0;
            _loudCnt = 0;
            _quietCnt = 0;
        }
    }

    return detected;
}

void LoudnessDetector::ResetStates() {
    _powerAvg = 0;
    _powerAvgdB = -70;
    _level = 0;
    _idleCnt = 0;
    _loudCnt = 0;
    _quietCnt = 0;
}

void LoudnessDetector::ShowConfig() {
    float loudThreshold_dB = _thresholdLoud / 100.0 * (_dynRngHigh - _dynRngLow) + _dynRngLow;
    float quietThreshold_dB = _thresholdQuiet / 100.0 * (_dynRngHigh - _dynRngLow) + _dynRngLow;

    printf(
        "Model: %s\n==================\nDynamic High : %2.2fdB\nDynamic Low : %2.2fdB\n"
        "LoudThreshold : %2.2f\nLoudThreshold dB: %2.2f\n"
        "QuietThreshold : %2.2f\nQuiethreshold dB: %2.2f\n"
        "LoudCntThr: %d\nQuietCntThr: %d\n\n",
        _model.c_str(), _dynRngHigh, _dynRngLow, _thresholdLoud, loudThreshold_dB, _thresholdQuiet,
        quietThreshold_dB, _loudCntThr, _quietCntThr);
}

