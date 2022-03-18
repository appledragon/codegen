include(_check_os)

function(_add_compile_definitions)
    set(options "")
    set(oneValueArg TARGET_NAME)
    set(multiValueArgs
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
	PRIVATE_DEFINITIONS_EXTRA
	PUBLIC_DEFINITIONS_EXTRA
	INTERFACE_DEFINITIONS_EXTRA
    )

    cmake_parse_arguments(_add_compile_definitions "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    _check_os()

    target_compile_definitions(${_add_compile_definitions_TARGET_NAME} 
        PRIVATE
            ${_add_compile_definitions_DEFINITIONS}
            ${_add_compile_definitions_PRIVATE_DEFINITIONS}

	    ${_add_compile_definitions_PRIVATE_DEFINITIONS_EXTRA}

            # Windows
            $<$<BOOL:${TARGET_WINDOWS}>:${_add_compile_definitions_PRIVATE_DEFINITIONS_WINDOWS};${_add_compile_definitions_DEFINITIONS_WINDOWS}>

            # Linux
            $<$<BOOL:${TARGET_LINUX}>:${_add_compile_definitions_DEFINITIONS_LINUX};${_add_compile_definitions_PRIVATE_DEFINITIONS_LINUX}>

            # Android
            $<$<BOOL:${TARGET_ANDROID}>:${_add_compile_definitions_DEFINITIONS_ANDROID};${_add_compile_definitions_PRIVATE_DEFINITIONS_ANDROID}>

            # macOS
            $<$<BOOL:${TARGET_MACOSX}>:${_add_compile_definitions_DEFINITIONS_MACOSX};${_add_compile_definitions_PRIVATE_DEFINITIONS_MACOSX};${_add_compile_definitions_DEFINITIONS_APPLE};${_add_compile_definitions_PRIVATE_DEFINITIONS_APPLE};>

            # iOS
            $<$<BOOL:${TARGET_IOS}>:${_add_compile_definitions_DEFINITIONS_IOS};${_add_compile_definitions_PRIVATE_DEFINITIONS_IOS};${_add_compile_definitions_DEFINITIONS_APPLE};${_add_compile_definitions_PRIVATE_DEFINITIONS_APPLE}>
        PUBLIC
            ${_add_compile_definitions_PUBLIC_DEFINITIONS}
	    ${_add_compile_definitions_PUBLIC_DEFINITIONS_EXTRA}
            $<$<BOOL:${TARGET_WINDOWS}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_WINDOWS}>
            $<$<BOOL:${TARGET_LINUX}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_LINUX}>
            $<$<BOOL:${TARGET_MACOSX}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_MACOSX}>
            $<$<BOOL:${TARGET_IOS}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_IOS}>

            # Windows
            $<$<BOOL:${TARGET_WINDOWS}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_WINDOWS}>

            # Linux
            $<$<BOOL:${TARGET_LINUX}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_LINUX}>

            # Android
            $<$<BOOL:${TARGET_ANDROID}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_ANDROID}>

            # macOS
            $<$<BOOL:${TARGET_MACOSX}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_MACOSX}>

            # iOS
            $<$<BOOL:${TARGET_IOS}>:${_add_compile_definitions_PUBLIC_DEFINITIONS_IOS}>
        INTERFACE
       	    ${_add_compile_definitions_INTERFACE_DEFINITIONS_EXTRA}
    )
endfunction(_add_compile_definitions)
