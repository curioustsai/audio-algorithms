#include "smoke_detector.h"
#include <cmath>
#include <iostream>

using namespace ubnt::smartaudio;
const static int NUM_ON = 3;
const static int NUM_ON_OFF = 6;
const static float INTERVAL_SEC = 0.5; // second

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

    _observeBufLen = int(8.0 * INTERVAL_SEC * (float)_sampleRate / (float)_frameSize);
    _frameUpperBound = int(_observeBufLen * 4.0 / 8.0);
    _frameLowerBound = int(_observeBufLen * 2.5 / 8.0);

    _candidateBufLen = 10;
    _holdOn = 0;
    _holdOff = 0;
    _alarmCount = 0;

    _goertzel = new Goertzel* [_numTargetFreq];
    for (int i = 0; i < _numTargetFreq; ++i) {
        _goertzel[i] = new Goertzel(_sampleRate, _frameSize, targetFrequencies[i]);
    }

    _candidateBufIndex = 0;
    _candidateBuf = new bool[_candidateBufLen];
	for (int i = 0; i < _candidateBufLen; ++i) {
		_candidateBuf[i] = false;
	}

    _observeBufIndex = 0;
	_observeBuf = new bool[_observeBufLen];
	for (int i = 0; i < _observeBufLen; ++i) {
		_observeBuf[i] = false;
	}
}

void SmokeDetector::Release() {
    for (int i = 0; i < _numTargetFreq; ++i) {
        delete _goertzel[i];
    }
    delete [] _goertzel;
    delete [] _candidateBuf;
    delete [] _observeBuf;
}

AudioEventType SmokeDetector::Detect(float* data, int numSample) {
    bool filterOut;
    bool observe_prev;
    bool observe_now;
    float power;

    power = 0.f;
    for(int i = 0; i < _numTargetFreq; ++i) {
        power += _goertzel[i]->calculate(data, numSample);
    }

#ifdef AUDIO_ALGO_DEBUG
    _powerAvg = power / (float)numSample;
    _powerAvg = (_powerAvg >= 1.f) ? 1.0f : _powerAvg;
#endif

    power = 10.0f * (log10f(power) - log10f((float)numSample));
	_candidateBuf[_candidateBufIndex++] = power > _threshold;
    _candidateBufIndex = (_candidateBufIndex != _candidateBufLen) ? _candidateBufIndex : 0;

	filterOut = false;
	for (int i = 0; i < _candidateBufLen; ++i) {
		if (_candidateBuf[i]) {
			filterOut = true;
			break;
		}
	}

#ifdef AUDIO_ALGO_DEBUG
	_status = power > _threshold;
#endif

    observe_prev = _observeBuf[_observeBufIndex];
	_observeBuf[_observeBufIndex++] = filterOut;
    _observeBufIndex = (_observeBufIndex != _observeBufLen) ? _observeBufIndex : 0;
    observe_now = _observeBuf[_observeBufIndex];

	if (_holdOn > 0) { _holdOn--; } 
	if (_holdOn == 0) { _alarmCount = 0; }

    if (observe_prev != observe_now) {
        int numDetected = 0;
        int numDetectedOn[NUM_ON] = {0};
        int duration = int(INTERVAL_SEC * _sampleRate/_frameSize);

        int index = _observeBufIndex;
        for (int intervalCount = 0; intervalCount < NUM_ON_OFF; ++intervalCount) {
            int intervalCount_2 = intervalCount / 2;

            for (int step = 0; step < duration; ++step) {
                index = (index != _observeBufLen) ? index : 0;
                if ((intervalCount % 2) == 0) {
                    numDetectedOn[intervalCount_2] += _observeBuf[index];
                }
                numDetected += _observeBuf[index];
                index++;
            }
        }

        int stepRemain = _observeBufLen - NUM_ON_OFF * duration;
        for (int step = 0; step < stepRemain; ++step) {
            index = (index != _observeBufLen) ? index : 0;
            numDetected += _observeBuf[index];
            index++;
        }

        int legalCount = 0;
        int threshold = int (INTERVAL_SEC * _sampleRate/_frameSize * 0.7);
        for (int i = 0; i < NUM_ON; ++i) {
            if (numDetectedOn[i] > threshold) {
                legalCount++;
            }
        }

        if ((_frameLowerBound < numDetected) && (numDetected < _frameUpperBound) && (legalCount == NUM_ON)) {
			_holdOn = int(5.0 * _sampleRate / _frameSize);
			_alarmCount++;

			if (_alarmCount >= 2) {
				_alarmCount = 0;
            	return AUDIO_EVENT_SMOKE;
			}
        }
    }

    return AUDIO_EVENT_NONE;
}

void SmokeDetector::SetThreshold(float threshold) { _threshold = threshold; }
float SmokeDetector::GetThreshold() const { return _threshold; }

void SmokeDetector::ResetStates() {
    _holdOff = 0;
    _candidateBufIndex = 0;
    for (int i = 0; i < _candidateBufLen; ++i) {
        _candidateBuf[i] = false;
    }

    _observeBufIndex = 0;
    for (int i = 0; i < _observeBufLen; ++i) {
        _observeBuf[i] = false;
    }
}

#ifdef AUDIO_ALGO_DEBUG
float SmokeDetector::GetPowerAvg() const { return _powerAvg; }
bool SmokeDetector::GetStatus() const { return _status; }
#endif
