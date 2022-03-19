cmake_minimum_required(VERSION 3.23)

include(_check_os)
_check_os()


add_library(libclang SHARED IMPORTED GLOBAL)

target_include_directories(libclang
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libclang/include/clang
)
if(WIN32)
else()
	set_target_properties(libclang PROPERTIES
			IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../libclang/lib/libclang.dylib"
			IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../libclang/lib/libclang.dylib"
			)
endif()

if (TARGET_MACOSX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "@executable_path;${CMAKE_INSTALL_PREFIX}/lib")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

include(_build_target)
_build_target(
    NAME vcf-codegen
    IDE_FOLDER vcf-codegen
    EXECUTABLE
    IGNORE_WARNINGS_CLANG_IOS
        "no-#warnings"
    FILES
        main.cpp
        ClassInfo.h
        ClassInfo.cpp
        FileGenerator.h
        FileGenerator.cpp
    PRIVATE_HEADERS_DIR
        .
    PUBLIC_HEADERS_DIR
        .
    DEPENDS_ON
      libclang
      jinja2cpp
)

set_target_properties( vcf-codegen
		PROPERTIES
        CXX_STANDARD 20
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${UT_BIN_PATH}"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${UT_BIN_PATH}"
		)

if (TARGET_MACOSX OR TARGET_IOS)
	install(TARGETS vcf-codegen DESTINATION lib)
else()
	install(TARGETS vcf-codegen DESTINATION bin)
endif ()
