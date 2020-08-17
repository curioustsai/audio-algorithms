#pragma once

namespace ubnt {
namespace smartaudio {

enum AudioEventType {
    AUDIO_EVENT_NONE = 0,
    AUDIO_EVENT_SMOKE = 0x1,
    AUDIO_EVENT_CO = 0x2,
    AUDIO_EVENT_LOUD = 0x4,
    AUDIO_EVENT_QUIET = 0x8
};
}  // namespace smartaudio
}  // namespace ubnt
