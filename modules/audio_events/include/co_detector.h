/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

#include "alarm_detector.h"

namespace ubnt {
namespace smartaudio {

class CoDetector : public AlarmDetector {
public:
    CoDetector() = default;
    ~CoDetector() = default;
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
    class Impl;
    Impl *pimpl;
};

} // namespace smartaudio
} // namespace ubnt
