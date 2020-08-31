#cmake requirements
cmake_minimum_required(VERSION 3.0.2)

#Set the name of the project
project(all)

#include some utility functionalities
include(cmake/utils/debug.cmake)
include(cmake/utils/glob_source_files.cmake)

OPTION(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
OPTION(UBNT_WITH_TESTS "Compile test" OFF)

if(NOT DEFINED UBNT_PLATFORM_NAME)
    set(UBNT_PLATFORM_NAME x86)
endif()

#set the root folder
if(NOT DEFINED PROJECT_ROOT)
    get_filename_component(
        PROJECT_ROOT
        "./"
        ABSOLUTE
    )
endif()
get_filename_component(
    BUILD_OUTPUT_DIR
    "${PROJECT_ROOT}/build"
    ABSOLUTE
)
get_filename_component(
    SOURCES_ROOT
    "${PROJECT_ROOT}/modules"
    ABSOLUTE
)

get_filename_component(
    SAMPLES_ROOT
    "${PROJECT_ROOT}/samples"
    ABSOLUTE
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_ROOT}/cmake/utils/gitsha/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

message(STATUS "PROJECT_ROOT: ${PROJECT_ROOT}")
message(STATUS "SOURCES_ROOT: ${SOURCES_ROOT}")
message(STATUS "SAMPLES_ROOT: ${SAMPLES_ROOT}")


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/${UBNT_PLATFORM_NAME}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/${UBNT_PLATFORM_NAME}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/${UBNT_PLATFORM_NAME})

if(NOT DEFINED UBNT_LIBRARIES_TYPE)
    set(UBNT_LIBRARIES_TYPE STATIC)
endif()
message(STATUS "Libraries type will be: ${UBNT_LIBRARIES_TYPE}")

#define log level
if(AUDIO_LOG_LEVEL STREQUAL "debug")
    add_definitions(-DAUDIO_LOG_DEBUG)
elseif(AUDIO_LOG_LEVEL STREQUAL "info")
    add_definitions(-DAUDIO_LOG_INFO)
elseif(AUDIO_LOG_LEVEL STREQUAL "off")
    add_definitions(-DAUDIO_LOG_OFF)
else()
endif()

message(STATUS "Set AUDIO Log Level: ${AUDIO_LOG_LEVEL}")

#mandatory include and link folders from sys root
include_directories(
    AFTER
    ${CMAKE_FIND_ROOT_PATH}/usr/include
)
link_directories(
    ${CMAKE_FIND_ROOT_PATH}/lib
    ${CMAKE_FIND_ROOT_PATH}/usr/lib
)

#mandatory compile flags for all platforms
add_compile_options(
    -fno-PIC
    -fpic
    -Wall
    -Werror
    -ffunction-sections
    -fdata-sections
)

set(
    EXE_COMPILE_FLAGS
    ${EXE_COMPILE_FLAGS}
    -fvisibility=hidden
    -fno-pic
    -fpie
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g)
else()
    set(
        EXE_COMPILE_FLAGS
        ${EXE_COMPILE_FLAGS}
        -flto
        -Os
    )
endif()

set(
    CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--as-needed"
)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")

#if we are going to build static, shrink everything even more by making all
#symbols hidden
if(${UBNT_LIBRARIES_TYPE} MATCHES STATIC)
    add_compile_options(-fvisibility=hidden)
endif()

# use c++11 for all subprojects by default
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(UBNT_WITH_TESTS)
    include(CTest)
endif(UBNT_WITH_TESTS)

#Compile targets
add_subdirectory(modules)
add_subdirectory(samples)
