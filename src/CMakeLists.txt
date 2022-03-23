cmake_minimum_required(VERSION 3.23)

if (APPLE)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "@executable_path;${CMAKE_INSTALL_PREFIX}/lib")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

add_executable(vcf-codegen         
        main.cpp
        Utils.h
        ClassInfo.h
        ClassInfo.cpp
        FileGenerator.h
        FileGenerator.cpp
        HeaderParser.h
        ClassParser.h
        MethodParser.h
        )

target_include_directories(vcf-codegen
 PRIVATE
  "${CLANG_INCLUDE_DIRS}"
  "${JINJA2CPP_INCLUDE_DIR}"
   )
  
target_link_libraries(vcf-codegen PUBLIC libclang jinja2cpp)

set_target_properties( vcf-codegen
		PROPERTIES
        CXX_STANDARD 17
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}"
		)

if (APPLE)
	install(TARGETS vcf-codegen DESTINATION lib)
else()
	install(TARGETS vcf-codegen DESTINATION bin)
endif ()
