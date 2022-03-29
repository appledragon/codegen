#pragma once
#include <clang-c/Index.h>

#include <iostream>
#include <string>
#include "EnumParser.h"
#include "ClassParser.h"
#include "MethodParser.h"
#include "Utils.h"

class HeaderParser
{
public:
    static CXChildVisitResult Parser(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
            return CXChildVisit_Continue;

        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        if (kind == CXCursor_ClassDecl && !Utils::isForwardDeclaration(cursor)) {
            ClassParser::parse(cursor, parent, clientData);
        } else if (kind == CXCursor_EnumDecl && !Utils::isForwardDeclaration(cursor)) {
            EnumParser::parse(cursor, parent, clientData);
        } else if (kind == CXCursor_Namespace) {
            CXCursor parentNameSpace = clang_getCursorSemanticParent(cursor);
            while (parentNameSpace.kind == CXCursor_Namespace) {
                const auto usr = Utils::getCursorUSRString(parentNameSpace);
                parentNameSpace = clang_getCursorSemanticParent(parentNameSpace);
            }
        } else if (kind == CXCursor_CXXBaseSpecifier) {
        } else if (kind == CXCursor_FunctionDecl) {
        } else if (kind == CXCursor_CXXMethod) {
            MethodParser::parse(cursor, parent, clientData);
        } else if (kind == CXCursor_FunctionTemplate) {
        } else if (kind == CXCursor_ParmDecl) {
            auto typeName = Utils::getCursorTypeString(cursor);

            std::cout << "  " << name;
        } else if (kind == CXCursor_TypeRef) {
        } else {
            const CXSourceRange range = clang_getCursorExtent(cursor);
            const CXSourceLocation location = clang_getRangeStart(range);
            CXFile file;
            unsigned line, column, offset;
            clang_getFileLocation(location, &file, &line, &column, &offset);
            auto file_name = Utils::CXStringToStdString(clang_getFileName(file));
            std::cout << name;
        }

        return CXChildVisit_Recurse;
    }
};
