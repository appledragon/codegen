#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#ifdef __unix__
#include <limits.h>
#include <stdlib.h>
#endif

#include <iostream>
#include <string>
#include <type_traits>
#include "ClassInfo.h"
#include "FileGenerator.h"
#include "Utils.h"


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

CXChildVisitResult Parser(CXCursor cursor, CXCursor parent, CXClientData clientData)
{
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
        return CXChildVisit_Continue;

    auto classInfo = static_cast<ClassInfo*>(clientData);
    const CXCursorKind kind = clang_getCursorKind(cursor);
    const auto name = Utils::getCursorSpelling(cursor);
    const CXType type = clang_getCursorType(cursor);

    if (kind == CXCursor_ClassDecl && !Utils::isForwardDeclaration(cursor)) {
        classInfo->className = name;
        printf("class name is:%s\n", name.c_str());
        if (clang_getCursorKind(parent) == CXCursor_Namespace) {
            const auto ns = Utils::getCursorSpelling(parent);
            printf("name space is:%s\n", ns.c_str());
            classInfo->classNameSpace = ns;
        }
    } else if (kind == CXCursor_Namespace) {
        printf("name space is:%s\n", name.c_str());
    } else if (kind == CXCursor_CXXBaseSpecifier) {
        const enum CX_CXXAccessSpecifier access = clang_getCXXAccessSpecifier(cursor);
        const unsigned isVirtual = clang_isVirtualBase(cursor);
        const char* accessStr = nullptr;

        switch (access) {
            case CX_CXXInvalidAccessSpecifier:
                accessStr = "invalid";
                break;
            case CX_CXXPublic:
                accessStr = "public";
                break;
            case CX_CXXProtected:
                accessStr = "protected";
                break;
            case CX_CXXPrivate:
                accessStr = "private";
                break;
        }

        printf(" [name=%s access=%s isVirtual=%s]\n",
               name.c_str(),
               accessStr,
               isVirtual ? "true" : "false");
        printf(" [file location=%s]\n", Utils::getCursorSource(cursor).c_str());
        const CXCursorVisitor visitor = [](CXCursor cursor,
                                           CXCursor parent,
                                           CXClientData client_data) -> CXChildVisitResult {
            return CXChildVisit_Continue;
        };

        clang_visitChildren(cursor, visitor, nullptr);

    } else if (kind == CXCursor_FunctionDecl) {
        /* Collect the template parameter kinds from the base template. */
        const int NumTemplateArgs = clang_Cursor_getNumTemplateArguments(cursor);
        if (NumTemplateArgs < 0) {
            printf(" [no template arg info]");
        }
        for (int i = 0; i < NumTemplateArgs; i++) {
            const enum CXTemplateArgumentKind TAK =
                clang_Cursor_getTemplateArgumentKind(cursor, i);
            switch (TAK) {
                case CXTemplateArgumentKind_Type: {
                    const CXType T = clang_Cursor_getTemplateArgumentType(cursor, i);
                    const CXString S = clang_getTypeSpelling(T);
                    printf(" [Template arg %d: kind: %d, type: %s]",
                           i,
                           TAK,
                           clang_getCString(S));
                    clang_disposeString(S);
                }
                break;
                case CXTemplateArgumentKind_Integral:
                    printf(" [Template arg %d: kind: %d, intval: %lld]",
                           i,
                           TAK,
                           clang_Cursor_getTemplateArgumentValue(cursor, i));
                    break;
                default:
                    printf(" [Template arg %d: kind: %d]\n", i, TAK);
            }
        }
    } else if (kind == CXCursor_CXXMethod) {
        const auto function_name = Utils::getCursorNameString(cursor);
        const auto return_type = Utils::getCursorTypeString(clang_getResultType(type));

        MethodInfo method{};
        method.methodName = function_name;
        method.methodReturnType = return_type;

        method.isConst = clang_CXXMethod_isConst(cursor);
        method.isVirtual = clang_CXXMethod_isVirtual(cursor);
        method.isStatic = clang_CXXMethod_isStatic(cursor);
        /*
        const int num_args = clang_Cursor_getNumArguments(cursor);
        for (int i = 0; i < num_args - 1; ++i) {
            CXCursor arg = clang_Cursor_getArgument(cursor, i);
            const auto argName = Utils::getCursorNameString(arg);
            const auto argDataType = Utils::getCursorTypeString(clang_getArgType(type, i));
            ArgInfo argInfo{};
            argInfo.argName = argName;
            argInfo.argType = argDataType;
            argInfo.argFullName += argDataType;
            argInfo.argFullName += " ";
            argInfo.argFullName += argName;
            argInfo.isPod = clang_isPODType(clang_getArgType(type, i));

            if (clang_CXXMethod_isVirtual(cursor)) {
                method.methodArgs.emplace_back(argDataType);
            }
        }
        if (num_args > 0) {
            const auto arg_data_type =
                Utils::CXStringToString(clang_getTypeSpelling(clang_getArgType(type, num_args - 1)));
            if (clang_CXXMethod_isVirtual(cursor)) {
                method.methodArgs.emplace_back(arg_data_type);
            }
        }*/
        classInfo->methodList.emplace_back(method);
    } else if (kind == CXCursor_FunctionTemplate) {
        const CXSourceRange extent = clang_getCursorExtent(cursor);
        const CXSourceLocation startLocation = clang_getRangeStart(extent);
        const CXSourceLocation endLocation = clang_getRangeEnd(extent);

        unsigned int startLine = 0, startColumn = 0;
        unsigned int endLine = 0, endColumn = 0;

        clang_getSpellingLocation(startLocation, nullptr, &startLine, &startColumn, nullptr);
        clang_getSpellingLocation(endLocation, nullptr, &endLine, &endColumn, nullptr);

        std::cout << "  " << name << ": " << endLine - startLine << "\n";
    } else if (kind == CXCursor_ParmDecl) {
        auto typeName = Utils::getCursorTypeString(cursor);
        auto s2 = Utils::CXStringToString(
            clang_getCursorDisplayName(clang_getCursorSemanticParent(clang_getCursorReferenced(cursor))));

        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            const CXCursorKind childKind = clang_getCursorKind(cursor);

            if (childKind == CXCursor_TypeRef) {
                const auto argName = Utils::getCursorNameString(parent);
                auto typeValue = Utils::getCursorTypeString(parent);
                const auto referenced = clang_getCursorReferenced(cursor);
                const auto underlying = clang_getTypedefDeclUnderlyingType(referenced);
                auto ss = Utils::getCursorTypeString(underlying);
                const CXSourceRange range = clang_getCursorExtent(referenced);
                const CXSourceLocation location = clang_getRangeStart(range);
                CXFile file;
                unsigned line, column, offset;
                clang_getFileLocation(location, &file, &line, &column, &offset);
                auto file_name = Utils::CXFileToFilepath(file);

                std::cout << "  " << argName;
                return CXChildVisit_Break;
            }
    
        };

        clang_visitChildren(cursor, visitor, nullptr);
        std::cout << "  " << name;
    } else if (kind == CXCursor_TypeRef) {
        const auto argType = clang_getCursorType(cursor);
        const auto parentType = clang_getCursorKind(parent);
        if (parentType == CXCursor_ParmDecl) {


        }

    } else {
        CXSourceRange range = clang_getCursorExtent(cursor);
        CXSourceLocation location = clang_getRangeStart(range);
        CXFile file;
        unsigned line, column, offset;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        auto file_name = Utils::CXStringToString(clang_getFileName(file));
        std::cout << "  " << name << ": " "\n";

    }

    return CXChildVisit_Recurse;
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
        R"(-IC:\project\libclang\lib\clang\13.0.0\include)",
        R"(-IC:\project\common\src)",
        R"(-IC:\project\common\vendors\glog\win\x64\Release\include)",
        R"(-IC:\project\common\vendors\protobuf\include)"
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
    clang_visitChildren(rootCursor, Parser, classInfo.get());

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);
    FileGenerator generator;
    generator.setOutputFilePath(R"(D:\project\common\unittests\mock\services)");
    generator.generateFile(classInfo);
    return 0;
}
