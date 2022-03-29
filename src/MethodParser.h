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
        const auto classInfo = static_cast<ClassInfo*>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        const auto method = std::make_shared<MethodInfo>();
        method->methodName = Utils::getCursorNameString(cursor);
        const auto returnType = clang_getResultType(type);
        if (Utils::isBuiltinType(returnType)) {
            method->methodReturnInfo.type = Utils::getCursorTypeString(returnType);
            method->methodReturnInfo.isBuiltinType = true;
        } else {
            const auto returnCursor = clang_getTypeDeclaration(clang_getResultType(type));
            method->methodReturnInfo.name = Utils::getCursorNameString(returnCursor);
            method->methodReturnInfo.type = Utils::getCursorTypeString(returnCursor);
            const auto location = Utils::getCursorSourceLocation(returnCursor);
            method->methodReturnInfo.sourceLocation = location.first;
            method->methodReturnInfo.sourceLocationFullPath = location.second;
            method->methodReturnInfo.underlyingType = Utils::getCursorUnderlyingTypeString(returnCursor);
        }


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
                const auto isBuiltinType = Utils::isBuiltinType(type);
                arg->isBuiltinType = isBuiltinType;
                if (isBuiltinType) {
                    arg->name = Utils::getCursorNameString(cursor);
                    arg->type = Utils::getCursorTypeString(cursor);
                    methodInfo->methodArgs.emplace_back(*arg);
                    return CXChildVisit_Continue;
                }

                if (type.kind == CXType_LValueReference || type.kind == CXType_RValueReference) {
                    CXType pointee = clang_getPointeeType(type);

                   // arg->argName = Utils::getCursorNameString(referenced);
                    arg->name = Utils::getCursorNameString(cursor);
                    arg->type = Utils::getCursorTypeString(cursor);
                    methodInfo->methodArgs.emplace_back(*arg);
                    return CXChildVisit_Continue;
                }

                const CXCursorVisitor visitor =
                    [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
                    const auto argInfo = static_cast<ArgInfo*>(client_data);
                    const CXCursorKind childKind = clang_getCursorKind(cursor);
                    if (childKind == CXCursor_TypeRef || childKind == CXCursor_TemplateRef) {
                        argInfo->name = Utils::getCursorNameString(parent);
                        argInfo->type = Utils::getCursorTypeString(parent);
                        auto s2 =
                            Utils::CXStringToStdString(clang_getCursorSpelling(clang_getCursorSemanticParent(parent)));
                        const auto location = Utils::getCursorSourceLocation(cursor);
                        argInfo->sourceLocation = location.first;
                        argInfo->sourceLocationFullPath = location.second;
                        argInfo->underlyingType = Utils::getCursorUnderlyingTypeString(cursor);
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