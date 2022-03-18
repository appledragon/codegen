include(_check_os)

macro(GetAbsolutePath VAR paths base)
    if (${paths})
        foreach(path ${${paths}})
            get_filename_component(ABSOLUTE_PATH ${path} ABSOLUTE BASE_DIR ${base})
            list(APPEND ${VAR} "${ABSOLUTE_PATH}")
        endforeach()
    endif()
endmacro(GetAbsolutePath)

function(_add_include_directories)
    set(options "")
    set(oneValueArg TARGET_NAME)
    set(multiValueArgs
        PRIVATE_HEADERS_DIR
        PRIVATE_HEADERS_DIR_WINDOWS
        PRIVATE_HEADERS_DIR_LINUX
        PRIVATE_HEADERS_DIR_ANDROID
        PRIVATE_HEADERS_DIR_IOS
        PRIVATE_HEADERS_DIR_APPLE
        PRIVATE_HEADERS_DIR_MACOSX
        PUBLIC_HEADERS_DIR
        PUBLIC_HEADERS_DIR_WINDOWS
        PUBLIC_HEADERS_DIR_LINUX
        PUBLIC_HEADERS_DIR_ANDROID
        PUBLIC_HEADERS_DIR_IOS
        PUBLIC_HEADERS_DIR_APPLE
        PUBLIC_HEADERS_DIR_MACOSX
    )

    cmake_parse_arguments(_add_include_directories "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    _check_os()

    GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE         _add_include_directories_PUBLIC_HEADERS_DIR         ${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR})
    if (TARGET_WINDOWS)
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_WINDOWS})
    elseif (TARGET_LINUX)
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_LINUX   ${CMAKE_CURRENT_SOURCE_DIR})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_LINUX})
    elseif (TARGET_ANDROID)
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_ANDROID ${CMAKE_CURRENT_SOURCE_DIR})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_ANDROID})
    elseif (TARGET_IOS)
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_IOS     ${CMAKE_CURRENT_SOURCE_DIR})
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_APPLE   ${CMAKE_CURRENT_SOURCE_DIR})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_IOS})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_APPLE})
    elseif (TARGET_MACOSX)
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_MACOSX  ${CMAKE_CURRENT_SOURCE_DIR})
        GetAbsolutePath(_PUBLIC_HEADERS_BUILD_INTERFACE     _add_include_directories_PUBLIC_HEADERS_DIR_APPLE   ${CMAKE_CURRENT_SOURCE_DIR})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_MACOSX})
        list(APPEND _PUBLIC_HEADERS_INSTALL_INTERFACE ${_add_include_directories_PUBLIC_HEADERS_DIR_APPLE})
    endif()
    
    GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE         _add_include_directories_PRIVATE_HEADERS_DIR         ${CMAKE_CURRENT_SOURCE_DIR})
    if (TARGET_WINDOWS)
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR})
    elseif (TARGET_LINUX)
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_LINUX   ${CMAKE_CURRENT_SOURCE_DIR})
    elseif (TARGET_ANDROID)
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_ANDROID ${CMAKE_CURRENT_SOURCE_DIR})
    elseif (TARGET_IOS)
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_IOS     ${CMAKE_CURRENT_SOURCE_DIR})
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_APPLE   ${CMAKE_CURRENT_SOURCE_DIR})
    elseif (TARGET_MACOSX)
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_MACOSX  ${CMAKE_CURRENT_SOURCE_DIR})
        GetAbsolutePath(_PRIVATE_HEADERS_BUILD_INTERFACE     _add_include_directories_PRIVATE_HEADERS_DIR_APPLE   ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    target_include_directories(${_add_include_directories_TARGET_NAME} 
        PRIVATE
            ${_PRIVATE_HEADERS_BUILD_INTERFACE}
            ${_PUBLIC_HEADERS_BUILD_INTERFACE}
        INTERFACE
            $<BUILD_INTERFACE:${_PUBLIC_HEADERS_BUILD_INTERFACE}>
            $<INSTALL_INTERFACE:${_PUBLIC_HEADERS_INSTALL_INTERFACE}>
    )

endfunction(_add_include_directories)
