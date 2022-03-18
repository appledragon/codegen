#pragma once
#include <string>
#include <vector>


class MethodInfo
{
public:
	std::string methodReturnType;
	std::string methodName;
	std::vector<std::string> methodArgs{};
	bool isConst{ false };
	bool isVirtual{ false };
};

class ClassInfo
{
public:
	std::string classNameSpace;
	std::string className;
	std::vector<MethodInfo> methodList{};

};

