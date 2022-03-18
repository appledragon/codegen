include(_check_os)


function(_gen_source_file_list)
    set(options "")
    set(oneValueArg VAR)
    set(multiValueArg
        FILES
        FILES_WINDOWS
        FILES_LINUX
        FILES_ANDROID
        FILES_MACOSX
        FILES_IOS
        FILES_APPLE
    )

    cmake_parse_arguments(_gen_source_file_list "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    _check_os()

    set(${_gen_source_file_list_VAR}

	    ${_gen_source_file_list_FILES}

            # Windows
	    $<$<BOOL:${TARGET_WINDOWS}>:${_gen_source_file_list_FILES_WINDOWS}>

            # Linux
	    $<$<BOOL:${TARGET_LINUX}>:${_gen_source_file_list_FILES_LINUX}>

            # Android
	    $<$<BOOL:${TARGET_ANDROID}>:${_gen_source_file_list_FILES_ANDROID}>

            # macOS
	    $<$<BOOL:${TARGET_MACOSX}>:${_gen_source_file_list_FILES_MACOSX} ${_gen_source_file_list_FILES_APPLE}>

            # iOS
	    $<$<BOOL:${TARGET_IOS}>:${_gen_source_file_list_FILES_IOS} ${_gen_source_file_list_FILES_APPLE}>

	    PARENT_SCOPE
    )

endfunction(_gen_source_file_list)
