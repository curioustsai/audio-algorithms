#pragma once

#include "alarm_detector.h"
#include "goertzel.h"


namespace ubnt {
namespace smartaudio {

class SmokeDetector : public AlarmDetector {
public:
	SmokeDetector ()  = default;
	~SmokeDetector () = default;;
    void Init(Config config, int* targetFrequencies, int numTargetFreq);
    void Release();
	bool Detect(float* data, int numSample) override;
    void SetThreshold(float threshold) override;
    float GetThreshold() const override;
    void ResetStates();

#ifdef AUDIO_ALGO_DEBUG
    float GetPowerAvg() const;
    bool GetStatus() const;
#endif

private:
    int _sampleRate;
    int _frameSize;
    float _threshold;
    int _numTargetFreq;
    Goertzel** _goertzel;

	bool *_observeBuf;
	bool *_candidateBuf;
	int _candidateBufLen;
	int _candidateBufIndex;
    int _observeBufLen;
	int _observeBufIndex;

    int _observeFrameNumHead;
    int _observeFrameNumTail;
    int _frameUpperBound;
    int _frameLowerBound;
	int _holdOff;

#ifdef AUDIO_ALGO_DEBUG
        float _powerAvg;
        bool _status;
#endif
};

} // namespace smartaudio
} // namespace ubnt
