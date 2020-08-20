#pragma once

namespace ubnt {
namespace smartaudio {

enum AudioEventType {
    AUDIO_EVENT_NONE = 0,
    AUDIO_EVENT_QUIET = 0x1,
    AUDIO_EVENT_LOUD = 0x2,
    AUDIO_EVENT_SMOKE = 0x4,
    AUDIO_EVENT_CO = 0x8
};

template <class T>
inline T operator~(T a) {
    return (T) ~(int)a;
}
template <class T>
inline T operator|(T a, T b) {
    return (T)((int)a | (int)b);
}
template <class T>
inline T operator&(T a, T b) {
    return (T)((int)a & (int)b);
}
template <class T>
inline T operator^(T a, T b) {
    return (T)((int)a ^ (int)b);
}
template <class T>
inline T& operator|=(T& a, T b) {
    return (T&)((int&)a |= (int)b);
}
template <class T>
inline T& operator&=(T& a, T b) {
    return (T&)((int&)a &= (int)b);
}
template <class T>
inline T& operator^=(T& a, T b) {
    return (T&)((int&)a ^= (int)b);
}
template <AudioEventType>
inline AudioEventType operator~(AudioEventType a);
template <AudioEventType>
inline AudioEventType operator|(AudioEventType a, AudioEventType b);
template <AudioEventType>
inline AudioEventType operator&(AudioEventType a, AudioEventType b);
template <AudioEventType>
inline AudioEventType operator^(AudioEventType a, AudioEventType b);
template <AudioEventType>
inline AudioEventType& operator|=(AudioEventType a, AudioEventType b);
template <AudioEventType>
inline AudioEventType& operator&=(AudioEventType a, AudioEventType b);
template <AudioEventType>
inline AudioEventType& operator^=(AudioEventType a, AudioEventType b);
} // namespace smartaudio
} // namespace ubnt
