cmake_minimum_required(VERSION 3.1.1)

#Set the name of the project
project(pffft)

#detect the source files
glob_source_files()
file (
    GLOB_RECURSE
    PFFFT_C_SOURCES
    "${PROJECT_ROOT}/thirdparty/pffft/fftpack.c"
    "${PROJECT_ROOT}/thirdparty/pffft/pffft.c"
)

add_definitions(-fexceptions)

# Save project-releated include paths into variable
set(
    ${PROJECT_NAME}_include_dirs
    ${PROJECT_ROOT}/thirdparty/pffft/
    ${SOURCES_ROOT}/${PROJECT_NAME}/include
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
    ${PFFFT_C_SOURCES}
    ${VERSION_SOURCES}
    ${CPP_SOURCES}
    ${C_SOURCES}
)

install(FILES ${SOURCES_ROOT}/${PROJECT_NAME}/include/pffftwrap.h DESTINATION /include/${PROJECT_NAME})
# install(FILES ${PROJECT_ROOT}/thirdparty/pffft/pffft.h DESTINATION /include/${PROJECT_NAME})
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}.a DESTINATION /lib)
# install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}-ori.a DESTINATION /lib)


# if(UBNT_WITH_TESTS)
# 	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
# 	add_subdirectory(test)
# endif()
