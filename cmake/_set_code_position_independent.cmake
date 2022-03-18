
function(_set_code_position_independent)
    set(options "")
    set(oneValueArg TARGET_NAME VALUE)
    set(multiValueArgs "")

    cmake_parse_arguments(_set_code_position_independent "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    set_target_properties(${_set_code_position_independent_TARGET_NAME} PROPERTIES
	    POSITION_INDEPENDENT_CODE ${_set_code_position_independent_VALUE}
    )
endfunction(_set_code_position_independent)
