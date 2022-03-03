#include <cmath>
#include "co_detector.h"
#include "goertzel.h"
#include "countdown.h"
#include "observer.h"

using namespace ubnt::smartaudio;
const static int NUM_ON = 4;
const static int NUM_ON_OFF = 8;
const static float INTERVAL_SEC = 0.1f; // second

void CoDetector::Init(Config config, int* targetFrequencies, int numTargetFreq) {
    _numTargetFreq = numTargetFreq;
    _sampleRate = config.sampleRate;
    _frameSize = config.frameSize;
    _threshold = config.threshold;
    _framesPerSec = (float)_sampleRate / (float)_frameSize;

    _onThreshold = int(INTERVAL_SEC * _framesPerSec * 0.7);
    _energyOnThreshold = int(INTERVAL_SEC * _framesPerSec * 0.84);

    // 4 cycles of 100 ms and 100 ms off, then 5 seconds off
    // units in second
    _frameUpperBound = int(INTERVAL_SEC * _framesPerSec * 8.75f);
    _frameLowerBound = int(INTERVAL_SEC * _framesPerSec * 3.5f);
    _holdOn =
        new CountDown(static_cast<unsigned int>(10.0f * _framesPerSec));
    _holdOn->setCounter(0);
    _holdLong =
        new CountDown(static_cast<unsigned int>(4.0f * _framesPerSec));
    _holdLong->setCounter(0);

    _alarmCount = 0;

    _goertzel = new Goertzel*[_numTargetFreq];
    for (int i = 0; i < _numTargetFreq; ++i) {
        _goertzel[i] = new Goertzel(_sampleRate, _frameSize, targetFrequencies[i]);
    }

    _shortObserver =
        new Observer(int(INTERVAL_SEC * _framesPerSec * 10.0));
    _energyObserver =
        new Observer(int(INTERVAL_SEC * _framesPerSec * 10.0));
    _longObserver = new Observer(int(10.6f * _framesPerSec));
}

void CoDetector::Release() {
    if (_goertzel != nullptr) {
        for (int i = 0; i < _numTargetFreq; ++i) { delete _goertzel[i]; }
        delete[] _goertzel;
        _goertzel = nullptr;
    }

    if (_holdOn != nullptr) {
        delete _holdOn;
        _holdOn = nullptr;
    }

    if (_holdLong != nullptr) {
        delete _holdLong;
        _holdLong = nullptr;
    }

    if (_shortObserver != nullptr) {
        _shortObserver->release();
        delete _shortObserver;
        _shortObserver = nullptr;
    }

    if (_energyObserver != nullptr) {
        _energyObserver->release();
        delete _energyObserver;
        _energyObserver = nullptr;
    }

    if (_longObserver != nullptr) {
        _longObserver->release();
        delete _longObserver;
        _longObserver = nullptr;
    }
}

AudioEventType CoDetector::Detect(float* data, int numSample) {
    float power = getPower(data, numSample);
    float sigPower = getSignalPower(data, numSample);
    _energyObserver->put(power > sigPower);

    return DetectLongPattern(power);
}

void CoDetector::SetThreshold(float threshold) { _threshold = threshold; }

float CoDetector::GetThreshold() const { return _threshold; }

void CoDetector::ResetStates() {
    if (_holdOn != nullptr) _holdOn->reset();
    if (_holdLong != nullptr) _holdLong->reset();
    if (_shortObserver != nullptr) _shortObserver->reset();
    if (_energyObserver != nullptr) _energyObserver->reset();
    if (_longObserver != nullptr) _longObserver->reset();
}

float CoDetector::getPower(float* data, int numSample) {
    if (_goertzel == nullptr) return 0.0f;

    float power = 0.f;
    for (int i = 0; i < _numTargetFreq; ++i) { power += _goertzel[i]->calculate(data, numSample); }

    if (power < 0.000001f) {
        power = -96.0f;
    }
    else {
        power = 10.0f * (log10f(power) - log10f((float)numSample));
    }

    return power;
}

float CoDetector::getSignalPower(float* data, int numSample) {
    float power = 0.0f;
    for (int i = 0; i < numSample; i++) { power += data[i] * data[i]; }

    if (power < 0.000001f) {
        power = -96.0f;
    } else {
        power = 10.0f * (log10f(power) - log10f((float)numSample));
    }
    

    return power;
}

AudioEventType CoDetector::DetectShortPattern(float power) {
    bool observePrev = false;
    bool observeNow = false;

    if (_shortObserver == nullptr || _holdOn == nullptr) {
        return AUDIO_EVENT_NONE;
    }

#ifdef AUDIO_ALGO_DEBUG
    powerAvg = power / (float)numSample;
    powerAvg = (powerAvg >= 1.f) ? 1.0f : powerAvg;
#endif

    observePrev = _shortObserver->get();
    _shortObserver->put((power > _threshold));
    observeNow = _shortObserver->get();

#ifdef AUDIO_ALGO_DEBUG
    _status = power > _threshold;
#endif

    if (_holdOn->count() == 0 && _alarmCount != 0) {
        _alarmCount = 0;
        _shortObserver->reset();
        // _energyObserver->reset();
    }
    
    if (observePrev == observeNow) { return AUDIO_EVENT_NONE; }

    int numDetected = 0;
    int numDetectedOn[NUM_ON] = {0};
    int energyDetectedOn[NUM_ON] = {0};
    int duration = int(INTERVAL_SEC * (float)_sampleRate / (float)_frameSize);

    int index = _shortObserver->getCurrentIndex();
    for (int intervalCount = 0; intervalCount < NUM_ON_OFF; ++intervalCount) {
        int intervalCount_2 = intervalCount / 2;
        bool isEven = ((intervalCount & 1) == 0);
        for (int step = 0; step < duration; ++step) {
            if (isEven) {
                numDetectedOn[intervalCount_2] += _shortObserver->get(index);
                energyDetectedOn[intervalCount_2] += _energyObserver->get(index);
            }
            numDetected += _shortObserver->get(index);
            index = (index + 1 < _shortObserver->getLength() ? index + 1 : 0);
        }
    }

    int stepRemain = _shortObserver->getLength() - NUM_ON_OFF * duration;
    for (int step = 0; step < stepRemain; ++step) {
        numDetected += _shortObserver->get(index);
        index = (index + 1 < _shortObserver->getLength() ? index + 1 : 0);
    }

    int legalCount = 0;
    for (int i = 0; i < NUM_ON; ++i) {
        if (numDetectedOn[i] > _onThreshold && energyDetectedOn[i] > _energyOnThreshold) { legalCount++; }
    }
    
    if ((_frameLowerBound < numDetected) && (numDetected < _frameUpperBound) && (legalCount >= NUM_ON)) {
        _holdOn->reset();
        _alarmCount++;

        if (_alarmCount >= 2) {
            _alarmCount = 0;
            _shortObserver->reset();
            return AUDIO_EVENT_CO;
        }
    }

    return AUDIO_EVENT_NONE;
}

AudioEventType CoDetector::DetectLongPattern(float power) {
    if (_longObserver == nullptr || _holdLong == nullptr) { return AUDIO_EVENT_NONE; }

    _longObserver->put(power > _threshold);
    const int upperBound = int(_framesPerSec * 5.8f * 0.3f);
    const int lowerBound = _frameLowerBound * 2;

    if (DetectShortPattern(power) == AUDIO_EVENT_CO) {
        _shortPatternDetected = true;
        _holdLong->reset();
    }

    if (_shortPatternDetected && _holdLong->count() == 0) {
        int numDetected = _longObserver->sum();

        if ((lowerBound < numDetected) && (numDetected < upperBound)) {
            _alarmCountLong++;
            if (_alarmCountLong >= 2) {
                _alarmCountLong = 0;
                _longObserver->reset();
                return AUDIO_EVENT_CO;
            }
        }
        _shortPatternDetected = false;
    }

    return AUDIO_EVENT_NONE;
}

#ifdef AUDIO_ALGO_DEBUG
float CoDetector::GetPowerAvg() const { return _powerAvg; }
bool CoDetector::GetStatus() const { return _status; }
#endif
