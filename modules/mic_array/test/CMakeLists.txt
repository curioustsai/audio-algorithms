cmake_minimum_required(VERSION 3.10)

project(test_mic_array)

# helper function to build and register a test executable
function(
    register_test
    TEST_NAME
    SRCS
)
    message(${SRCS})
    add_executable(${TEST_NAME} ${SRCS})
    target_compile_options(${TEST_NAME} PRIVATE -g)
    target_include_directories(
        ${TEST_NAME}
        PRIVATE
        ${SOURCES_ROOT}/test/include
        ${SOURCES_ROOT}/mic_array/src
    )

    target_link_libraries(
        ${TEST_NAME}
	    -L${PROJECT_ROOT}/thirdparty/ubnt_speexdsp/build/${UBNT_PLATFORM_NAME}/lib
	    -lfftw3f
        gmock
        gmock_main
        gtest
        gtest_main
        pthread
        )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

endfunction()

file(
    GLOB
    SRCS_test_mic_array
    ${SOURCES_ROOT}/mic_array/test/test_mic_array.cpp
    ${SOURCES_ROOT}/mic_array/src/*.c
)

register_test(
    test_mic_array
    "${SRCS_test_mic_array}"
)
