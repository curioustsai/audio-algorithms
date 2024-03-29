#gen3b toolchain descriptor

#the brand of the OS
set(CMAKE_SYSTEM_NAME Linux)

#this one is required by cmake, but kinda obscure
set(CMAKE_SYSTEM_VERSION 1)

#where the C and C++ compilers are located
set(
    CMAKE_C_COMPILER
    ${UBNT_FW_CACHE}/gen3b/toolchains/bin/arm-openwrt-linux-gcc
)
set(
    CMAKE_CXX_COMPILER
    ${UBNT_FW_CACHE}/gen3b/toolchains/bin/arm-openwrt-linux-g++
)

#where is the sysroot
set(
    CMAKE_FIND_ROOT_PATH
    ${UBNT_FW_CACHE}/gen3b/barebones/staging_dir/target-arm-openwrt-linux-uclibcgnueabi
)

#never search for utility apps in the target
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

#always search for libs and headers in target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set the UBNT platform name, used across the cmake builder to include the necessary
#platform files or do other switching
set(UBNT_PLATFORM_NAME "gen3b")
set(UBNT_PLATFORM_CPU "s2lm")
set(UBNT_PLATFORM_HOST "arm-openwrt-linux")
set(UBNT_COMPILE_OPTION "-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables")
string(REPLACE " " ";" UBNT_COMPILE_OPTION_LIST ${UBNT_COMPILE_OPTION})

#definitions
add_definitions(
    -DGEN3B
    -DGEN3M
)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DUBNT_USE_BACKTRACE)

#compile flags
foreach(OPT ${UBNT_COMPILE_OPTION_LIST})
    add_compile_options(
        ${OPT}
    )
endforeach()


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
