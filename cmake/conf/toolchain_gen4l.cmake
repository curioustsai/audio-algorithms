#gen4l toolchain descriptor

#the brand of the OS
set(CMAKE_SYSTEM_NAME Linux)

#this one is required by cmake, but kinda obscure
set(CMAKE_SYSTEM_VERSION 1)

#where the C and C++ compilers are located
set(
    CMAKE_C_COMPILER
    ${UBNT_FW_CACHE}/gen4l/toolchains/bin/aarch64-linux-gnu-gcc
)
set(
    CMAKE_CXX_COMPILER
    ${UBNT_FW_CACHE}/gen4l/toolchains/bin/aarch64-linux-gnu-g++
)

#where is the sysroot
set(
    CMAKE_FIND_ROOT_PATH
    ${UBNT_FW_CACHE}/gen4l/barebones/staging_dir/target-aarch64-linux-gnu_glibc
)

#never search for utility apps in the target
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

#always search for libs and headers in target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set the UBNT platform name, used across the cmake builder to include the necessary
#platform files or do other switching
set(UBNT_PLATFORM_NAME "gen4l")
set(UBNT_PLATFORM_CPU "s5l")

#definitions
add_definitions(-DGEN4L)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DUBNT_USE_BACKTRACE)
add_definitions(-DCONFIG_AMBARELLA_MAX_CHANNEL_NUM=2)
add_definitions(-DCONFIG_ARCH_S5L)

#compile flags
add_compile_options(
    -pipe
    -march=armv8-a
    -mcpu=cortex-a53+crypto
    -mlittle-endian
    -funwind-tables
)

#linker flags
set(EXTRA_LINKER_FLAGS "-Wl,--hash-style=gnu")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_LINKER_FLAGS}")
set(
    CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} ${EXTRA_LINKER_FLAGS}"
)
set(
    CMAKE_MODULE_LINKER_FLAGS
    "${CMAKE_MODULE_LINKER_FLAGS} ${EXTRA_LINKER_FLAGS}"
)
