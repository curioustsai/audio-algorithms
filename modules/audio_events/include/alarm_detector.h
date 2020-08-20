//
// Created by richard on 7/9/20.
//

#pragma once

#include "audio_event_type.h"
#include "goertzel.h"

namespace ubnt {
namespace smartaudio {

typedef struct Config_ {
    int sampleRate;
    int frameSize;
    float threshold;
} Config;

class AlarmDetector {
public:
    virtual AudioEventType Detect(float *data, int num_sample) = 0;
    virtual void SetThreshold(float threshold) = 0;
    virtual float GetThreshold() const = 0;
};
} // namespace smartaudio
} // namespace ubnt
