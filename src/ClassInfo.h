#pragma once
#include <string>
#include <vector>
#include "Utils.h"

enum class AccessSpecifiers
{
    PUBLIC,
    PRIVATE,
    PROTECTED
};

class TypeInfo
{
public:
    std::string type;
    std::string rawType;  // remove cv,reference
    std::string underlyingType;
    std::string sourceLocation;          // header file name;
    std::string sourceLocationFullPath;  // header file full path in disk
    Utils::DefaultValueType defaultValue;
    bool isInSystemHeader{false};
    bool isConst{false};
    bool isReference{false};
    bool isBuiltinType{false};
    bool isPointer{false};
};

class ArgInfo : public TypeInfo
{
public:
    std::string name;
    std::string fullName;
};

class FiledInfo : public ArgInfo
{
public:
    AccessSpecifiers acessSpecifier {AccessSpecifiers::PUBLIC};
};

using ReturnInfo = TypeInfo;

class MethodInfo
{
public:
    ReturnInfo methodReturnInfo;
    std::string methodName;
    std::string methodFullName;
    std::vector<ArgInfo> methodArgs{};
    bool isConst{false};
    bool isVirtual{false};
    bool isStatic{false};
};

class ClassInfo
{
public:
    std::string classNameSpace;
    std::string className;
    std::string sourceLocation;          // header file name;
    std::string sourceLocationFullPath;  // header file full path in disk
    std::vector<MethodInfo> methodList{};
    std::vector<ClassInfo> baseClass{};
    std::vector<FiledInfo> members{};
    bool isTemplateClass{false};
};
