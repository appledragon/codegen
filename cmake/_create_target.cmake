include(_check_os)

function(_create_target)
    set(options "")
    set(oneValueArg NAME SHARED STATIC OBJECT MODULE EXECUTABLE WIN32APP FRAMEWORK FRAMEWORK_IOS FRAMEWORK_MACOSX BUNDLE BUNDLE_IOS BUNDLE_MACOSX
                    PLIST_FILE_IOS BUNDLE_IDENTIFIER_IOS PLIST_FILE_MACOSX BUNDLE_IDENTIFIER_MACOSX CFBUNDLE_IOS
                    CFBUNDLE_MACOSX CFBUNDLE_TESTEE_IOS CFBUNDLE_TESTEE_MACOSX
       )
    set(multiValueArgs FILES)

    cmake_parse_arguments(_create_target "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        _check_os()

    if (_create_target_CFBUNDLE_IOS AND TARGET_IOS)
        message(STATUS "Build ${_create_target_NAME} as executable (CFBundle)")
        BuildXCTestBundle(
            NAME ${_create_target_NAME}
            TESTEE ${_create_target_CFBUNDLE_TESTEE_IOS}
            FILES_XCTEST "${_create_target_FILES}"
        )
    elseif (_create_target_CFBUNDLE_MACOSX AND TARGET_MACOSX)
        message(STATUS "Build ${_create_target_NAME} as executable (CFBundle)")
        BuildXCTestBundle(
            NAME ${_create_target_NAME}
            TESTEE ${_create_target_CFBUNDLE_TESTEE_MACOSX}
            FILES_XCTEST "${_create_target_FILES}"
        )
    elseif ((_create_target_BUNDLE_IOS OR _create_target_BUNDLE) AND TARGET_IOS)
        message(STATUS "Build ${_create_target_NAME} as executable (Bundle)")
        add_executable(${_create_target_NAME} MACOSX_BUNDLE
            "${_create_target_FILES}"
        )
        set_target_properties(${_create_target_NAME}
            PROPERTIES
                MACOSX_BUNDLE_GUI_IDENTIFIER   ${_create_target_BUNDLE_IDENTIFIER_IOS}
        )

        if (_create_target_PLIST_FILE_IOS)
            set_target_properties(${_create_target_NAME}
                PROPERTIES
                    MACOSX_BUNDLE_INFO_PLIST       ${_create_target_PLIST_FILE_IOS}
            )
        endif()
    elseif ((_create_target_BUNDLE_MACOSX OR _create_target_BUNDLE) AND TARGET_MACOSX)
        message(STATUS "Build ${_create_target_NAME} as executable (Bundle)")
        add_executable(${_create_target_NAME} MACOSX_BUNDLE
            "${_create_target_FILES}"
        )
        set_target_properties(${_create_target_NAME}
            PROPERTIES
                MACOSX_BUNDLE_GUI_IDENTIFIER   ${_create_target_BUNDLE_IDENTIFIER_MACOSX}
        )
        if (_create_target_PLIST_FILE_MACOSX)
            set_target_properties(${_create_target_NAME}
                PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST       ${_create_target_PLIST_FILE_MACOSX}
            )
        endif()
    elseif ((_create_target_FRAMEWORK_IOS OR _create_target_FRAMEWORK) AND TARGET_IOS AND _create_target_SHARED)
        message(STATUS "Build ${_create_target_NAME} as shared library [Framework]")
        add_library(${_create_target_NAME}
	    SHARED
	    "${_create_target_FILES}"
        )

        if (_create_target_BUNDLE_IDENTIFIER_IOS)
            message(STATUS "set bundle identifier ${_create_target_BUNDLE_IDENTIFIER_IOS} to shared library [Framework]")
            set_target_properties(${_create_target_NAME}
                PROPERTIES
                    FRAMEWORK TRUE
                    MACOSX_FRAMEWORK_IDENTIFIER ${_create_target_BUNDLE_IDENTIFIER_IOS}
            )
        else()
            set_target_properties(${_create_target_NAME}
            PROPERTIES
                FRAMEWORK TRUE
            )
        endif()
        
        
    elseif ((_create_target_FRAMEWORK_MACOSX OR _create_target_FRAMEWORK) AND TARGET_MACOSX AND _create_target_SHARED)
        message(STATUS "Build ${_create_target_NAME} as shared library [Framework]")
        add_library(${_create_target_NAME}
	    SHARED
	    "${_create_target_FILES}"
        )
        
        set_target_properties(${_create_target_NAME}
            PROPERTIES
                FRAMEWORK TRUE
       )
    elseif ((_create_target_FRAMEWORK_IOS OR _create_target_FRAMEWORK) AND TARGET_IOS AND _create_target_STATIC)
        message(STATUS "Build ${_create_target_NAME} as static library [Framework]")
        add_library(${_create_target_NAME}
		STATIC
            "${_create_target_FILES}"
        )

        set_target_properties(${_create_target_NAME}
            PROPERTIES
                FRAMEWORK TRUE
       )
    elseif ((_create_target_FRAMEWORK_MACOSX OR _create_target_FRAMEWORK) AND TARGET_MACOSX AND _create_target_STATIC)
        message(STATUS "Build ${_create_target_NAME} as static library [Framework]")
        add_library(${_create_target_NAME}
		STATIC
            "${_create_target_FILES}"
        )

        set_target_properties(${_create_target_NAME}
            PROPERTIES
                FRAMEWORK TRUE
       )
    elseif (${_create_target_MODULE})
        message(STATUS "Build ${_create_target_NAME} as shared library")
        add_library(${_create_target_NAME}
	    MODULE
	    "${_create_target_FILES}"
        )
    elseif (${_create_target_SHARED})
        message(STATUS "Build ${_create_target_NAME} as shared library")
        add_library(${_create_target_NAME}
	    SHARED
	    "${_create_target_FILES}"
        )
    elseif (${_create_target_OBJECT})
	message(STATUS "Build ${_create_target_NAME} as object file")
        add_library(${_create_target_NAME}
            OBJECT
	    "${_create_target_FILES}"
        )
    elseif (${_create_target_STATIC})
	message(STATUS "Build ${_create_target_NAME} as static library (explicit)")
        add_library(${_create_target_NAME}
	    STATIC
	    "${_create_target_FILES}"
        )
    elseif (${_create_target_EXECUTABLE})
       if (WIN32 AND ${_create_target_WIN32APP})
           set(WIN32APP WIN32)
       endif()

       if(WIN32APP)
           set(EXE_TYPE "Windows")
       else()
           set(EXE_TYPE "Console")
       endif()

       message(STATUS "Build ${_create_target_NAME} as executable [${EXE_TYPE}]")
       add_executable(${_create_target_NAME} ${WIN32APP} 
           "${_create_target_FILES}"
       ) 
    else()
        set(TARGET_TYPE STATIC)
        if (DEFAULT_TARGET_TYPE)
            set(TARGET_TYPE ${DEFAULT_TARGET_TYPE})
        endif()
	message(STATUS "Build ${_create_target_NAME} as ${TARGET_TYPE} (implicit)")
        add_library(${_create_target_NAME}
	    ${TARGET_TYPE}
	    "${_create_target_FILES}"
        )
    endif()
endfunction(_create_target)
