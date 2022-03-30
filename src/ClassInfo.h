#pragma once
#include <string>
#include <vector>

class ArgInfo
{
public:
    std::string type;
    std::string rawType; // remove cv,reference
    std::string underlyingType;
    std::string name;
    std::string fullName;
    std::string sourceLocation;          // header file name;
    std::string sourceLocationFullPath;  // header file full path in disk
    bool isConst{false};
    bool isReference{false};
    bool isBuiltinType{false};
};

using ReturnInfo = ArgInfo;

class MethodInfo
{
public:
    ReturnInfo methodReturnInfo;
    std::string methodName;
    std::string methodFullName;
    std::vector<ArgInfo> methodArgs{};
    bool isConst{ false };
    bool isVirtual{ false };
    bool isStatic{false};
};

class ClassInfo
{
public:
    std::string classNameSpace;
    std::string className;
    std::string sourceLocation; // header file name;
    std::string sourceLocationFullPath;  // header file full path in disk
    std::vector<MethodInfo> methodList{};
    std::vector<ClassInfo> baseClass{};
    bool isTemplateClass{false};
};

