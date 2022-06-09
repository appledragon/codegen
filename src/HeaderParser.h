#pragma once
#include <clang-c/Index.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <string>

#include "ClassParser.h"
#include "EnumParser.h"
#include "MethodParser.h"
#include "Utils.h"

using fnFindTheRightGirl = std::function<CXClientData(CXCursor, CXCursor, void*)>;

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
            return CXChildVisit_Recurse;

        if (nullptr == clientData)
            return CXChildVisit_Break;

        const auto* header_parser_client_data = static_cast<HeaderParserClientData*>(clientData);
        if (nullptr == header_parser_client_data->p_func)
            return CXChildVisit_Break;

        const CXClientData clang_index_client_data =
            header_parser_client_data->p_func(cursor, parent, header_parser_client_data->p_data);
        if (nullptr == clang_index_client_data)
            return CXChildVisit_Recurse;

        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
#if defined DEBUGLOG_ENABLE
        std::string cursor_spelling = Utils::getCursorSpelling(cursor);
        std::string cursor_kind_spelling = Utils::getCursorKindSpelling(cursor);
        std::printf("%s--->%s\n", cursor_kind_spelling.c_str(), cursor_spelling.c_str());
#endif
        switch (kind) {
            case CXCursor_ClassDecl:
            case CXCursor_StructDecl:
            case CXCursor_ClassTemplate: {
                if (!Utils::isForwardDeclaration(cursor)) {
                    ClassParser::VisitClass(cursor, parent, clang_index_client_data);
                }
            } break;
            case CXCursor_EnumDecl: {
                if (!Utils::isForwardDeclaration(cursor)) {
                    EnumParser::VisitEnum(cursor, parent, clang_index_client_data);
                }
            } break;
            case CXCursor_CXXMethod:
            case CXCursor_Constructor:
            case CXCursor_FunctionDecl: {
                MethodParser::VisitClassMethod(cursor, parent, clang_index_client_data);
            } break;
            case CXCursor_Namespace:
            case CXCursor_CXXBaseSpecifier:
            case CXCursor_ParmDecl:
            case CXCursor_FunctionTemplate:
            case CXCursor_TypeRef:
            default: {
            } break;
        }
        return CXChildVisit_Recurse;
    }
};
