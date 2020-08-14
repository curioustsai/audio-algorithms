#include <cmath>
#include "goertzel.h"
#include <iostream>

using namespace ubnt::smartaudio;

Goertzel::Goertzel (int sample_rate, int frame_size, int target_freq) {
	float k, omega, cos_omega;

	_sampleRate = sample_rate;
	_frameSize = frame_size;
	_targetFreq = target_freq;

	k  = roundf(0.5f + (float)_frameSize * (float)_targetFreq / (float)_sampleRate);
	omega = 2.0 * M_PI * k / frame_size;
	cos_omega = cosf(omega);
	_coefficient = 2 * cos_omega;

}

float Goertzel::calculate(const float* data, int num_sample) const {
	float state, state1 = 0, state2 = 0;
	float power;

	for (int i = 0; i < num_sample; i++) {
		state = (_coefficient * state1) - state2 + data[i];
		state2 = state1;
		state1 = state;
	}
    power = (state1 * state1) + (state2 * state2) - (state1 * state2 * _coefficient);

	return power;
}
