#Set the name of the project
project(ui_athena)

#build for libathena-signal
file(
    GLOB_RECURSE
    ATHENA_SOURCE_FILES
    "${PROJECT_ROOT}/thirdparty/athena-signal/athena_signal/kernels/*.c"
    )

add_library(
    athena_signal
    ${ATHENA_SOURCE_FILES})

install(FILES "${PROJECT_ROOT}/thirdparty/athena-signal/athena_signal/kernels/dios_ssp_api.h" DESTINATION /include)
install(FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libathena_signal.a" DESTINATION /lib)

#detect the source files
file(
    GLOB_RECURSE
    CPP_SOURCES
    "${SAMPLES_ROOT}/${PROJECT_NAME}/src/*.cpp"
    )
file(
    GLOB_RECURSE
    C_SOURCES
    "${SAMPLES_ROOT}/${PROJECT_NAME}/src/*.c"
    )

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DAUDIO_ALGO_DEBUG)
endif()

include_directories(
    AFTER
    ${SOURCES_ROOT}/${PROJECT_NAME}/include/
    ${SOURCES_ROOT}/version
    ${PROJECT_ROOT}/thirdparty/libsndfile/${UBNT_PLATFORM_NAME}/include
    ${PROJECT_ROOT}/thirdparty/CLI11/include
    ${PROJECT_ROOT}/thirdparty/json/include
    ${PROJECT_ROOT}/thirdparty/athena-signal/athena_signal/kernels
    )

#print some debug info about the list of files and include folders
show_include_and_src_files()

#define the executable
add_executable(
    ${PROJECT_NAME}
    ${CPP_SOURCES}
    ${C_SOURCES}
    ${ATHENA_SOURCE_FILES}
    )

target_compile_options(${PROJECT_NAME} PRIVATE ${EXE_COMPILE_FLAGS})

message(${CMAKE_BINARY_DIR})
#define the list of library dependencies
target_link_libraries(
    ${PROJECT_NAME}
    -L${PROJECT_ROOT}/thirdparty/libsndfile/${UBNT_PLATFORM_NAME}/lib
    # -lathena_signal
    -lsndfile
    -lpthread
    -lm
    -lz
    -ldl
    -lcrypt
    -lrt
    )

#where do we want the app(s) installed
install(
    TARGETS
    ${PROJECT_NAME}
    PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
    DESTINATION
    /bin
    )
