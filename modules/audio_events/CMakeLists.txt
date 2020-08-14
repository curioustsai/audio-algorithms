#Set the name of the project
project(audio_events)

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

install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/goertzel.h DESTINATION /include)
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/alarm_detector.h DESTINATION /include)
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/co_detector.h DESTINATION /include)
install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/smoke_detector.h DESTINATION /include)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}.a DESTINATION /lib)
