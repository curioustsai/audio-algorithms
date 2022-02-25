cmake_minimum_required(VERSION 3.1.1)

#Set the name of the project
project(SoundTouch)

#detect the source files
glob_source_files()
file (
    GLOB_RECURSE
    SOUNDTOUCH_C_SOURCES
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/AAFilter.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/BPMDetect.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/cpu_detect_x86.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/FIFOSampleBuffer.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/FIRFilter.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/InterpolateCubic.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/InterpolateLinear.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/InterpolateShannon.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/mmx_optimized.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/PeakFinder.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/RateTransposer.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/SoundTouch.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/sse_optimized.cpp"
    "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundTouch/TDStretch.cpp"
)

add_definitions(-fexceptions)

# Save project-releated include paths into variable
set(
    ${PROJECT_NAME}_include_dirs
    ${PROJECT_ROOT}/thirdparty/soundtouch/include
    CACHE INTERNAL "" FORCE
)

#set the include folder search path
include_directories(
    AFTER
    ${SOURCES_ROOT}/version/
    ${${PROJECT_NAME}_include_dirs}
)

#print some debug info about the list of files and include folders
show_include_and_src_files()

add_library(
    ${PROJECT_NAME}
    STATIC
    ${SOUNDTOUCH_C_SOURCES}
    ${VERSION_SOURCES}
    ${CPP_SOURCES}
    ${C_SOURCES}
)

option(INTEGER_SAMPLES "Use integers instead of floats for samples" ON)
if(INTEGER_SAMPLES)
  set(SOUNDTOUCH_INTEGER_SAMPLES, 1)
  target_compile_definitions(${PROJECT_NAME} PRIVATE SOUNDTOUCH_INTEGER_SAMPLES)
else()
  set(SOUNDTOUCH_FLOAT_SAMPLES, 1)
  target_compile_definitions(${PROJECT_NAME} PRIVATE SOUNDTOUCH_FLOAT_SAMPLES)
endif()


if(${UBNT_PLATFORM_HOST} MATCHES "^(armv7.*|armv8.*|aarch64.*)$")
  set(NEON_CPU ON)
else()
  set(NEON_CPU OFF)
endif()

option(NEON "Use ARM Neon SIMD instructions if in ARM CPU" ON)
if(${NEON} AND ${NEON_CPU})
  set(SOUNDTOUCH_USE_NEON, 1)
  target_compile_definitions(${PROJECT_NAME} PRIVATE SOUNDTOUCH_USE_NEON)
endif()

install(
  FILES
    ${PROJECT_ROOT}/thirdparty/soundtouch/include/SoundTouch.h
    ${PROJECT_ROOT}/thirdparty/soundtouch/include/BPMDetect.h
    ${PROJECT_ROOT}/thirdparty/soundtouch/include/FIFOSampleBuffer.h
    ${PROJECT_ROOT}/thirdparty/soundtouch/include/FIFOSamplePipe.h
    ${PROJECT_ROOT}/thirdparty/soundtouch/include/STTypes.h
    ${SOURCES_ROOT}/soundtouch/soundtouch_config.h
  DESTINATION
    /include/${PROJECT_NAME}
)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libSoundTouch.a DESTINATION /lib)

# if(UBNT_WITH_TESTS)
# 	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
# 	add_subdirectory(test)
# endif()


#######################
# soundstretch utility
if(${UBNT_PLATFORM_NAME} STREQUAL "x86")
  option(SOUNDSTRETCH "Build soundstretch command line utility." ON)
  if(SOUNDSTRETCH)
    add_executable(soundstretch
      "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundStretch/main.cpp"
      "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundStretch/RunParameters.cpp"
      "${PROJECT_ROOT}/thirdparty/soundtouch/source/SoundStretch/WavFile.cpp"
    )
    target_include_directories(soundstretch
      PUBLIC
      ${SOURCES_ROOT}/version/
      ${${PROJECT_NAME}_include_dirs}
    )
    target_compile_definitions(soundstretch PRIVATE ${COMPILE_DEFINITIONS})
    target_compile_options(soundstretch PRIVATE ${COMPILE_OPTIONS})
    target_link_libraries(soundstretch PRIVATE ${PROJECT_NAME})
    if(INTEGER_SAMPLES)
      target_compile_definitions(soundstretch PRIVATE SOUNDTOUCH_INTEGER_SAMPLES)
    endif()

    install(TARGETS soundstretch
      DESTINATION /bin
    )
  endif()
endif()