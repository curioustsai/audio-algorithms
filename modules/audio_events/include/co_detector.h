#pragma once

#include "alarm_detector.h"
#include "goertzel.h"

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
    int _numTargetFreq;
    int _sampleRate;
    int _frameSize;
    float _threshold;
    Goertzel** _goertzel;

    bool* _observeBuf;
    int _observeBufLen;
    int _observeBufIndex;
    int _frameUpperBound;
    int _frameLowerBound;
    int _holdOn;
    int _holdOff;
    int _alarmCount;

#ifdef AUDIO_ALGO_DEBUG
    float _powerAvg;
    bool _status;
#endif
};

} // namespace smartaudio
} // namespace ubnt
