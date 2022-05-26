#pragma once
#include <clang-c/Index.h>

#include <iostream>
#include <string>

#include "ClassInfo.h"
#include "Utils.h"

class EnumParser
{
public:
    static CXChildVisitResult parse(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        auto *const classInfo = static_cast<ClassInfo *>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        const CXCursorVisitor visitor = [](CXCursor cursor, CXCursor parent, CXClientData client_data) {
            const CXCursorKind childKind = clang_getCursorKind(cursor);
            if (childKind == CXCursor_EnumConstantDecl) {
                const auto key = Utils::getCursorSpelling(cursor);
                const auto value = clang_getEnumConstantDeclValue(cursor);
            }
            return CXChildVisit_Continue;
        };
        clang_visitChildren(cursor, visitor, classInfo);
        return CXChildVisit_Continue;
    }
};
