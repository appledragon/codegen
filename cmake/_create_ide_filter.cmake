
function(_create_ide_filter)
    set(options "")
    set(oneValueArg FILTER_NAME TARGET_NAME)
    set(multiValueArg)

    cmake_parse_arguments(_create_ide_filter "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    if (NOT "${_create_ide_filter_FILTER_NAME}" STREQUAL "")
	    set_target_properties(${_create_ide_filter_TARGET_NAME}
                PROPERTIES
	            FOLDER "${_create_ide_filter_FILTER_NAME}"
        )
    endif()

endfunction(_create_ide_filter)
