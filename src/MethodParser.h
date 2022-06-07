#pragma once
#include <clang-c/Index.h>

#include <string>

#include "ClassInfo.h"
#include "Utils.h"

class MethodParser
{
public:
    static CXChildVisitResult VisitClassMethod(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        auto *const classInfo = static_cast<ClassInfo *>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const auto name = Utils::getCursorSpelling(cursor);
        const CXType type = clang_getCursorType(cursor);

        const auto method = std::make_shared<MethodInfo>();
        method->methodName = Utils::getCursorNameString(cursor);

        VisitReturnInfo(type, *method);

        method->isConst = clang_CXXMethod_isConst(cursor) != 0U;
        method->isVirtual = clang_CXXMethod_isVirtual(cursor) != 0U;
        method->isStatic = clang_CXXMethod_isStatic(cursor) != 0U;

        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            return VisitMethodArgs(client_data, cursor);
        };

        clang_visitChildren(cursor, visitor, method.get());
        classInfo->methodList.emplace_back(*method);

        return CXChildVisit_Continue;
    }

    static CXChildVisitResult VisitMethodArgs(CXClientData client_data, CXCursor cursor)
    {
        auto *const methodInfo = static_cast<MethodInfo *>(client_data);
        const CXCursorKind childKind = clang_getCursorKind(cursor);
        if (childKind == CXCursor_ParmDecl) {
            const auto arg = std::make_shared<ArgInfo>();
            const auto type = clang_getCursorType(cursor);
            const auto isBuiltinType = Utils::isBuiltinType(type);
            arg->isBuiltinType = isBuiltinType;
            arg->isInSystemHeader = Utils::isInSystemHeader(cursor);
            arg->name = Utils::getCursorNameString(cursor);
            arg->type = Utils::getCursorTypeString(cursor);

            const CXCursorVisitor visitor =
                [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
                auto *const argInfo = static_cast<ArgInfo *>(client_data);
                const CXCursorKind childKind = clang_getCursorKind(cursor);
                if (childKind == CXCursor_TypeRef || childKind == CXCursor_TemplateRef) {
                    argInfo->isReference = true;
                    const auto type = clang_getCursorType(parent);
                    argInfo->isConst = clang_isConstQualifiedType(clang_getPointeeType(type)) != 0;

                    const auto referenced = clang_getCursorReferenced(cursor);
                    if (argInfo->type.empty()) {
                        argInfo->type = Utils::getCursorTypeString(cursor);
                    }
                    if (argInfo->name.empty()) {
                        argInfo->name = Utils::getCursorNameString(referenced);
                    }
                    argInfo->rawType = Utils::getCursorTypeString(referenced);
                    auto s2 =
                        Utils::CXStringToStdString(clang_getCursorSpelling(clang_getCursorSemanticParent(parent)));
                    const auto location = Utils::getCursorSourceLocation(cursor);
                    argInfo->sourceLocation = location.first;
                    argInfo->sourceLocationFullPath = location.second;
                    argInfo->underlyingType = Utils::getCursorUnderlyingTypeString(cursor);
                    // return CXChildVisit_Break;
                } else if (childKind >= CXCursor_FirstExpr && childKind <= CXCursor_LastExpr) {
                    if (Utils::hasDefaultValue(childKind)) {
                        Utils::DefaultValueType defaultValue;
                        Utils::EvaluateDefaultValue(cursor, defaultValue);
                        argInfo->defaultValue = defaultValue;
                    } else {
                        VisitArgDefaultValue(childKind, cursor, argInfo);
                    }
                }
                return CXChildVisit_Continue;
            };
            clang_visitChildren(cursor, visitor, arg.get());
            methodInfo->methodArgs.emplace_back(*arg);
        } else if (CXCursor_TypeRef == childKind && methodInfo->methodReturnInfo.type.empty()) {
            const CXType type = clang_getCursorType(cursor);
            methodInfo->methodReturnInfo.type = Utils::getCursorNameString(cursor);
            methodInfo->methodReturnInfo.isBuiltinType = false;
            methodInfo->methodReturnInfo.isInSystemHeader = false;
            // printf("%s-->return", methodInfo->methodReturnInfo.type.c_str());
        }

        return CXChildVisit_Continue;
    }

    static void VisitArgDefaultValue(const CXCursorKind childKind, CXCursor cursor, ArgInfo *argInfo)
    {
        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            auto *const argInfo = static_cast<ArgInfo *>(client_data);
            const CXCursorKind childKind = clang_getCursorKind(cursor);
            Utils::DefaultValueType defaultValue;
            Utils::EvaluateDefaultValue(cursor, defaultValue);
            argInfo->defaultValue = defaultValue;

            return CXChildVisit_Continue;
        };
        clang_visitChildren(cursor, visitor, argInfo);
    }

    static void VisitReturnInfo(const CXType type, MethodInfo &method)
    {
        const auto returnType = clang_getResultType(type);

        const auto returnCursor = clang_getTypeDeclaration(returnType);

        if (Utils::isBuiltinType(returnType)) {
            method.methodReturnInfo.type = Utils::getCursorTypeString(returnType);
            method.methodReturnInfo.isBuiltinType = true;
            method.methodReturnInfo.isInSystemHeader = true;
        } else if (Utils::isForwardDeclaration(returnCursor)) {
            if (CXType_Pointer == returnType.kind) {
                method.methodReturnInfo.isPointer = true;
            } else if (CXType_LValueReference == returnType.kind || CXType_RValueReference == returnType.kind) {
                method.methodReturnInfo.isReference = true;
            }
            method.methodReturnInfo.type = Utils::getCursorTypeString(clang_getPointeeType(returnType));
        } else {
            // return has no name
            // method->methodReturnInfo.name = Utils::getCursorNameString(returnCursor);
            method.methodReturnInfo.type = Utils::getCursorTypeString(returnCursor);
            const auto location = Utils::getCursorSourceLocation(returnCursor);
            method.methodReturnInfo.isInSystemHeader = Utils::isInSystemHeader(returnCursor);

            method.methodReturnInfo.sourceLocation = location.first;
            method.methodReturnInfo.sourceLocationFullPath = location.second;
            method.methodReturnInfo.underlyingType = Utils::getCursorUnderlyingTypeString(returnCursor);
        }
    }
};
