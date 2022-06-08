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
    AccessSpecifiers accessSpecifier{AccessSpecifiers::UNKNOWN};
};

using ReturnInfo = TypeInfo;

class MethodInfo
{
public:
    ReturnInfo returnInfo;
    std::string name;
    std::string fullName;
    std::vector<ArgInfo> args{};
    bool isConst{false};
    bool isVirtual{false};
    bool isStatic{false};
    AccessSpecifiers accessSpecifier{AccessSpecifiers::UNKNOWN};
};

class BaseClassInfo;

class ClassInfo
{
public:
    std::string nameSpace;
    std::string name;
    std::string sourceLocation;          // header file name;
    std::string sourceLocationFullPath;  // header file full path in disk
    std::vector<MethodInfo> constructors{};
    std::vector<MethodInfo> methodList{};
    std::vector<BaseClassInfo> baseClass{};
    std::vector<FiledInfo> members{};
    bool isTemplateClass{false};
};

class BaseClassInfo : public ClassInfo
{
public:
    AccessSpecifiers accessSpecifier{AccessSpecifiers::UNKNOWN};
};