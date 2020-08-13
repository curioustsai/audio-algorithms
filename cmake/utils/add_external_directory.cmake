function(
    add_external_directory
    DIR_PATH
)
    get_filename_component(COMPONENT_NAME ${DIR_PATH} NAME)
    add_subdirectory(${DIR_PATH} ${CMAKE_BINARY_DIR}/${COMPONENT_NAME})
endfunction()
