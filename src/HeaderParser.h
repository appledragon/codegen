#pragma once
#include <clang-c/Index.h>

#include <iostream>
#include <string>
#include "ClassInfo.h"
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

        auto classInfo = static_cast<ClassInfo*>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        if (kind == CXCursor_ClassDecl && !Utils::isForwardDeclaration(cursor)) {
            ClassParser::parse(cursor, parent, clientData);
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
            auto file_name = Utils::cXStringToStdString(clang_getFileName(file));
            std::cout << "  " << name
                      << ": "
                         "\n";
        }

        return CXChildVisit_Recurse;
    }
};
