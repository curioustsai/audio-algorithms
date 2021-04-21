cmake_minimum_required(VERSION 3.1.1)

#Set the name of the project
project(pffft)

#detect the source files
file (
    GLOB_RECURSE
    C_SOURCES
    "${PROJECT_ROOT}/thirdparty/pffft/*.c"
)

add_definitions(-fexceptions)

#set the include folder search path
include_directories(
    AFTER
    ${SOURCES_ROOT}/version/
    ${PROJECT_ROOT}/thirdparty/pffft/
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

#install(FILES ${PROJECT_ROOT}/thirdparty/pffft.h DESTINATION /include/${PROJECT_NAME})
#install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}.a DESTINATION /lib)
