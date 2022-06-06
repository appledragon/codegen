#pragma once
#include <clang-c/Index.h>

#include <algorithm>
#include <iterator>
#include <vector>
#include <string>

#include "ClassInfo.h"
#include "Utils.h"

class ClassParser
{
public:
    static CXChildVisitResult VisitClass(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        auto *const classInfo = static_cast<ClassInfo *>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        classInfo->className = name;

        if (clang_getCursorKind(parent) == CXCursor_Namespace) {
            VisitClassNameSpaces(cursor, classInfo);
        }

        const auto location = Utils::getCursorSourceLocation(cursor);
        classInfo->sourceLocation = location.first;
        classInfo->sourceLocationFullPath = location.second;

        const CXCursorVisitor visitor = [](CXCursor cursor, CXCursor parent, CXClientData client_data) {
            const CXCursorKind childKind = clang_getCursorKind(cursor);

            auto *const classInfo = static_cast<ClassInfo *>(client_data);
            if (childKind == CXCursor_FieldDecl) {
                VisitClassFields(cursor, classInfo);

            } else if (childKind == CXCursor_CXXBaseSpecifier) {
                return VisitBaseClasses(cursor, classInfo);
            }
            return CXChildVisit_Continue;
        };

        clang_visitChildren(cursor, visitor, classInfo);
        return CXChildVisit_Continue;
    }

    static void VisitClassNameSpaces(CXCursor cursor, ClassInfo *classInfo)
    {
        std::vector<std::string> nameSpaces{};
        CXCursor parentNameSpace = clang_getCursorSemanticParent(cursor);
        while (parentNameSpace.kind == CXCursor_Namespace) {
            nameSpaces.emplace_back(Utils::getCursorSpelling(parentNameSpace));
            parentNameSpace = clang_getCursorSemanticParent(parentNameSpace);
        }
        std::string ns;
        for (int x = nameSpaces.size() - 1; x >= 0; --x) {
            ns += nameSpaces.at(x);
            if (x != 0)
                ns += "::";
        }
        classInfo->classNameSpace = ns;
    }

    static void VisitClassFields(CXCursor cursor, ClassInfo *classInfo)
    {
        FiledInfo field{};
        // TODO the same code, unify it
        const auto type = clang_getCursorType(cursor);
        const auto isBuiltinType = Utils::isBuiltinType(type);
        field.isBuiltinType = isBuiltinType;
        field.isInSystemHeader = Utils::isInSystemHeader(cursor);
        field.name = Utils::getCursorNameString(cursor);
        field.type = Utils::getCursorTypeString(cursor);
        const auto location = Utils::getCursorSourceLocation(cursor);
        field.sourceLocation = location.first;
        field.sourceLocationFullPath = location.second;
        field.underlyingType = Utils::getCursorUnderlyingTypeString(cursor);
        classInfo->members.emplace_back(field);
    }

    static CXChildVisitResult VisitBaseClasses(CXCursor cursor, ClassInfo *classInfo)
    {
        const auto baseClass = std::make_shared<ClassInfo>();
        const auto location = Utils::getCursorSourceLocation(cursor);

        baseClass->className = Utils::getCursorSpelling(cursor);
        baseClass->sourceLocation = location.first;
        baseClass->sourceLocationFullPath = location.second;

        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            auto *const bassClassInfo = static_cast<ClassInfo *>(client_data);
            const CXCursorKind childKind = clang_getCursorKind(cursor);
            if (childKind == CXCursor_TemplateRef) {
                bassClassInfo->isTemplateClass = true;
            } else if (childKind == CXCursor_NamespaceRef) {
                bassClassInfo->classNameSpace = Utils::getCursorSpelling(cursor);
            }
            return CXChildVisit_Continue;
        };
        clang_visitChildren(cursor, visitor, baseClass.get());
        classInfo->baseClass.emplace_back(*baseClass);
        return CXChildVisit_Continue;
    }
};
