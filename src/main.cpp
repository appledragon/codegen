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



/* Auxiliary function for resolving a (relative) path into an absolute path */
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


static std::shared_ptr<ClassInfo> classInfo{nullptr};

CXChildVisitResult Parser(CXCursor cursor, CXCursor parent, CXClientData /* clientData */)
{
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
        return CXChildVisit_Continue;

    const CXCursorKind kind = clang_getCursorKind(cursor);
    const auto name = Utils::getCursorSpelling(cursor);
    const CXType type = clang_getCursorType(cursor);

    if (kind == CXCursorKind::CXCursor_ClassDecl && !Utils::isForwardDeclaration(cursor)) {
        classInfo = std::make_shared<ClassInfo>();
        classInfo->className = name;
        printf("class name is:%s\n", name.c_str());
        if (clang_getCursorKind(parent) == CXCursorKind::CXCursor_Namespace) {
            const auto ns = Utils::getCursorSpelling(parent);
            printf("name space is:%s\n", ns.c_str());
            classInfo->classNameSpace = ns;
        }
    } else if (kind == CXCursorKind::CXCursor_Namespace) {
        printf("name space is:%s\n", name.c_str());
    } else if (kind == CXCursorKind::CXCursor_CXXBaseSpecifier) {
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
        printf(" [filelocation=%s]\n", Utils::getCursorSource(cursor).c_str());
        const CXCursorVisitor visitor = [](CXCursor cursor,
                                           CXCursor parent,
                                           CXClientData client_data) -> CXChildVisitResult
        {
            return CXChildVisit_Continue;
        };

        clang_visitChildren(cursor, visitor, nullptr);

    } else if (kind == CXCursorKind::CXCursor_FunctionDecl) {
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
    } else if (kind == CXCursorKind::CXCursor_CXXMethod) {
        const char* function_name = clang_getCString(clang_getCursorSpelling(cursor));
        const char* return_type = clang_getCString(clang_getTypeSpelling(clang_getResultType(type)));
        printf("%s,%s(", return_type, function_name);

        MethodInfo method{};
        method.methodName = function_name;
        method.methodReturnType = return_type;

        method.isConst = clang_CXXMethod_isConst(cursor);
        method.isVirtual = clang_CXXMethod_isVirtual(cursor);

        const int num_args = clang_Cursor_getNumArguments(cursor);
        for (int i = 0; i < num_args - 1; ++i) {
            CXCursor arg = clang_Cursor_getArgument(cursor, i);
            auto argName = clang_getCString(clang_getCursorSpelling(arg));
            auto argType = clang_getCursorType(arg);
            const char* argDataType = clang_getCString(clang_getTypeSpelling(clang_getArgType(type, i)));
            ArgInfo argInfo{};
            argInfo.argName = argName;
            argInfo.argType = argDataType;
            argInfo.argFullName += argDataType;
            argInfo.argFullName += " ";
            argInfo.argFullName += argName;
            argInfo.isPod = clang_isPODType(clang_getArgType(type, i));
            printf("%s,", argDataType);
            if (clang_CXXMethod_isVirtual(cursor)) {
                method.methodArgs.emplace_back(argDataType);
            }

            auto parentType = clang_getCursorKind(parent);
            if (parentType == CXCursor_TypeRef) {
                
            }
        }
        if (num_args > 0) {
            const char* arg_data_type = clang_getCString(clang_getTypeSpelling(clang_getArgType(type, num_args - 1)));
            printf("%s", arg_data_type);
            if (clang_CXXMethod_isVirtual(cursor)) {
                method.methodArgs.emplace_back(arg_data_type);
            }
        }
        classInfo->methodList.emplace_back(method);
        printf(")\n");

    } else if (kind == CXCursorKind::CXCursor_FunctionTemplate) {
        const CXSourceRange extent = clang_getCursorExtent(cursor);
        const CXSourceLocation startLocation = clang_getRangeStart(extent);
        const CXSourceLocation endLocation = clang_getRangeEnd(extent);

        unsigned int startLine = 0, startColumn = 0;
        unsigned int endLine = 0, endColumn = 0;

        clang_getSpellingLocation(startLocation, nullptr, &startLine, &startColumn, nullptr);
        clang_getSpellingLocation(endLocation, nullptr, &endLine, &endColumn, nullptr);

        std::cout << "  " << name << ": " << endLine - startLine << "\n";
    } else if (kind == CXCursor_ParmDecl) {
        CXCursorKind parentKind = clang_getCursorKind(parent);
        const auto type = clang_getCursorType(cursor);
        auto defType = clang_getTypeSpelling(type);
        auto ss = Utils::CXStringToString(defType);
        auto s2 = clang_getCString(clang_getCursorDisplayName(clang_getCursorSemanticParent(clang_getCursorReferenced(cursor))));
        std::cout << "  " << name
                  << ": "
                     "\n";
    } else if (kind == CXCursor_TypeRef)
    {
        auto argType = clang_getCursorType(cursor);
        auto typeValue = Utils::CXStringToString(clang_getTypeSpelling(argType));
        auto referenced = clang_getCursorReferenced(cursor);
        CXSourceRange range = clang_getCursorExtent(referenced);
        CXSourceLocation location = clang_getRangeStart(range);
        CXFile file;
        unsigned line, column, offset;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        auto file_name = Utils::CXStringToString(clang_getFileName(file));
        std::cout << "  " << name
                  << ": "
                     "\n";

    }
    else
    {
        CXSourceRange range = clang_getCursorExtent(cursor);
        CXSourceLocation location = clang_getRangeStart(range);
        CXFile file;
        unsigned line, column, offset;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        auto file_name = Utils::CXStringToString(clang_getFileName(file));
        std::cout << "  " << name << ": "  "\n";

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

    const CXCursor rootCursor = clang_getTranslationUnitCursor(translation_unit);
    clang_visitChildren(rootCursor, Parser, nullptr);

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);
    FileGenerator generator;
    generator.setFilePath(R"(D:\project\common\unittests\mock\services)");
    generator.generateFile(classInfo);
    return 0;
}
