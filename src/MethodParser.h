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
        method->name = Utils::getCursorNameString(cursor);
        method->accessSpecifier = Utils::getCursorAccessSpecifier(cursor);

        VisitReturnInfo(type, *method);

        method->isConst = clang_CXXMethod_isConst(cursor) != 0U;
        method->isVirtual = clang_CXXMethod_isVirtual(cursor) != 0U;
        method->isStatic = clang_CXXMethod_isStatic(cursor) != 0U;

        const CXCursorVisitor visitor =
            [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            return VisitMethodArgs(client_data, cursor);
        };

        clang_visitChildren(cursor, visitor, method.get());
        if (kind == CXCursor_Constructor) {
            classInfo->constructors.emplace_back(*method);
        } else {
            classInfo->methodList.emplace_back(*method);
        }
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
            arg->isConst = clang_isConstQualifiedType(clang_getPointeeType(type)) != 0;
            arg->isPointer = CXType_Pointer == type.kind;

            const CXCursorVisitor visitor =
                [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
                auto *const argInfo = static_cast<ArgInfo *>(client_data);
                const CXCursorKind childKind = clang_getCursorKind(cursor);

                if (childKind == CXCursor_TypeRef || childKind == CXCursor_TemplateRef) {
                    argInfo->underlyingType = Utils::getCursorUnderlyingTypeString(cursor);
                    const auto type = clang_getCursorType(parent);
                    argInfo->isConst = clang_isConstQualifiedType(clang_getPointeeType(type)) != 0;
                    if (CXType_LValueReference == type.kind || CXType_RValueReference == type.kind) {
                        argInfo->isReference = true;
                    }
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
            methodInfo->args.emplace_back(*arg);
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
            method.returnInfo.type = Utils::getCursorTypeString(returnType);
            method.returnInfo.isBuiltinType = true;
            method.returnInfo.isInSystemHeader = true;
        } else if (Utils::isForwardDeclaration(returnCursor)) {
            if (CXType_Pointer == returnType.kind) {
                method.returnInfo.isPointer = true;
            } else if (CXType_LValueReference == returnType.kind || CXType_RValueReference == returnType.kind) {
                method.returnInfo.isReference = true;
            }
            CXType type = clang_getPointeeType(returnType);
            std::string underflying_name;
            std::string type_name = Utils::getCursorTypeString(clang_getPointeeType(returnType));
            if (CXType_Typedef == type.kind) {
                const auto typedef_Cursor = clang_getTypeDeclaration(type);
                underflying_name = Utils::getCursorUnderlyingTypeString(typedef_Cursor);
                //std::string typedef_name = Utils::getCursorTypeString(typedef_Cursor);
                //Utils::replaceAllSubString(type_name, typedef_name, underflying_name); //not best solution
            }
            method.returnInfo.type = type_name;
            if (!underflying_name.empty())
                method.returnInfo.underlyingType = underflying_name;
        } else {
            // return has no name
            // method->methodReturnInfo.name = Utils::getCursorNameString(returnCursor);
            method.returnInfo.type = Utils::getCursorTypeString(returnCursor);
            const auto location = Utils::getCursorSourceLocation(returnCursor);
            method.returnInfo.isInSystemHeader = Utils::isInSystemHeader(returnCursor);

            method.returnInfo.sourceLocation = location.first;
            method.returnInfo.sourceLocationFullPath = location.second;
            method.returnInfo.underlyingType = Utils::getCursorUnderlyingTypeString(returnCursor);
        }
    }
};
