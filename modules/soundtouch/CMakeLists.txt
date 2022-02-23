cmake_minimum_required(VERSION 3.1.1)

#Set the name of the project
project(soundtouch)

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


if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv7.*|armv8.*|aarch64.*)$")
  set(NEON_CPU ON)
else()
  set(NEON_CPU OFF)
endif()

option(NEON "Use ARM Neon SIMD instructions if in ARM CPU" ON)
if(${NEON} AND ${NEON_CPU})
  target_compile_definitions(${PROJECT_NAME} PRIVATE SOUNDTOUCH_USE_NEON)
  target_compile_options(${PROJECT_NAME} PRIVATE -mfpu=neon)
endif()

find_package(OpenMP)
option(OPENMP "Use parallel multicore calculation through OpenMP" OFF)
if(OPENMP AND OPENMP_FOUND)
    add_definitions(-DUSE_OPEN_MP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

install(FILES ${PROJECT_ROOT}/thirdparty/${PROJECT_NAME}/include/SoundTouch.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}.a DESTINATION /lib)

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
    target_link_libraries(soundstretch PRIVATE soundtouch)
    if(INTEGER_SAMPLES)
      target_compile_definitions(soundstretch PRIVATE SOUNDTOUCH_INTEGER_SAMPLES)
    endif()

    install(TARGETS soundstretch
      DESTINATION /bin
    )
  endif()
endif()