#include "smoke_detector.h"
#include <cmath>
#include <iostream>

using namespace ubnt::smartaudio;

void SmokeDetector::Init(Config config, int* targetFrequencies, int numTargetFreq) {
    _numTargetFreq = numTargetFreq;
    _sampleRate = config.sampleRate;
    _frameSize = config.frameSize;
    _threshold = config.threshold;

    _observeBufLen = int(8.0 * 0.5 * (float)_sampleRate / (float)_frameSize);
    _observeFrameNumHead = int(0.5 * (float)_sampleRate / (float)_frameSize);
    _observeFrameNumTail = int(1.5 * (float)_sampleRate / (float)_frameSize);
    _frameUpperBound = int(_observeBufLen * 4.0 / 8.0);
    _frameLowerBound = int(_observeBufLen * 3.0 / 8.0);

    _candidateBufLen = 10;
    _holdOff = 0;

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

bool SmokeDetector::Detect(float* data, int numSample) {
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

    if (_holdOff > 0) {
        _holdOff--;
        return false;
    }

    if (observe_prev != observe_now) {
        int num_detected = 0;
        int num_detected_head = 0;
		int num_detected_tail = 0;

		for (int index = _observeBufIndex, step = 0; step < _observeBufLen; ++index, ++step) {
		    index = (index != _observeBufLen) ? index : 0;

			num_detected += _observeBuf[index];
			if (step < _observeFrameNumHead) {
				num_detected_head += _observeBuf[index];
			}
			else if (step > (_observeBufLen - _observeFrameNumTail)) {
				num_detected_tail += _observeBuf[index];
			}
		}

        num_detected_tail = _observeFrameNumTail - num_detected_tail;

        if ((_frameLowerBound < num_detected) && (num_detected < _frameUpperBound) &&
        (num_detected_head > (_observeFrameNumHead * 0.9)) &&
        (num_detected_tail > (_observeFrameNumTail * 0.8))) {
            _holdOff = int(0.5 * _sampleRate / _frameSize);
            return true;
        }
    }

    return false;
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
