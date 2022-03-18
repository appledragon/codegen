
function(_set_cxx_standard)
    set(options "")
    set(oneValueArg TARGET_NAME CXX_STANDARD CXX_EXTENSIONS)
    set(multiValueArgs "")

    cmake_parse_arguments(_set_cxx_standard "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    set_target_properties(${_set_cxx_standard_TARGET_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS  ${_set_cxx_standard_CXX_EXTENSIONS}
    )
    
    if (DEFINED _set_cxx_standard_CXX_STANDARD)
        set_target_properties(${_set_cxx_standard_TARGET_NAME} PROPERTIES
            CXX_STANDARD ${_set_cxx_standard_CXX_STANDARD}
	    CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS  ${_set_cxx_standard_CXX_EXTENSIONS}
        )
        message("Overload CXX standard for ${_set_cxx_standard_TARGET_NAME}: new value ${_set_cxx_standard_CXX_STANDARD}")
    endif()
endfunction(_set_cxx_standard)
