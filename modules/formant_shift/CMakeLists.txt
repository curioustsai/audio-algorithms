#Set the name of the project
project(formant_shift)

#detect the source files
glob_source_files()

add_definitions(-fexceptions)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DAUDIO_ALGO_DEBUG)
endif()

# Save project-releated include paths into variable
set(
    ${PROJECT_NAME}_include_dirs
    ${pffft_include_dirs}
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

#define the executable
add_library(
    ${PROJECT_NAME}
    ${UBNT_LIBRARIES_TYPE}
    ${VERSION_SOURCES}
    ${CPP_SOURCES}
    ${C_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    pffft
)

if(UBNT_WITH_TESTS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
	add_subdirectory(test)
endif()

