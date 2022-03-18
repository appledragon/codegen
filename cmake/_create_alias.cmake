
function(_create_alias)
    set(options "")
    set(oneValueArg ALIAS_NAME TARGET_NAME)
    set(multiValueArgs ""
    )

    cmake_parse_arguments(_create_alias "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    if (NOT "${_create_alias_ALIAS_NAME}" STREQUAL "")
	add_library(${_create_alias_ALIAS_NAME} ALIAS ${_create_alias_TARGET_NAME})
	message("Alias: ${_create_alias_ALIAS_NAME} ->  ${_create_alias_TARGET_NAME}")
    endif()
endfunction(_create_alias)
