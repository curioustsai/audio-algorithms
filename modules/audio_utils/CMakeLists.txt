#Set the name of the project
project(audio_utils)

#detect the source files
glob_source_files()

add_definitions(-fexceptions)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DAUDIO_ALGO_DEBUG)
endif()


#set the include folder search path
include_directories(
    AFTER
    ${SOURCES_ROOT}/${PROJECT_NAME}/include
    ${SOURCES_ROOT}/version/
)

#print some debug info about the list of files and include folders
show_include_and_src_files()

#define the executable
add_library(
    ${PROJECT_NAME}
    ${UBNT_LIBRARIES_TYPE}
    ${VERSION_SOURCES}
    ${CPP_SOURCES}
    ${C_SOURCES}
)

install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/biquad.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/equalizer.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/highpass.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/lowpass.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/gain.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/comfort_noise.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}.a DESTINATION /lib)

if(UBNT_WITH_TESTS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
	add_subdirectory(test)
endif()
