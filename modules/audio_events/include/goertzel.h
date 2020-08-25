/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

namespace ubnt {
namespace smartaudio {

class Goertzel {
public:
    Goertzel(int sample_rate, int frame_size, int target_freq);
    float calculate(const float *data, int num_sample) const;

private:
    int _sampleRate{48000};
    int _frameSize{128};
    int _targetFreq{0};
    float _coefficient{0.0};
};
} // namespace smartaudio
} // namespace ubnt
