function(_set_ignore_warning_list)
    set(options)
    set(oneValueArg COMPILER OUTPUT)
    set(multiValueArgs
        WARNINGS)

    cmake_parse_arguments(_set_ignore_warning_list "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    set(compiler_ignore "")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	if (${_set_ignore_warning_list_COMPILER} STREQUAL ${CMAKE_CXX_COMPILER_ID})
            foreach(warning ${_set_ignore_warning_list_WARNINGS})
                set(compiler_ignore ${compiler_ignore} /wd${warning})
            endforeach(warning)
        endif()
    endif()

    set(${_set_ignore_warning_list_OUTPUT} ${compiler_ignore} PARENT_SCOPE)

endfunction()
