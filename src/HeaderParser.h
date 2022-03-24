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

        if (Utils::isForwardDeclaration(cursor)) {
            return CXChildVisit_Continue;
        }

        if (kind == CXCursor_ClassDecl) {
            ClassParser::parse(cursor, parent, clientData);
        } else if (kind == CXCursor_EnumDecl) {
            EnumParser::parse(cursor, parent, clientData);
        } else if (kind == CXCursor_Namespace) {
        } else if (kind == CXCursor_CXXBaseSpecifier) {
        } else if (kind == CXCursor_FunctionDecl) {
            /* Collect the template parameter kinds from the base template. */
            const int NumTemplateArgs = clang_Cursor_getNumTemplateArguments(cursor);
            if (NumTemplateArgs < 0) {
                printf(" [no template arg info]");
            }
            for (int i = 0; i < NumTemplateArgs; i++) {
                const enum CXTemplateArgumentKind TAK = clang_Cursor_getTemplateArgumentKind(cursor, i);
                switch (TAK) {
                    case CXTemplateArgumentKind_Type: {
                        const CXType T = clang_Cursor_getTemplateArgumentType(cursor, i);
                        const CXString S = clang_getTypeSpelling(T);
                        printf(" [Template arg %d: kind: %d, type: %s]", i, TAK, clang_getCString(S));
                        clang_disposeString(S);
                    } break;
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
            auto file_name = Utils::cXStringToStdString(clang_getFileName(file));
            std::cout << name;
        }

        return CXChildVisit_Recurse;
    }
};
