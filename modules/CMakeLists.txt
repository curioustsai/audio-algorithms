# embed git commit sha1 in each modules
configure_file("${SOURCES_ROOT}/version/version.cpp.in" "${CMAKE_BINARY_DIR}/version.cpp" @ONLY)
set(VERSION_SOURCES "${CMAKE_BINARY_DIR}/version.cpp" "${SOURCES_ROOT}/version/version.h")

if (${UBNT_PLATFORM_NAME} MATCHES "gen[34].")
	add_subdirectory(audio_events)
endif()

if (${UBNT_PLATFORM_NAME} STREQUAL "gen4l")
	add_subdirectory(pffft-git)
	add_subdirectory(mic_array)
	add_subdirectory(formant_shift)
endif()

if (${UBNT_PLATFORM_NAME} STREQUAL "x86")
	add_subdirectory(pffft-git)
	add_subdirectory(audio_events)
	add_subdirectory(mic_array)
	add_subdirectory(formant_shift)
endif()

if(UBNT_WITH_TESTS)
	add_subdirectory(test)
endif()
