#include <cmath>
#include "smoke_detector.h"
#include "goertzel.h"
#include "countdown.h"
#include "observer.h"

using namespace ubnt::smartaudio;
const static int NUM_ON = 3;
const static int NUM_ON_OFF = 6;
const static float INTERVAL_SEC = 0.5; // second

inline bool anyTrueInArray(bool *in, unsigned int arraySize) {
    unsigned int i = 0;
    while (!in[i] && i < arraySize) i++;
    return (i < arraySize);
}

void SmokeDetector::Init(Config config, int* targetFrequencies, int numTargetFreq) {
    _numTargetFreq = numTargetFreq;
    _sampleRate = config.sampleRate;
    _frameSize = config.frameSize;
    _threshold = config.threshold;

    /*
     |-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|
     ON,     OFF,   ON,    OFF,   ON,    OFF,   OFF,   OFF
     INTERVAL ON: 3
     INTERVAL OFF: 5
     */

    _framesPerSec = (float)_sampleRate / (float)_frameSize;
    _onThreshold = static_cast<int>(round(INTERVAL_SEC * _framesPerSec * 0.75f));
    _energyOnThreshold = static_cast<int>(round(INTERVAL_SEC * _framesPerSec * 0.9f));

    float observeBufLen = int(8.0f * INTERVAL_SEC * _framesPerSec);
    _frameUpperBound = int(observeBufLen * 1.0 / 8.0);
    _frameLowerBound = int(observeBufLen * 2.5 / 8.0);

    _candidateBufLen = 10;
    _holdOn = new CountDown(static_cast<unsigned int>(10.0 * _framesPerSec));
    _holdOn->setCounter(0);
    _alarmCount = 0;

    _goertzel = new Goertzel*[_numTargetFreq];
    for (int i = 0; i < _numTargetFreq; ++i) {
        _goertzel[i] = new Goertzel(_sampleRate, _frameSize, targetFrequencies[i]);
    }

    _candidateBufIndex = 0;
    _candidateBuf = new bool[_candidateBufLen];
    for (int i = 0; i < _candidateBufLen; ++i) { _candidateBuf[i] = false; }

    _observer = new Observer(observeBufLen);
    _energyObserver = new Observer(observeBufLen);
}

void SmokeDetector::Release() {
    if (_goertzel != nullptr) {
        for (int i = 0; i < _numTargetFreq; ++i) { delete _goertzel[i]; }
        delete _goertzel; _goertzel = nullptr;
    }

    if (_candidateBuf != nullptr) {
        delete[] _candidateBuf; _candidateBuf = nullptr;
    }
    
    if (_observer != nullptr) {
        _observer->release();
        delete _observer; _observer = nullptr;
    }

    if (_energyObserver != nullptr) {
        _energyObserver->release();
        delete _energyObserver; _energyObserver = nullptr;
    }

    if (_holdOn != nullptr) {
        delete _holdOn; _holdOn = nullptr;
    }
}

AudioEventType SmokeDetector::Detect(float* data, int numSample) {
    return DetectPattern(data, numSample);
}

AudioEventType SmokeDetector::DetectPattern(float* data, int numSample) {
    if (_observer == nullptr || _holdOn == nullptr) return AUDIO_EVENT_NONE;

    bool filterOut = false;
    bool observe_prev = false;
    bool observe_now = false;
    float power = getPower(data, numSample);
    float sigPower = getSignalPower(data, numSample);

    _energyObserver->put(power > sigPower);

#ifdef AUDIO_ALGO_DEBUG
    _powerAvg = power / (float)numSample;
    _powerAvg = (_powerAvg >= 1.f) ? 1.0f : _powerAvg;
#endif

    _candidateBuf[_candidateBufIndex] = power > _threshold;
    _candidateBufIndex = (_candidateBufIndex + 1 != _candidateBufLen) ? _candidateBufIndex + 1 : 0;

    // Consider a range of frames in order to prevent sudden drop caused by audio codec processing
    filterOut = anyTrueInArray(_candidateBuf, _candidateBufLen);

#ifdef AUDIO_ALGO_DEBUG
    status = power > threshold;
#endif

    observe_prev = _observer->get();
    _observer->put(filterOut);
    observe_now = _observer->get();

    if (_holdOn->count() == 0 && _alarmCount != 0) {
        _alarmCount = 0;
        _observer->reset();
        _energyObserver->reset();
    }
    
    if (observe_prev == observe_now) { return AUDIO_EVENT_NONE; }

    int numDetected = 0;
    int numDetectedOn[NUM_ON] = {0};
    int energyDetectedOn[NUM_ON] = {0};
    int duration = int(INTERVAL_SEC * _framesPerSec);

    int index = _observer->getCurrentIndex();
    for (int intervalCount = 0; intervalCount < NUM_ON_OFF - 1; ++intervalCount) {
        int intervalCount_2 = intervalCount / 2;
        bool isEven = ((intervalCount & 1) == 0);
        for (int step = 0; step < duration; ++step) {
            if (isEven) {
                numDetectedOn[intervalCount_2] += _observer->get(index);
                energyDetectedOn[intervalCount_2] += _energyObserver->get(index);
            }
            numDetected += _observer->get(index);
            index = (index + 1 < _observer->getLength() ? index + 1 : 0);
        }
    }

    int stepRemain = _observer->getLength() - NUM_ON_OFF * duration;
    int remaining = 0;
    for (int step = 0; step < stepRemain; ++step) {
        remaining += _observer->get(index);
        index = (index + 1 < _observer->getLength() ? index + 1 : 0);
    }

    int legalCount = 0;
    for (int i = 0; i < NUM_ON; ++i) {
        if (numDetectedOn[i] > _onThreshold && energyDetectedOn[i] > _energyOnThreshold) { legalCount++; }
    }

    if ((_frameLowerBound < numDetected) && (remaining < _frameUpperBound) &&
        (legalCount >= NUM_ON)) {
        _holdOn->reset();
        _alarmCount++;

        if (_alarmCount >= 5) {
            _alarmCount = 0;
            _observer->reset();
            _energyObserver->reset();
            return AUDIO_EVENT_SMOKE;
        }
    }

    return AUDIO_EVENT_NONE;
}


float SmokeDetector::getPower(float* data, int numSample) {
    if (_goertzel == nullptr) return 0.0f;

    float power = 0.f;
    for (int i = 0; i < _numTargetFreq; ++i) { power += _goertzel[i]->calculate(data, numSample); }
    // Reduce power slightly to avoid missing alerts
    if (power < 0.000001f) {
        power = -96.0f;
    } else {
        power = 10.0f * (log10f(power) - log10f((float)numSample));
    }

    return power;
}


float SmokeDetector::getSignalPower(float *data, int numSample) {
    float power = 0.0f;
    for (int i = 0; i < numSample; i++) {
        power += data[i] * data[i];
    }
    if (power < 0.000001f) {
        power = -96.0f;
    } else {
        power = 10.0f * (log10f(power) - log10f((float)numSample));
    }

    return power;
}

void SmokeDetector::SetThreshold(float threshold) { _threshold = threshold; }
float SmokeDetector::GetThreshold() const { return _threshold; }

void SmokeDetector::ResetStates() {
    _candidateBufIndex = 0;
    if (_candidateBuf != nullptr)
        memset(_candidateBuf, 0, sizeof(bool) * _candidateBufLen);

    if (_observer != nullptr) _observer->reset();
    if (_energyObserver != nullptr) _energyObserver->reset();
    if (_holdOn != nullptr) _holdOn->reset();
}

#ifdef AUDIO_ALGO_DEBUG
float SmokeDetector::GetPowerAvg() const { return _powerAvg; }
bool SmokeDetector::GetStatus() const { return _status; }
#endif
