cmake_policy(PUSH)

include(_check_os)

cmake_policy(SET CMP0003 NEW) # Libraries linked via full path no longer produce linker search paths.
cmake_policy(SET CMP0004 NEW) # Libraries linked may not have leading or trailing whitespace.
cmake_policy(SET CMP0022 NEW) # INTERFACE_LINK_LIBRARIES defines the link interface.
cmake_policy(SET CMP0028 NEW) # Double colon in target name means ALIAS or IMPORTED target

# We do it here because cmake 3.6 doesnt have this function
function(join_strings output glue)
    set(out "")
    list(LENGTH ARGN NSTR)

	if (NSTR GREATER 0)
	    list(GET ARGN 0 out)
		list(REMOVE_AT ARGN 0)

        foreach(str ${ARGN})
	        string(APPEND out "${glue}" "${str}")
	    endforeach()
	endif()

	set(${output} "${out}" PARENT_SCOPE)
endfunction(join_strings)


function(_add_link_libraries)
    set(options "")
    set(oneValueArg TARGET_NAME)
    set(multiValueArgs
        DEPENDS_ON_PRIVATE
        DEPENDS_ON_PUBLIC
        DEPENDS_ON
        DEPENDS_ON_PRIVATE_WINDOWS
        DEPENDS_ON_PUBLIC_WINDOWS
        DEPENDS_ON_WINDOWS
        DEPENDS_ON_PRIVATE_UNIX
        DEPENDS_ON_PUBLIC_UNIX
        DEPENDS_ON_UNIX
        DEPENDS_ON_PRIVATE_LINUX
        DEPENDS_ON_PUBLIC_LINUX
        DEPENDS_ON_LINUX
        DEPENDS_ON_PRIVATE_ANDROID
        DEPENDS_ON_PUBLIC_ANDROID
        DEPENDS_ON_ANDROID
        DEPENDS_ON_PRIVATE_IOS
        DEPENDS_ON_PUBLIC_IOS
        DEPENDS_ON_IOS
        DEPENDS_ON_PRIVATE_MACOSX
        DEPENDS_ON_PUBLIC_MACOSX
        DEPENDS_ON_MACOSX
        DEPENDS_ON_PRIVATE_APPLE
        DEPENDS_ON_PUBLIC_APPLE
        DEPENDS_ON_APPLE
        # This is a workaround for cmake < 3.12
        DEPENDS_ON_OBJECT
        PROVIDES
    )

    cmake_parse_arguments(_add_link_libraries "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    _check_os()


    get_target_property(
        TARGET_TYPE ${_add_link_libraries_TARGET_NAME} TYPE
    )
    
    join_strings(PRIVATE_LIBS ";"
        "${_add_link_libraries_DEPENDS_ON}"
        "${_add_link_libraries_DEPENDS_ON_PRIVATE}"
        "$<$<BOOL:${TARGET_WINDOWS}>:${_add_link_libraries_DEPENDS_ON_WINDOWS};${_add_link_libraries_DEPENDS_ON_PRIVATE_WINDOWS}>"
        "$<$<BOOL:${TARGET_ANDROID}>:${_add_link_libraries_DEPENDS_ON_ANDROID};${_add_link_libraries_DEPENDS_ON_PRIVATE_ANDROID}>"
        "$<$<BOOL:${TARGET_LINUX}>:${_add_link_libraries_DEPENDS_ON_LINUX};${_add_link_libraries_DEPENDS_ON_PRIVATE_LINUX}>"
        "$<$<BOOL:${TARGET_IOS}>:${_add_link_libraries_DEPENDS_ON_IOS};${_add_link_libraries_DEPENDS_ON_PRIVATE_IOS};${_add_link_libraries_DEPENDS_ON_APPLE};${_add_link_libraries_DEPENDS_ON_PRIVATE_APPLE}>"
        "$<$<BOOL:${TARGET_MACOSX}>:${_add_link_libraries_DEPENDS_ON_MACOSX};${_add_link_libraries_DEPENDS_ON_PRIVATE_MACOSX};${_add_link_libraries_DEPENDS_ON_APPLE};${_add_link_libraries_DEPENDS_ON_PRIVATE_APPLE}>"
    )
    
    join_strings(PUBLIC_LIBS ";"
        "${_add_link_libraries_DEPENDS_ON_PUBLIC}"
        "$<$<BOOL:${TARGET_WINDOWS}>:${_add_link_libraries_DEPENDS_ON_PUBLIC_WINDOWS}>"
        "$<$<BOOL:${TARGET_ANDROID}>:${_add_link_libraries_DEPENDS_ON_PUBLIC_ANDROID}>"
        "$<$<BOOL:${TARGET_LINUX}>:${_add_link_libraries_DEPENDS_ON_PUBLIC_LINUX}>"
        "$<$<BOOL:${TARGET_IOS}>:${_add_link_libraries_DEPENDS_ON_PUBLIC_IOS};${_add_link_libraries_DEPENDS_ON_PUBLIC_APPLE}>"
        "$<$<BOOL:${TARGET_MACOSX}>:${_add_link_libraries_DEPENDS_ON_PUBLIC_MACOSX};${_add_link_libraries_DEPENDS_ON_PUBLIC_APPLE}>"
    )

    foreach(object_target IN LISTS _add_link_libraries_DEPENDS_ON_OBJECT)
        list(APPEND OBJECT_SOURCES "$<TARGET_OBJECTS:${object_target}>")
    endforeach()

    set_property(
        TARGET ${_add_link_libraries_TARGET_NAME} 
        APPEND
        PROPERTY
            SOURCES ${OBJECT_SOURCES}
    )


    target_link_libraries(${_add_link_libraries_TARGET_NAME}
        PRIVATE
            "${PRIVATE_LIBS}"
        PUBLIC
             "${PUBLIC_LIBS}"
        INTERFACE
             "${_add_link_libraries_PROVIDES}"
    )
endfunction(_add_link_libraries)

cmake_policy(POP)
