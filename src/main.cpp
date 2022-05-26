#include <clang-c/Index.h>

#include <iostream>
#include <string>

#include "ClassInfo.h"
#include "FileGenerator.h"
#include "HeaderParser.h"

int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;

    auto* const resolvedPath = argv[1];
    std::cerr << "Parsing " << resolvedPath << "...\n";

    const CXIndex index = clang_createIndex(0, 1);

    constexpr const char* defaultArguments[] = {
        "-x",
        "c++",
        "-std=c++17",
#ifdef __APPLE__
        R"(-I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1)",
        R"(-I/Users/xiaolongcao/project/libclang/lib/clang/14.0.0/include)",
        R"(-I/Users/xiaolongcao/project/common/src)",
        R"(-I/Users/xiaolongcao/project/common/vendors/nlohmann_json/include)",
        R"(-I/Users/xiaolongcao/project/common/vendors/glog/mac/Release/include)",
        R"(-I/Users/xiaolongcao/project/common/vendors/protobuf/include)"
#else
        R"(-IC:\project\libclang\lib\clang\13.0.0\include)",
        R"(-IC:\project\common\src)",
        R"(-IC:\project\common\vendors\nlohmann_json\include)",
        R"(-IC:\project\common\vendors\glog\win\x64\Release\include)",
        R"(-IC:\project\common\vendors\protobuf\include)"
#endif
    };

    const CXTranslationUnit translation_unit = clang_parseTranslationUnit(index,
                                                                          resolvedPath,
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
    generator.setOutputFilePath(R"(C:\project\common\unittests\mock\adapters)");
    generator.generateFile(classInfo);
    return 0;
}
