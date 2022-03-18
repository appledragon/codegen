include(_set_ignore_warning_list)
include(_check_os)
include(_create_target)
include(_gen_source_file_list)
include(_create_alias)
include(_create_ide_filter)
include(_add_compile_definitions)
include(_add_link_libraries)
include(_add_include_directories)
include(_set_cxx_standard)
include(_set_code_position_independent)
include(_install_target)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0004 NEW)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    # string(REGEX REPLACE " /Zi" " " CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    # string(REGEX REPLACE " /Zi" " " CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REGEX REPLACE " /W[0-9]" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    # string(REGEX REPLACE " /Zi" " " CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    # string(REGEX REPLACE " /Zi" " " CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    string(REGEX REPLACE " /W[0-9]" " " CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
endif()

function(_build_target)
    set(options SHARED STATIC EXECUTABLE OBJECT WIN32 FRAMEWORK FRAMEWORK_MACOSX FRAMEWORK_IOS BUNDLE BUNDLE_IOS BUNDLE_MACOSX EXCLUDE_FROM_INSTALL INCLUDE_TO_INSTALL)
    set(oneValueArg NAME ALIAS CXX_STANDARD PRECOMPILED_HEADER CHARSET IDE_FOLDER INSTALL_DIRECTORY OUTPUT_DIRECTORY
                    PLIST_FILE_IOS BUNDLE_IDENTIFIER_IOS PLIST_FILE_MACOSX BUNDLE_IDENTIFIER_MACOSX NODEFAULTLIBS
                    NOENTRY ENTRY_POINT)
    set(multiValueArgs
        FILES
        FILES_WINDOWS
        FILES_LINUX
        FILES_ANDROID
        FILES_MACOSX
        FILES_IOS
        FILES_APPLE
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
        DEFINITIONS
        PRIVATE_DEFINITIONS
        PUBLIC_DEFINITIONS
        DEFINITIONS_WINDOWS
        PRIVATE_DEFINITIONS_WINDOWS
        PUBLIC_DEFINITIONS_WINDOWS
        DEFINITIONS_ANDROID
        PRIVATE_DEFINITIONS_ANDROID
        PUBLIC_DEFINITIONS_ANDROID
        DEFINITIONS_LINUX
        PRIVATE_DEFINITIONS_LINUX
        PUBLIC_DEFINITIONS_LINUX
        DEFINITIONS_IOS
        PRIVATE_DEFINITIONS_IOS
        PUBLIC_DEFINITIONS_IOS
        DEFINITIONS_MACOSX
        PRIVATE_DEFINITIONS_MACOSX
        PUBLIC_DEFINITIONS_MACOSX
        DEFINITIONS_APPLE
        PRIVATE_DEFINITIONS_APPLE
        PUBLIC_DEFINITIONS_APPLE
        IGNORE_WARNINGS_MSVC
        IGNORE_WARNINGS_GCC
        IGNORE_WARNINGS_CLANG
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
        PROVIDES
        EXCLUDE_FROM_PRECOMPILED_HEADER
    )

    cmake_parse_arguments(_build_target "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    if ("${_build_target_NAME}" STREQUAL "")
        message(FATAL_ERROR "Please provide NAME")
    endif()

    _check_os()

    set(CMAKE_Swift_LANGUAGE_VERSION 4.0)

    #
    # Create a list of files to compile
    #

    _gen_source_file_list(VAR SOURCE_FILES
        FILES         ${_build_target_FILES}
        FILES_WINDOWS ${_build_target_FILES_WINDOWS}
        FILES_LINUX   ${_build_target_FILES_LINUX}
        FILES_ANDROID ${_build_target_FILES_ANDROID}
        FILES_MACOSX  ${_build_target_FILES_MACOSX} ${_build_target_FILES_APPLE}
        FILES_IOS     ${_build_target_FILES_IOS}    ${_build_target_FILES_APPLE}
    )

    #
    # Remove files excluded from precompiled build
    #
    list(LENGTH _build_target_EXCLUDE_FROM_PRECOMPILED_HEADER _NUMBER_OF_EXCLUDED_FILES)
    if (${_NUMBER_OF_EXCLUDED_FILES} GREATER 0)
        list(REMOVE_ITEM SOURCE_FILES ${_build_target_EXCLUDE_FROM_PRECOMPILED_HEADER})
    endif()

    #
    # Create a target
    #
    _create_target(
        NAME         "${_build_target_NAME}"
        SHARED       "${_build_target_SHARED}"
        OBJECT       "${_build_target_OBJECT}"
        STATIC       "${_build_target_STATIC}"
        FRAMEWORK    "${_build_target_FRAMEWORK}"
        FRAMEWORK_MACOSX "${_build_target_FRAMEWORK_MACOSX}"
        FRAMEWORK_IOS    "${_build_target_FRAMEWORK_IOS}"
        BUNDLE           "${_build_target_BUNDLE}"
        BUNDLE_IOS       "${_build_target_BUNDLE_IOS}"
        BUNDLE_MACOSX    "${_build_target_BUNDLE_MACOSX}"
        EXECUTABLE   "${_build_target_EXECUTABLE}"
        WIN32APP     "${_build_target_WIN32}"
        PLIST_FILE_IOS            "${_build_target_PLIST_FILE_IOS}"
        BUNDLE_IDENTIFIER_IOS     "${_build_target_BUNDLE_IDENTIFIER_IOS}"
        PLIST_FILE_MACOSX         "${_build_target_PLIST_FILE_MACOSX}"
        BUNDLE_IDENTIFIER_MACOSX  "${_build_target_BUNDLE_IDENTIFIER_MACOSX}"
        FILES "${SOURCE_FILES}"
        )

    #
    # Set C++ Standard
    #
    _set_cxx_standard(
	 TARGET_NAME     ${_build_target_NAME}
         CXX_STANDARD    ${_build_target_CXX_STANDARD}
         CXX_EXTENSIONS OFF
    )

    
    #
    # Compile with precompiled heades if needed
    # 
    # Linux precompiled headers are tricky to implements with CMake. Disable for the moment
    # 
    if (NOT "${_build_target_PRECOMPILED_HEADER}" STREQUAL "" AND NOT TARGET_LINUX)
        add_precompiled_header(${_build_target_NAME}
            ${_build_target_PRECOMPILED_HEADER}
        )
    endif()

    #
    # Add files excluded from precompiled headers
    # todo: fixme: add only removed files
    set_property(TARGET ${_build_target_NAME}
        APPEND
        PROPERTY
        SOURCES ${_build_target_EXCLUDE_FROM_PRECOMPILED_HEADER}
    )

    _create_alias(
        ALIAS_NAME  ${_build_target_ALIAS}
        TARGET_NAME ${_build_target_NAME}
    )


    _create_ide_filter(
        FILTER_NAME ${_build_target_IDE_FOLDER}
        TARGET_NAME ${_build_target_NAME}
    )

    #
    # Set a character set (Windows Only)
    #
    if(TARGET_WINDOWS)
        set(CHARSET "UNICODE;_UNICODE")
        string(TOLOWER "${_build_target_CHARSET}" _build_target_CHARSET)
        if ("${_build_target_CHARSET}" STREQUAL "multibyte")
            set(CHARSET _MBCS)
        endif()
        set(CHARSET $<$<C_COMPILER_ID:MSVC>:${CHARSET}>;$<$<CXX_COMPILER_ID:MSVC>:${CHARSET}>)
    endif()

    #
    # Add definitionss to the target
    #

    _add_compile_definitions(
    TARGET_NAME                 ${_build_target_NAME}
        DEFINITIONS                 ${_build_target_DEFINITIONS}
        PRIVATE_DEFINITIONS         ${_build_target_PRIVATE_DEFINITIONS} $<$<AND:$<NOT:$<BOOL:${TARGET_ANDROID}>>,$<CONFIG:Debug>>:DEBUG;_DEBUG>
        PUBLIC_DEFINITIONS          ${_build_target_PUBLIC_DEFINITIONS}
        DEFINITIONS_WINDOWS         ${_build_target_DEFINITIONS_WINDOWS}
        PRIVATE_DEFINITIONS_WINDOWS ${_build_target_PRIVATE_DEFINITIONS_WINDOWS}
        PUBLIC_DEFINITIONS_WINDOWS  ${_build_target_PUBLIC_DEFINITIONS_WINDOWS}
        DEFINITIONS_ANDROID         ${_build_target_DEFINITIONS_ANDROID}
        PRIVATE_DEFINITIONS_ANDROID ${_build_target_PRIVATE_DEFINITIONS_ANDROID}
        PUBLIC_DEFINITIONS_ANDROID  ${_build_target_PUBLIC_DEFINITIONS_ANDROID}
        DEFINITIONS_IOS             ${_build_target_DEFINITIONS_IOS}
        PRIVATE_DEFINITIONS_IOS     ${_build_target_PRIVATE_DEFINITIONS_IOS}
        PUBLIC_DEFINITIONS_IOS      ${_build_target_PUBLIC_DEFINITIONS_IOS}
        DEFINITIONS_LINUX           ${_build_target_DEFINITIONS_LINUX}
        PRIVATE_DEFINITIONS_LINUX   ${_build_target_PRIVATE_DEFINITIONS_LINUX}
        PUBLIC_DEFINITIONS_LINUX    ${_build_target_PUBLIC_DEFINITIONS_LINUX}
        DEFINITIONS_MACOSX          ${_build_target_DEFINITIONS_MACOSX}
        PRIVATE_DEFINITIONS_MACOSX  ${_build_target_PRIVATE_DEFINITIONS_MACOSX}
        PUBLIC_DEFINITIONS_MACOSX   ${_build_target_PUBLIC_DEFINITIONS_MACOSX}
        DEFINITIONS_APPLE           ${_build_target_DEFINITIONS_APPLE}
        PRIVATE_DEFINITIONS_APPLE   ${_build_target_PRIVATE_DEFINITIONS_APPLE}
        PUBLIC_DEFINITIONS_APPLE    ${_build_target_PUBLIC_DEFINITIONS_APPLE}
        PRIVATE_DEFINITIONS_EXTRA   ${CHARSET}
    )

    #
    # Set ignored warnings
    #
    _set_ignore_warning_list(
        OUTPUT IGNORE_WARNINGS
        COMPILER MSVC
        WARNINGS
        4244 4251 4267 4996 ${_build_target_IGNORE_WARNINGS_MSVC}
    )



    target_compile_options(${_build_target_NAME}
        PRIVATE
            #$<$<CXX_COMPILER_ID:MSVC>:$<IF:$<BOOL:${FAST_BUILD_ENABLED}>, /Z7, /Zi>>
            $<$<CXX_COMPILER_ID:MSVC>: /MP /bigobj /EHsc /F2000000 /W3 >
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<BOOL:${_build_target_NODEFAULTLIBS}>>>: /guard:cf >
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:${_build_target_NODEFAULTLIBS}>,$<CONFIG:Release>>: /Zl >
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>: /Od /MDd>
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>: $<IF:$<BOOL:${FAST_BUILD_ENABLED}>, /Od, /O2> /Oi /MD /Os /Gy /Qpar >

            $<$<C_COMPILER_ID:MSVC>: /MP /bigobj /EHsc /F2000000 >
            $<$<AND:$<C_COMPILER_ID:MSVC>,$<NOT:$<BOOL:${_build_target_NODEFAULTLIBS}>>>: /guard:cf >
            $<$<AND:$<C_COMPILER_ID:MSVC>,$<BOOL:${_build_target_NODEFAULTLIBS}>,$<CONFIG:Release>>: /Zl >
            $<$<AND:$<C_COMPILER_ID:MSVC>,$<CONFIG:Debug>>: /Od /MDd>
            $<$<AND:$<C_COMPILER_ID:MSVC>,$<CONFIG:Release>>: $<IF:$<BOOL:${FAST_BUILD_ENABLED}>, /Od, /O2> /Oi /MD /Os /Gy /Qpar>

            ${IGNORE_WARNINGS}
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${TARGET_IOS}>>:-Wno-reorder -Wno-unused-local-typedef -Wno-missing-braces -Wno-implicit-function-declaration>
            $<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:${TARGET_IOS}>>:-Wno-reorder -Wno-unused-local-typedef -Wno-missing-braces -Wno-implicit-function-declaration>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<AND:$<BOOL:${TARGET_MACOSX}>,$<CXX_COMPILER_ID:Clang>>>:-fobjc-arc>
            $<$<AND:$<COMPILE_LANGUAGE:C>,$<AND:$<BOOL:${TARGET_MACOSX}>,$<CXX_COMPILER_ID:Clang>>>:-fobjc-arc>
    )

    # Enable Linking optimization for MSVC
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        set_property(TARGET ${_build_target_NAME}  
            APPEND_STRING PROPERTY  LINK_FLAGS_RELEASE " /OPT:REF /OPT:ICF "
        )
    endif()

    # Enable Extra linking flags for MSVC
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        set_property(TARGET ${_build_target_NAME}  
            APPEND_STRING PROPERTY  LINK_FLAGS_RELEASE " /DEBUG /INCREMENTAL:NO /ignore:4099 "
        )

        set_property(TARGET ${_build_target_NAME}  
            APPEND_STRING PROPERTY  LINK_FLAGS_DEBUG " /DEBUG /INCREMENTAL /ignore:4099 "
        )
    endif()

    if ((${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC) AND _build_target_ENTRY_POINT)
        set_property(TARGET ${_build_target_NAME}  
            APPEND_STRING PROPERTY  LINK_FLAGS_RELEASE " /ENTRY:${_build_target_ENTRY_POINT}"
        )
    endif()

    if ((${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC) AND _build_target_NOENTRY)
        set_property(TARGET ${_build_target_NAME}  
            APPEND_STRING PROPERTY  LINK_FLAGS_RELEASE " /NOENTRY"
        )
    endif()

    if ((${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC) AND _build_target_NODEFAULTLIBS)
        set_property(TARGET ${_build_target_NAME}  
            APPEND_STRING PROPERTY  LINK_FLAGS_RELEASE " /NODEFAULTLIB"
        )

        set_property(TARGET ${_build_target_NAME}  
            APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:/GS->
        )
    endif()

    # Set extra properties
    set_target_properties(
        ${_build_target_NAME}
        PROPERTIES
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "$<IF:$<CONFIG:Debug>,dwarf,dwarf-with-dsym>"
	    XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "$<IF:$<CONFIG:Debug>,NO,YES>"
        #XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "$<IF:$<CONFIG:Debug>,NO,YES>"
        #XCODE_ATTRIBUTE_STRIP_STYLE "$<IF:$<CONFIG:Debug>,debugging,non-global>"
    )


    _add_link_libraries(
        TARGET_NAME                ${_build_target_NAME}
        DEPENDS_ON_PRIVATE         ${_build_target_DEPENDS_ON_PRIVATE}
        DEPENDS_ON_PUBLIC          ${_build_target_DEPENDS_ON_PUBLIC}
        DEPENDS_ON                 ${_build_target_DEPENDS_ON}
        DEPENDS_ON_PRIVATE_WINDOWS ${_build_target_DEPENDS_ON_PRIVATE_WINDOWS}
        DEPENDS_ON_PUBLIC_WINDOWS  ${_build_target_DEPENDS_ON_PUBLIC_WINDOWS}
        DEPENDS_ON_WINDOWS         ${_build_target_DEPENDS_ON_WINDOWS}
        DEPENDS_ON_PRIVATE_UNIX    ${_build_target_DEPENDS_ON_PRIVATE_UNIX}
        DEPENDS_ON_PUBLIC_UNIX     ${_build_target_DEPENDS_ON_PUBLIC_UNIX}
        DEPENDS_ON_UNIX            ${_build_target_DEPENDS_ON_UNIX}
        DEPENDS_ON_PRIVATE_LINUX   ${_build_target_DEPENDS_ON_PRIVATE_LINUX}
        DEPENDS_ON_PUBLIC_LINUX    ${_build_target_DEPENDS_ON_PUBLIC_LINUX}
        DEPENDS_ON_LINUX           ${_build_target_DEPENDS_ON_LINUX}
        DEPENDS_ON_PRIVATE_ANDROID ${_build_target_DEPENDS_ON_PRIVATE_ANDROID}
        DEPENDS_ON_PUBLIC_ANDROID  ${_build_target_DEPENDS_ON_PUBLIC_ANDROID}
        DEPENDS_ON_ANDROID         ${_build_target_DEPENDS_ON_ANDROID}
        DEPENDS_ON_PRIVATE_IOS     ${_build_target_DEPENDS_ON_PRIVATE_IOS}
        DEPENDS_ON_PUBLIC_IOS      ${_build_target_DEPENDS_ON_PUBLIC_IOS}
        DEPENDS_ON_IOS             ${_build_target_DEPENDS_ON_IOS}
        DEPENDS_ON_PRIVATE_MACOSX  ${_build_target_DEPENDS_ON_PRIVATE_MACOSX}
        DEPENDS_ON_PUBLIC_MACOSX   ${_build_target_DEPENDS_ON_PUBLIC_MACOSX}
        DEPENDS_ON_MACOSX          ${_build_target_DEPENDS_ON_MACOSX}
        DEPENDS_ON_PRIVATE_APPLE   ${_build_target_DEPENDS_ON_PRIVATE_APPLE}
        DEPENDS_ON_PUBLIC_APPLE    ${_build_target_DEPENDS_ON_PUBLIC_APPLE}
        DEPENDS_ON_APPLE           ${_build_target_DEPENDS_ON_APPLE}
        PROVIDES                   ${_build_target_PROVIDES}
    )

    _add_include_directories(
	TARGET_NAME                 ${_build_target_NAME}
	PRIVATE_HEADERS_DIR         ${_build_target_PRIVATE_HEADERS_DIR}
        PRIVATE_HEADERS_DIR_WINDOWS ${_build_target_PRIVATE_HEADERS_DIR_WINDOWS}
        PRIVATE_HEADERS_DIR_LINUX   ${_build_target_PRIVATE_HEADERS_DIR_LINUX}
        PRIVATE_HEADERS_DIR_ANDROID ${_build_target_PRIVATE_HEADERS_DIR_ANDROID}
        PRIVATE_HEADERS_DIR_IOS     ${_build_target_PRIVATE_HEADERS_DIR_IOS}
        PRIVATE_HEADERS_DIR_APPLE   ${_build_target_PRIVATE_HEADERS_DIR_APPLE}
        PRIVATE_HEADERS_DIR_MACOSX  ${_build_target_PRIVATE_HEADERS_DIR_MACOSX}
        PUBLIC_HEADERS_DIR          ${_build_target_PUBLIC_HEADERS_DIR}
        PUBLIC_HEADERS_DIR_WINDOWS  ${_build_target_PUBLIC_HEADERS_DIR_WINDOWS}
        PUBLIC_HEADERS_DIR_LINUX    ${_build_target_PUBLIC_HEADERS_DIR_LINUX}
        PUBLIC_HEADERS_DIR_ANDROID  ${_build_target_PUBLIC_HEADERS_DIR_ANDROID}
        PUBLIC_HEADERS_DIR_IOS      ${_build_target_PUBLIC_HEADERS_DIR_IOS}
        PUBLIC_HEADERS_DIR_APPLE    ${_build_target_PUBLIC_HEADERS_DIR_APPLE}
        PUBLIC_HEADERS_DIR_MACOSX   ${_build_target_PUBLIC_HEADERS_DIR_MACOSX}
    )
    
    # Keep output folder the same as install folder
    get_target_property(CURRENT_RUNTIME_OUTPUT_DIR   ${_build_target_NAME} RUNTIME_OUTPUT_DIRECTORY)
    if(NOT "${CURRENT_RUNTIME_OUTPUT_DIR}" STREQUAL "CURRENT_RUNTIME_OUTPUT_DIR-NOTFOUND")
        set_target_properties( ${_build_target_NAME}
            PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CURRENT_RUNTIME_OUTPUT_DIR}/Debug/${_build_target_OUTPUT_DIRECTORY}"
				RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CURRENT_RUNTIME_OUTPUT_DIR}/Release/${_build_target_OUTPUT_DIRECTORY}"
        )
    endif()

    
    # Select compiler standard
    

    _set_code_position_independent(
        TARGET_NAME ${_build_target_NAME}
        VALUE ON
    )

    set(INSTALL_DIRECTORY .)
    if(_build_target_INSTALL_DIRECTORY)
        set(INSTALL_DIRECTORY ${_build_target_INSTALL_DIRECTORY})
    endif()
    
    # Create CMake export + install artefacts
    _install_target(
        TARGET_NAME          ${_build_target_NAME}
        EXCLUDE_FROM_INSTALL ${_build_target_EXCLUDE_FROM_INSTALL}
        INCLUDE_TO_INSTALL   ${_build_target_INCLUDE_TO_INSTALL}
        DESTINATION ${INSTALL_DIRECTORY}
    )

endfunction()
