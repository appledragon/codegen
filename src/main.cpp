#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#ifdef __unix__
#include <limits.h>
#include <stdlib.h>
#endif

#include <iostream>
#include <string>

#include "ClassInfo.h"
#include "FileGenerator.h"
#include "HeaderParser.h"


std::string resolvePath(const char* path)
{
    std::string resolvedPath;

#ifdef __unix__
	char* resolvedPathRaw = new char[PATH_MAX];
	char* result = realpath(path, resolvedPathRaw);

	if (result)
		resolvedPath = resolvedPathRaw;

	delete[] resolvedPathRaw;
#else
    resolvedPath = path;
#endif

    return resolvedPath;
}



int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;

    const auto resolvedPath = resolvePath(argv[1]);
    std::cerr << "Parsing " << resolvedPath << "...\n";

    const CXIndex index = clang_createIndex(0, 1);

    constexpr const char* defaultArguments[] = {
        "-x", "c++",
        "-std=c++17",
        R"(-ID:\libclang\lib\clang\13.0.0\include)",
        R"(-ID:\project\common\src)",
        R"(-ID:\project\common\vendors\glog\win\x64\Release\include)",
        R"(-ID:\project\common\vendors\protobuf\include)"
    };

    const CXTranslationUnit translation_unit = clang_parseTranslationUnit(index,
                                                                          resolvedPath.c_str(),
                                                                          defaultArguments,
                                                                          std::extent_v<decltype(defaultArguments)>,
                                                                          nullptr,
                                                                          0,
                                                                          CXTranslationUnit_None);

    const std::shared_ptr<ClassInfo> classInfo = std::make_shared<ClassInfo>();
    const CXCursor rootCursor = clang_getTranslationUnitCursor(translation_unit);
    clang_visitChildren(rootCursor, HeaderParser::Parser, classInfo.get());

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);
    FileGenerator generator;
    generator.setOutputFilePath(R"(D:\project\common\unittests\mock\services)");
    generator.generateFile(classInfo);
    return 0;
}
