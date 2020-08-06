macro(show_include_and_src_files)
    if(DEFINED SHOW_CMAKE_DEBUG_FILES)
        get_directory_property(
            TEMP_${PROJECT_NAME}
            INCLUDE_DIRECTORIES
        )
        message(
            STATUS
                "INCLUDE_DIRECTORIES for project ${PROJECT_NAME}: ${TEMP_${PROJECT_NAME}}"
        )
        message(
            STATUS "CPP_SOURCES for project ${PROJECT_NAME}: ${CPP_SOURCES}"
        )
        message(STATUS "C_SOURCES for project ${PROJECT_NAME}: ${C_SOURCES}")
    endif()
endmacro()
