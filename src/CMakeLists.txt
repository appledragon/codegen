cmake_minimum_required(VERSION 3.16)

if (APPLE OR LINUX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "@executable_path;${CMAKE_INSTALL_PREFIX}/lib")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

file(GLOB vcf-codegen-common-src 
        main.cpp
        Utils.h
        ClassInfo.h
        ClassInfo.cpp
        HeaderParser.h
        ClassParser.h
        MethodParser.h
        EnumParser.h
    )

set(vcf-codegen-common-link libclang)
set(vcf-codegen-common-inc "${CLANG_INCLUDE_DIRS}")

if (AST_DUMP_JSON)
    add_executable(vcf-codegen
            ${vcf-codegen-common-src}
            JsonRender.cpp
            )
    
    target_include_directories(vcf-codegen
            PRIVATE
            vcf-codegen-common-inc
            "${CMAKE_CURRENT_SOURCE_DIR}/../Jinja2Cpp/output/include"
            )
   target_link_libraries(vcf-codegen PUBLIC ${vcf-codegen-common-link} jinja2cpp)
else()
    add_executable(vcf-codegen
            ${vcf-codegen-common-src}
            Jinja2Render.cpp
            generator/FileGenerator.h
            generator/FileGenerator.cpp
            )
    target_include_directories(vcf-codegen
            PRIVATE
            vcf-codegen-common-inc
            "${JINJA2CPP_INCLUDE_DIR}"
            )
    target_link_libraries(vcf-codegen PUBLIC ${vcf-codegen-common-link} jinja2cpp)
endif(AST_DUMP_JSON)

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
