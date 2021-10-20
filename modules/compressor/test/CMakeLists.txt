cmake_minimum_required(VERSION 3.10)

project(test_compressor)

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
        ${SOURCES_ROOT}/compressor/include
    )

    target_link_libraries(
        ${TEST_NAME}
        compressor
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
    SRCS_test_compressor
    ${SOURCES_ROOT}/compressor/test/*.cpp
)

register_test(
    test_compressor
    "${SRCS_test_compressor}"
)
