#Set the name of the project
project(ubnt_logger)

#detect the source files
glob_source_files()

#set the UBNT module name (used inside the logger)
add_definitions(-D_UBNT_MODULE_NAME=${PROJECT_NAME})
add_definitions(-fno-exceptions)

#set the include folder search path
include_directories(AFTER ${SOURCES_ROOT}/${PROJECT_NAME}/include)

#print some debug info about the list of files and include folders
show_include_and_src_files()

#define the executable
add_library(${PROJECT_NAME} ${UBNT_LIBRARIES_TYPE} ${CPP_SOURCES} ${C_SOURCES})

install(
    TARGETS ${PROJECT_NAME}
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
    DESTINATION /lib
)
