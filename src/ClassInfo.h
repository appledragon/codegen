#pragma once
#include <string>
#include <vector>

#include "AccessSpecifier.h"
#include "Utils.h"

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
    AccessSpecifiers acessSpecifier{AccessSpecifiers::UNKNOWN};
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

class BaseClassInfo;

class ClassInfo
{
public:
    std::string classNameSpace;
    std::string className;
    std::string sourceLocation;          // header file name;
    std::string sourceLocationFullPath;  // header file full path in disk
    std::vector<MethodInfo> methodList{};
    std::vector<BaseClassInfo> baseClass{};
    std::vector<FiledInfo> members{};
    bool isTemplateClass{false};
};

class BaseClassInfo : public ClassInfo
{
public:
    AccessSpecifiers acessSpecifier{AccessSpecifiers::UNKNOWN};
};