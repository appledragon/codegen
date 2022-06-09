cmake_minimum_required(VERSION 3.16)

if (APPLE OR LINUX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "@executable_path;${CMAKE_INSTALL_PREFIX}/lib")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

file(GLOB codegen-common-src 
        main.cpp
        Utils.h
        ClassInfo.h
        ClassInfo.cpp
        HeaderParser.h
        ClassParser.h
        MethodParser.h
        EnumParser.h
    )

set(codegen-common-link libclang)
set(codegen-common-inc "${CLANG_INCLUDE_DIRS}")

if (AST_DUMP_JSON)
    add_executable(codegen
            ${codegen-common-src}
            JsonRender.cpp
            )
    
    target_include_directories(codegen
            PRIVATE
            codegen-common-inc
            )
   target_link_libraries(codegen PUBLIC ${codegen-common-link})
else()
    add_executable(codegen
            ${codegen-common-src}
            Jinja2Render.cpp
            generator/FileGenerator.h
            generator/FileGenerator.cpp
            )
    target_include_directories(codegen
            PRIVATE
            codegen-common-inc
            "${JINJA2CPP_INCLUDE_DIR}"
            )
    target_link_libraries(codegen PUBLIC ${codegen-common-link} jinja2cpp)
endif(AST_DUMP_JSON)

set_target_properties(codegen
		PROPERTIES
        CXX_STANDARD 17
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}"
		)

if (APPLE)
	install(TARGETS codegen DESTINATION lib)
else()
	install(TARGETS codegen DESTINATION bin)
endif ()
