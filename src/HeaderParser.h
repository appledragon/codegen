#pragma once
#include <clang-c/Index.h>

#include <functional>
#include <iostream>
#include <string>

#include "ClassParser.h"
#include "EnumParser.h"
#include "MethodParser.h"
#include "Utils.h"

using fnFindTheRightGirl = std::function<CXClientData(CXCursor, CXCursor, void *)>;

struct HeaderParserClientData
{
    fnFindTheRightGirl p_func = nullptr;
    void* p_data = nullptr;
};

class HeaderParser
{
public:
    static CXChildVisitResult Parser(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
            return CXChildVisit_Continue;

        if (nullptr == clientData)
            return CXChildVisit_Break;

        auto* header_parser_client_data = static_cast<HeaderParserClientData* >(clientData);
        if (nullptr == header_parser_client_data->p_func)
            return CXChildVisit_Break;

        CXClientData clang_index_client_data = header_parser_client_data->p_func(cursor, parent, header_parser_client_data->p_data);
        if (nullptr == clang_index_client_data)
            return CXChildVisit_Recurse;

        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl || kind == CXCursor_ClassTemplate) &&
            !Utils::isForwardDeclaration(cursor)) {
            ClassParser::VisitClass(cursor, parent, clang_index_client_data);
        } else if (kind == CXCursor_EnumDecl && !Utils::isForwardDeclaration(cursor)) {
            EnumParser::VisitEnum(cursor, parent, clang_index_client_data);
        } else if (kind == CXCursor_Namespace) {
            CXCursor parentNameSpace = clang_getCursorSemanticParent(cursor);
            while (parentNameSpace.kind == CXCursor_Namespace) {
                const auto usr = Utils::getCursorUSRString(parentNameSpace);
                parentNameSpace = clang_getCursorSemanticParent(parentNameSpace);
            }
        } else if (kind == CXCursor_CXXBaseSpecifier) {
        } else if (kind == CXCursor_FunctionDecl) {
        } else if (kind == CXCursor_CXXMethod) {
            MethodParser::VisitClassMethod(cursor, parent, clang_index_client_data);
        } else if (kind == CXCursor_FunctionTemplate) {
        } else if (kind == CXCursor_ParmDecl) {
            auto typeName = Utils::getCursorTypeString(cursor);

            std::cout << "  " << name;
        } else if (kind == CXCursor_TypeRef) {
        } else {
            const CXSourceRange range = clang_getCursorExtent(cursor);
            const CXSourceLocation location = clang_getRangeStart(range);
            CXFile file = nullptr;
            unsigned line = 0;
            unsigned column = 0;
            unsigned offset = 0;
            clang_getFileLocation(location, &file, &line, &column, &offset);
            auto file_name = Utils::CXStringToStdString(clang_getFileName(file));
            std::cout << name;
        }

        return CXChildVisit_Recurse;
    }
};
