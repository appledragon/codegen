#pragma once
#include <clang-c/Index.h>

#include <iostream>
#include <string>
#include "ClassInfo.h"
#include "Utils.h"

class MethodParser
{
public:
    static CXChildVisitResult parse(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        auto classInfo = static_cast<ClassInfo*>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        auto method = std::make_shared<MethodInfo>();
        method->methodName = Utils::getCursorNameString(cursor);;
        method->methodReturnType = Utils::getCursorTypeString(clang_getResultType(type));

        method->isConst = clang_CXXMethod_isConst(cursor);
        method->isVirtual = clang_CXXMethod_isVirtual(cursor);
        method->isStatic = clang_CXXMethod_isStatic(cursor);

        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            const auto methodInfo = static_cast<MethodInfo*>(client_data);
            const CXCursorKind childKind = clang_getCursorKind(cursor);
            if (childKind == CXCursor_ParmDecl) {
                const auto arg = std::make_shared<ArgInfo>();
                const auto type = clang_getCursorType(cursor);
                if (Utils::isBuiltinType(type)) {
                    arg->argName = Utils::getCursorNameString(cursor);
                    arg->argType = Utils::getCursorTypeString(cursor);
                    methodInfo->methodArgs.emplace_back(*arg);
                    return CXChildVisit_Continue;
                }

                const CXCursorVisitor visitor =
                    [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
                    const auto argInfo = static_cast<ArgInfo*>(client_data);
                    const CXCursorKind childKind = clang_getCursorKind(cursor);
                    if (childKind == CXCursor_TypeRef || childKind == CXCursor_TemplateRef) {
                        argInfo->argName = Utils::getCursorNameString(parent);
                        argInfo->argType = Utils::getCursorTypeString(parent);
                        auto s2 =
                            Utils::cXStringToStdString(clang_getCursorSpelling(clang_getCursorSemanticParent(parent)));
                        const auto location = Utils::getCursorSourceLocation(cursor);
                        argInfo->sourceLocation = location.first;
                        argInfo->sourceLocationFullPath = location.second;
                        argInfo->argUnderlyingType = Utils::getCursorUnderlyingTypeString(cursor);
                        // return CXChildVisit_Break;
                    }
                    return CXChildVisit_Continue;
                };
                clang_visitChildren(cursor, visitor, arg.get());
                methodInfo->methodArgs.emplace_back(*arg);
            }

            return CXChildVisit_Continue;
        };

        clang_visitChildren(cursor, visitor, method.get());
        classInfo->methodList.emplace_back(*method);

        return CXChildVisit_Continue;
    }
};
