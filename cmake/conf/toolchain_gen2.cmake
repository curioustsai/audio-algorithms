#gen2 toolchain descriptor

#the brand of the OS
set(CMAKE_SYSTEM_NAME Linux)

#this one is required by cmake, but kinda obscure
set(CMAKE_SYSTEM_VERSION 1)

#where the C and C++ compilers are located
set(
    CMAKE_C_COMPILER
    ${UBNT_FW_CACHE}/gen2/toolchains/bin/arm-openwrt-linux-uclibcgnueabi-gcc
)
set(
    CMAKE_CXX_COMPILER
    ${UBNT_FW_CACHE}/gen2/toolchains/bin/arm-openwrt-linux-uclibcgnueabi-g++
)

#where is the sysroot
set(
    CMAKE_FIND_ROOT_PATH
    ${UBNT_FW_CACHE}/gen2/barebones/staging_dir/target-arm-openwrt-linux-uclibcgnueabi
)

#never search for utility apps in the target
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

#always search for libs and headers in target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set the UBNT platform name, used across the cmake builder to include the necessary
#platform files or do other switching
set(UBNT_PLATFORM_NAME "gen2")
set(UBNT_PLATFORM_CPU "a5s")

#definitions
add_definitions(-DGEN2)
add_definitions(-D_FILE_OFFSET_BITS=64)

#compile flags
add_compile_options(
    -march=armv6k
    -mtune=arm1136j-s
    -msoft-float
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
