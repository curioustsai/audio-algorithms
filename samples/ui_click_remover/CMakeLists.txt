#Set the name of the project
project(ui_click_remover)

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
    ${SOURCES_ROOT}/click_removal/include/
    ${SOURCES_ROOT}/version
    ${PROJECT_ROOT}/thirdparty/libsndfile/${UBNT_PLATFORM_NAME}/include
	${PROJECT_ROOT}/thirdparty/CLI11/include
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
	${PROJECT_ROOT}/thirdparty/libsndfile/${UBNT_PLATFORM_NAME}/lib/libsndfile.a
    click_removal
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
