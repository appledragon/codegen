
function(_set_signing_after_build target)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        if(SIGNCODE)
            find_package(signtool)
            if (signtool_FOUND)
			    separate_arguments(command WINDOWS_COMMAND ${signtool_COMMAND})
                add_custom_command(
                    TARGET ${target}
                    POST_BUILD
                    COMMAND
                        ${command} $<TARGET_FILE:${target}>
                    COMMENT "-- Enable signing for ${target}")
            else()
                message(WARNING "-- Signing required but signtool not found")
            endif()
        else()
            message(STATUS "-- Skip signing for ${target}")
        endif()
    else()
        message(WARNING "-- Signing not supported for ${CMAKE_HOST_SYSTEM_NAME}")
    endif()
endfunction()
