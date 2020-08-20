//
// Created by richard on 7/9/20.
//

#pragma once

namespace ubnt {
namespace smartaudio {

class Goertzel {
public:
    Goertzel(int sample_rate, int frame_size, int target_freq);
    float calculate(const float *data, int num_sample) const;

private:
    int _sampleRate;
    int _frameSize;
    int _targetFreq;
    float _coefficient;
};
} // namespace smartaudio
} // namespace ubnt
