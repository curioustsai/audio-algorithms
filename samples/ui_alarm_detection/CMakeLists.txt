#Set the name of the project
project(ui_alarm_detection)

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


#set the include folder search path
find_package(PythonLibs 2.7)
message("STATUS PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message("STATUS PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")

# set(SndFile_DIR, "/home/richard/workspace/libsndfile/build")
# list(APPEND CMAKE_MODULE_PATH "/home/richard/workspace/libsndfile/CMakeBuild")

# find_package(SndFile REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE SndFile::sndfile)
# find_path(LIBSNDFILE_INCLUDE_DIRS sndfile.h)
# find_library(LIBSNDFILE_LIBRARIES sndfile)

include_directories(
    AFTER
    ${SOURCES_ROOT}/${PROJECT_NAME}/include/
    ${SOURCES_ROOT}/alarm_detection/include/
    ${PROJECT_ROOT}/thirdparty/libsndfile/include
	${PYTHON_INCLUDE_DIRS}
    ${PROJECT_ROOT}/thirdparty/matplotlib-cpp
	${PROJECT_ROOT}/thirdparty/CLI11/include
	${PROJECT_ROOT}/thirdparty/spdlog/include
	${LIBSNDFILE_INCLUDE_DIRS}
)

#print some debug info about the list of files and include folders
show_include_and_src_files()

#define the executable
add_executable(
    ${PROJECT_NAME}
    ${CPP_SOURCES}
    ${C_SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE ${EXE_COMPILE_FLAGS})

#define the list of library dependencies
target_link_libraries(
    ${PROJECT_NAME}
    ${PYTHON_LIBRARIES}
	${LIBSNDFILE_LIBRARIES}
	${PROJECT_ROOT}/thirdparty/libsndfile/lib/libsndfile.a
    alarm_detection
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
