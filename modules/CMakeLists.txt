# embed git commit sha1 in each modules
configure_file("${SOURCES_ROOT}/version/version.cpp.in" "${CMAKE_BINARY_DIR}/version.cpp" @ONLY)
set(VERSION_SOURCES "${CMAKE_BINARY_DIR}/version.cpp" "${SOURCES_ROOT}/version/version.h")

add_subdirectory(alarm_detection)

