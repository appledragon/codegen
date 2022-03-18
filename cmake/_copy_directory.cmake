
include(_create_ide_filter)

function(_copy_directory)
    set(options "")
    set(oneValueArg NAME WORKING_DIRECTORY COMMENT IDE_FOLDER)
    set(multiValueArgs DIRECTORY DESTINATION)

    cmake_parse_arguments(_copy_directory "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})  

    add_custom_target(${_copy_directory_NAME}
        COMMAND            ${CMAKE_COMMAND} -E copy_directory  ${_copy_directory_DIRECTORY} ${_copy_directory_DESTINATION}
        WORKING_DIRECTORY "${_copy_directory_WORKING_DIRECTORY}"
        COMMENT           "${_copy_directory_COMMENT}"
    )

    _create_ide_filter(
        TARGET_NAME "${_copy_directory_NAME}"
        FILTER_NAME "${_copy_directory_IDE_FOLDER}"
    )

endfunction(_copy_directory)
