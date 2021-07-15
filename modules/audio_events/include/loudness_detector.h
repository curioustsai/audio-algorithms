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
    void Release();
    void ResetStates();
    AudioEventType Detect(float* data, int numSample);

    void SetDynamicHigh(float dynamicHigh) { _dynRngHigh = dynamicHigh; }
    void SetDynamicLow(float dynamicLow) { _dynRngLow = dynamicLow; }
    void SetThresholdLoud(float thresholdLoud) { _thresholdLoud = thresholdLoud; }
    void SetThresholdQuiet(float thresholdQuiet) { _thresholdQuiet = thresholdQuiet; }
    void SetLoudCntThr(int loudCntThr) { _loudCntThr = loudCntThr; }
    void SetQuietCntThr(int quietCntThr) { _quietCntThr = quietCntThr; }
    void SetIdleCntThr(int idleCntThr) { _idleCntThr = idleCntThr; }
    void SetModel(std::string model) { _model = std::move(model); };

    float GetDynamicHigh() const { return _dynRngHigh; }
    float GetDynamicLow() const { return _dynRngLow; }
    float GetThresholdLoud() const { return _thresholdLoud; }
    float GetThresholdQuiet() const { return _thresholdQuiet; }
    float GetPowerAvg() const { return _powerAvg; }
    float GetPowerAvgdB() const { return _powerAvgdB; };
    int GetLevel() const { return _level; }
    int GetLoudCntThr() const { return _loudCntThr; }
    int GetQuietCntThr() const { return _quietCntThr; }
    int GetIdleCntThr() const { return _idleCntThr; }

    void ShowConfig();

private:
    std::string _model{"g4dome"};
    float _dynRngHigh{-10};
    float _dynRngLow{-70};

    float _powerAvg{0.0};
    float _powerAvgdB{0.0};
    float _thresholdLoud{80};
    float _thresholdQuiet{1};

    int _level{0};
    int _loudCnt{0};
    int _loudCntThr{38};
    int _quietCnt{0};
    int _quietCntThr{1000};
    int _idleCnt{0};
    int _idleCntThr{1000};
};

} // namespace smartaudio
} // namespace ubnt
