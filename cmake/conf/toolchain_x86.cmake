#gen2 toolchain descriptor

#the brand of the OS
set(CMAKE_SYSTEM_NAME Linux)

#this one is required by cmake, but kinda obscure
set(CMAKE_SYSTEM_VERSION 1)

#never search for utility apps in the target
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

#always search for libs and headers in target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set the UBNT platform name, used across the cmake builder to include the necessary
#platform files or do other switching
set(UBNT_PLATFORM_NAME "x86")
set(UBNT_PLATFORM_CPU "x86")

#definitions
add_definitions(-DEMU=1)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DUBNT_X86)
