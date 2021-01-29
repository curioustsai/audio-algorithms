if(${UBNT_PLATFORM_NAME} MATCHES "gen3.")
endif()

if(${UBNT_PLATFORM_NAME} MATCHES "gen4.")
add_subdirectory(ui_audio_events)
endif()

if(${UBNT_PLATFORM_NAME} STREQUAL "x86")
add_subdirectory(ui_audio_events)
add_subdirectory(ui_speexdsp)
add_subdirectory(clear_voice)
endif()
