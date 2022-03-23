# embed git commit sha1 in each modules
configure_file(
    "${SOURCES_ROOT}/version/audio_version.cpp.in" "${CMAKE_BINARY_DIR}/audio_version.cpp"
    @ONLY
)
set(VERSION_SOURCES "${CMAKE_BINARY_DIR}/audio_version.cpp"
                    "${SOURCES_ROOT}/version/audio_version.h"
)

add_subdirectory(ubnt_logger)
add_subdirectory(soundtouch)

if(${UBNT_PLATFORM_NAME} MATCHES "gen[34].")
    add_subdirectory(audio_utils)
    add_subdirectory(audio_events)
    add_subdirectory(pffft)
    add_subdirectory(formant_shift)
    add_subdirectory(compressor)
endif()

if(${UBNT_PLATFORM_NAME} STREQUAL "gen4l")
    add_subdirectory(speexdsp-builder)
endif()

if(${UBNT_PLATFORM_NAME} STREQUAL "gen4c")
    add_subdirectory(click_removal)
    add_subdirectory(speexdsp-builder)
endif()

if(${UBNT_PLATFORM_NAME} STREQUAL "x86")
    add_subdirectory(pffft)
    add_subdirectory(audio_events)
    add_subdirectory(mic_array)
    add_subdirectory(formant_shift)
    add_subdirectory(click_removal)
    add_subdirectory(audio_utils)
    add_subdirectory(speexdsp-builder)
    add_subdirectory(compressor)
endif()

if(UBNT_WITH_TESTS)
    add_subdirectory(test)
endif()
