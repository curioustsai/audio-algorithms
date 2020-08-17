#include "co_detector.h"
#include <cmath>
#include <iostream>

using namespace ubnt::smartaudio;
const static int NUM_ON = 4;
const static int NUM_ON_OFF = 8;
const static float INTERVAL_SEC = 0.1; // second

void CoDetector::Init(Config config, int* targetFrequencies, int numTargetFreq) {
	_numTargetFreq = numTargetFreq;
	_sampleRate = config.sampleRate;
	_frameSize = config.frameSize;
	_threshold = config.threshold;

	// 4 cycles of 100 ms and 100 ms off, then 5 seconds off
	// units in second
	_observeBufLen = int(INTERVAL_SEC * _sampleRate / _frameSize * 10.0);
	_frameUpperBound = int(INTERVAL_SEC * _sampleRate / _frameSize * 8.5);
	_frameLowerBound = int(INTERVAL_SEC * _sampleRate / _frameSize * 3.5);
	_holdOn = 0;
	_holdOff = 0;
	_alarmCount = 0;

	_goertzel = new Goertzel*[_numTargetFreq];
	for (int i = 0; i < _numTargetFreq; ++i) {
		_goertzel[i] = new Goertzel(_sampleRate, _frameSize, targetFrequencies[i]);
	}

	_observeBufIndex = 0;
	_observeBuf = new bool[_observeBufLen];
	for (int i = 0; i < _observeBufLen; ++i) {
		_observeBuf[i] = false;
	}
}

void CoDetector::Release() {
	for (int i = 0 ; i < _numTargetFreq; ++i) {
		delete _goertzel[i];
	}
	delete [] _goertzel;
	delete [] _observeBuf;
}


AudioEventType CoDetector::Detect(float* data, int numSample) {
	bool observePrev;
	bool observeNow;
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
	observePrev = _observeBuf[_observeBufIndex];
	_observeBuf[_observeBufIndex++] = (power > _threshold);
	_observeBufIndex = (_observeBufIndex != _observeBufLen) ? _observeBufIndex : 0;
	observeNow = _observeBuf[_observeBufIndex];

#ifdef AUDIO_ALGO_DEBUG
	_status = power > _threshold;
#endif
	
	if (_holdOn > 0) _holdOn--;
	if (_holdOn == 0) { _alarmCount = 0; }

	if (_holdOff > 0) {
		_holdOff--;
		return AUDIO_EVENT_NONE;
	}

	if (observePrev != observeNow) {
		int numDetected = 0;
		int numDetectedOn[NUM_ON] = {0};
		int duration = int(INTERVAL_SEC * _sampleRate / _frameSize);

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
			_holdOff = int(4.0 * _sampleRate / _frameSize);
			_holdOn = int(10.0 * _sampleRate / _frameSize);
			_alarmCount++;

			if (_alarmCount >= 2) {
				_alarmCount = 0;
				return AUDIO_EVENT_CO;
			}
		}
	}

	return AUDIO_EVENT_NONE;
}

void CoDetector::SetThreshold(float threshold) { _threshold = threshold; }
float CoDetector::GetThreshold() const { return _threshold; }
void CoDetector::ResetStates() {
	_holdOff = 0;
	_observeBufIndex = 0;
	for (int i = 0; i < _observeBufLen; ++i) {
		_observeBuf[i] = false;
	}
}

#ifdef AUDIO_ALGO_DEBUG
float CoDetector::GetPowerAvg() const { return _powerAvg; }
bool CoDetector::GetStatus() const { return _status; }
#endif
