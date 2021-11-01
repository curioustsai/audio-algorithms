project(speexdsp-builder)

if(${UBNT_PLATFORM_NAME} STREQUAL "x86")
execute_process(
    COMMAND ${PROJECT_SOURCE_DIR}/build-speexdsp.sh "-p ${UBNT_PLATFORM_NAME} -r ${PROJECT_ROOT}"
    )
else()
# Change space to @ in order to fool getopts that the following is not arguments
string(REPLACE " " "@" FLAG_OPTION ${UBNT_COMPILE_OPTION})
execute_process(
    COMMAND ${PROJECT_SOURCE_DIR}/build-speexdsp.sh "-p ${UBNT_PLATFORM_NAME} -h ${UBNT_PLATFORM_HOST} -t ${UBNT_FW_CACHE}/${UBNT_PLATFORM_NAME}/toolchains/bin -r ${PROJECT_ROOT} -f ${FLAG_OPTION}"
    )
endif()
