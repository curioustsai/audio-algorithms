cmake_minimum_required(VERSION 3.10)

project(test)

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
        # gmock
        gtest
        gtest_main
        pthread
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

endfunction()

# test_detect_utils
file(
    GLOB
    SRCS_test_detect_utils
    ${SOURCES_ROOT}/detection/test/test_detect_utils.cpp
)
register_test(
    test_detect_utils
    "${SRCS_test_detect_utils}"
)


