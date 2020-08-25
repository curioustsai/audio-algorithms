/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

#include "alarm_detector.h"
#include "goertzel.h"

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
    Goertzel** _goertzel;

    bool* _observeBuf{0};
    bool* _candidateBuf{0};
    int _candidateBufLen{0};
    int _candidateBufIndex{0};
    int _observeBufLen{0};
    int _observeBufIndex{0};

    int _observeFrameNumHead{0};
    int _observeFrameNumTail{0};
    int _frameUpperBound{0};
    int _frameLowerBound{0};
    int _holdOn{0};
    int _holdOff{0};
    int _alarmCount{0};

#ifdef AUDIO_ALGO_DEBUG
    float _powerAvg{0.0};
    bool _status{0};
#endif
};

} // namespace smartaudio
} // namespace ubnt
