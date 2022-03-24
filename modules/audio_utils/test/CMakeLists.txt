cmake_minimum_required(VERSION 3.10)

project(test_audio_utils)

# helper function to build and register a test executable
function(
    register_test
    TEST_NAME
    SRCS
)
    add_executable(${TEST_NAME} ${SRCS})
    target_compile_options(${TEST_NAME} PRIVATE -g)
    target_include_directories(
        ${TEST_NAME}
        PRIVATE
        ${SOURCES_ROOT}/test/include
    )

    target_link_libraries(
        ${TEST_NAME}
        -L${PROJECT_ROOT}/thirdparty/libsndfile/${UBNT_PLATFORM_NAME}/lib
        audio_utils
        sndfile
        gmock
        gmock_main
        gtest
        gtest_main
        pthread
        )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

endfunction()

file(
    GLOB
    SRCS_test_audio_utils
    ${SOURCES_ROOT}/audio_utils/test/*.cpp
)

include_directories(
    AFTER
    ${PROJECT_ROOT}/thirdparty/libsndfile/${UBNT_PLATFORM_NAME}/include
)

register_test(
    test_audio_utils
    "${SRCS_test_audio_utils}"
)
