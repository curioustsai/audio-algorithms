/**
 *  Copyright (C) 2020, Ubiquiti Networks, Inc,
 */

#pragma once

namespace ubnt {
namespace smartaudio {

enum AudioEventType {
    AUDIO_EVENT_NONE = 0,
    AUDIO_EVENT_QUIET = 1,
    AUDIO_EVENT_LOUD = 2,
    AUDIO_EVENT_SMOKE = 3,
    AUDIO_EVENT_CO = 4,
    AUDIO_EVENT_ALL = 5
};

inline AudioEventType operator~(AudioEventType a) { return (AudioEventType) ~(int)a; }
inline AudioEventType operator|(AudioEventType a, AudioEventType b) {
    return (AudioEventType)((int)a | (int)b);
}
inline AudioEventType operator&(AudioEventType a, AudioEventType b) {
    return (AudioEventType)((int)a & (int)b);
}
inline AudioEventType operator^(AudioEventType a, AudioEventType b) {
    return (AudioEventType)((int)a ^ (int)b);
}
inline AudioEventType& operator|=(AudioEventType& a, AudioEventType b) {
    return (AudioEventType&)((int&)a |= (int)b);
}
inline AudioEventType& operator&=(AudioEventType& a, AudioEventType b) {
    return (AudioEventType&)((int&)a &= (int)b);
}
inline AudioEventType& operator^=(AudioEventType& a, AudioEventType b) {
    return (AudioEventType&)((int&)a ^= (int)b);
}

} // namespace smartaudio
} // namespace ubnt
