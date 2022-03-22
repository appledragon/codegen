#pragma once
#include <clang-c/Index.h>

#include <iostream>
#include <string>
#include "ClassInfo.h"
#include "Utils.h"

class ClassParser
{
public:
    static CXChildVisitResult parse(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        auto classInfo = static_cast<ClassInfo*>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        classInfo->className = name;
        printf("class name is:%s\n", name.c_str());
        if (clang_getCursorKind(parent) == CXCursor_Namespace) {
            const auto ns = Utils::getCursorSpelling(parent);
            classInfo->classNameSpace = ns;
        }

        const auto location = Utils::getCursorSourceLocation(cursor);
        classInfo->sourceLocation = location.first;
        classInfo->sourceLocationFullPath = location.second;

        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data)  {

            const CXCursorKind childKind = clang_getCursorKind(cursor);
            if (childKind != CXCursor_CXXBaseSpecifier)
                return CXChildVisit_Continue;

            const auto classInfo = static_cast<ClassInfo*>(client_data);
            const auto location = Utils::getCursorSourceLocation(cursor);
            ClassInfo baseClass{};

            baseClass.className = Utils::getCursorSpelling(cursor);
            baseClass.sourceLocation = location.first;
            baseClass.sourceLocationFullPath = location.second;
            if (childKind == CXCursor_TemplateRef) {
                baseClass.isTemplateClass = true;
            } else if (childKind == CXCursor_TypeRef) {
                baseClass.isTemplateClass = false;
            }
            classInfo->baseClass.emplace_back(baseClass);
            return CXChildVisit_Continue;
        };

        clang_visitChildren(cursor, visitor, classInfo);
        return CXChildVisit_Continue;
    }
};
