/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

#include "alarm_detector.h"

namespace ubnt {
namespace smartaudio {

class SmokeDetector : public AlarmDetector {
public:
    SmokeDetector() = default;
    ~SmokeDetector() = default;
    void Init(Config config, int* targetFrequencies, int numTargetFreq);
    void Release();
    AudioEventType Detect(float* data, int numSample) override;
    void SetThreshold(float threshold) override;
    float GetThreshold() const override;
    void ResetStates();

#ifdef AUDIO_ALGO_DEBUG
    float GetPowerAvg() const;
    bool GetStatus() const;
#endif

private:
    int _sampleRate{48000};
    int _frameSize{128};
    float _threshold{-20.0};
    int _numTargetFreq{2};

    Goertzel** _goertzel{nullptr};
    Observer *_observer{nullptr};
    CountDown *_holdOn{nullptr};

    bool* _candidateBuf{0};
    int _candidateBufLen{0};
    int _candidateBufIndex{0};
    int _frameUpperBound{0};
    int _frameLowerBound{0};
    int _alarmCount{0};
    int _onThreshold{0};
    float _framesPerSec{0.0f};

    float getPower(float* data, int numSample);
    AudioEventType DetectPattern(float *data, int numSample);

#ifdef AUDIO_ALGO_DEBUG
    float _powerAvg{0.0};
    bool _status{0};
#endif
};

} // namespace smartaudio
} // namespace ubnt
