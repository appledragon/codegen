include(_set_signing_after_build)

function(_install_target)
    set(options "")
    set(oneValueArg TARGET_NAME EXCLUDE_FROM_INSTALL INCLUDE_TO_INSTALL DESTINATION)
    set(multiValueArgs "")

    cmake_parse_arguments(_install_target "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    if (${_install_target_EXCLUDE_FROM_INSTALL} AND ${_install_target_INCLUDE_TO_INSTALL})
        message(ERROR "EXCLUDE_FROM_INSTALL and INCLUDE_TO_INSTALL specified as True. Make your mind.")
    endif()

    get_target_property(
        TARGET_TYPE ${_install_target_TARGET_NAME} TYPE
    )

	# not install automatically
    #if (NOT ${_install_target_EXCLUDE_FROM_INSTALL} AND (${_install_target_INCLUDE_TO_INSTALL} OR (${TARGET_TYPE} STREQUAL "SHARED_LIBRARY") OR (${TARGET_TYPE} STREQUAL "EXECUTABLE")))
    #    _set_signing_after_build(${_install_target_TARGET_NAME})
	#    install(FILES $<TARGET_FILE:${_install_target_TARGET_NAME}>
    #        DESTINATION ${_install_target_DESTINATION}
    #    )
	#
    #    message(STATUS "-- Install ${_install_target_TARGET_NAME} to <prefix>/${_install_target_DESTINATION}")
	#
    #endif()

endfunction(_install_target)
