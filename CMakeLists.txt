cmake_minimum_required(VERSION 3.16)

project("vcf-codegen")

option (AST_DUMP_JSON
  "only dump AST to json"
  ON
)

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/vendors/libclang/lib/cmake/clang)
set (CLANG_INSTALL_PREFIX ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/vendors/libclang)
if (AST_DUMP_JSON)
else()
    set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/vendors/Jinja2Cpp/output/lib/jinja2cpp)
endif(AST_DUMP_JSON)

set(CMAKE_ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR})
set (JINJA2CPP_DEPS_MODE internal CACHE STRING "" FORCE)


# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CONFIGURATION_TYPES Release;Debug)

find_package(Clang REQUIRED)

if (AST_DUMP_JSON)
    add_compile_definitions(AST_DUMP_JSON)
else()
    find_package(jinja2cpp REQUIRED)
endif(AST_DUMP_JSON)

if(APPLE)
    find_library(CORE_FOUNDATION NAMES CoreFoundation)
    find_library(FOUNDATION NAMES Foundation)
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf-with-dsym")
endif()

# Set CXX Flag
string(COMPARE EQUAL "${CMAKE_CXX_STANDARD}" "" no_cmake_cxx_standard_set)
if(no_cmake_cxx_standard_set)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
	message(STATUS "Using default C++ standard ${CMAKE_CXX_STANDARD}")

else()
    message(STATUS "Using user specified C++ standard ${CMAKE_CXX_STANDARD}")
endif()


if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")
endif (APPLE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set Library Type
if (APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "" CACHE INTERNAL "")
elseif (WIN32 OR WIN64)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zc:wchar_t-")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zc:wchar_t-")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif (APPLE)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}" CACHE PATH "..." FORCE)

set(LLVM_PATH ../libclang)
link_directories(${LLVM_PATH}/lib)
include_directories()

include_directories(
    ${LLVM_PATH}/include
    ./src
    ./src/generator
)

add_subdirectory( src )

if (APPLE)
    file(GLOB LIBCLANG_RUNTIME "vendors/libclang/lib/libclang.dylib")
elseif (WIN32 OR WIN64)
    file(GLOB LIBCLANG_RUNTIME "vendors/libclang/bin/libclang.dll")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    file(GLOB LIBCLANG_RUNTIME "vendors/libclang/lib/libclang.so")
endif (APPLE)
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    file(COPY ${LIBCLANG_RUNTIME} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
else()
    file(COPY ${LIBCLANG_RUNTIME} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
endif()

if (AST_DUMP_JSON)
else()
    file(GLOB JINJA_TPL "${CMAKE_CURRENT_SOURCE_DIR}/src/generator/jinja_tpl/*.tpl")
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        file(COPY ${JINJA_TPL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
    else()
        file(COPY ${JINJA_TPL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
    endif()
endif(AST_DUMP_JSON)
    