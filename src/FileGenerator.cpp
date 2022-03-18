#include "FileGenerator.h"
#include <jinja2cpp/filesystem_handler.h>
#include <jinja2cpp/template_env.h>
#include <vector>
#include <fstream>
#include <filesystem>

void FileGenerator::setFilePath(const std::string& path)
{
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directories(path);
	}
	mOutputDir = path;
}

void FileGenerator::generateFile(std::shared_ptr<ClassInfo>& classInfo)
{
	if (mOutputDir.empty())
		return;

	if (!classInfo)
		return;

	if (classInfo->className.empty())
		return;

	if (classInfo->methodList.empty())
		return;

	if (classInfo->className.ends_with("Service"))
	{
		// ILoginService -> LoginService
		mClassName = classInfo->className.substr(1, classInfo->className.length());

		// ILoginService -> LoginAdapter
		mAdapterName = classInfo->className.substr(1, classInfo->className.length() - 8);
		mAdapterName += "Adapter";

		generateServiceByJinja(classInfo);
	}
	else if (classInfo->className.ends_with("Adapter"))
	{
		// ILoginAdapter -> LoginAdapter
		mAdapterName = classInfo->className.substr(1, classInfo->className.length());
		generateAdapterByJinja(classInfo);
	}
}

void FileGenerator::generateAdapterByJinja(std::shared_ptr<ClassInfo>& classInfo)
{
	generateAdapterHeaderByJinja(classInfo);
	generateAdapterCppByJinja(classInfo);
}

void FileGenerator::generateServiceByJinja(std::shared_ptr<ClassInfo>& classInfo)
{
	generateServiceHeaderByJinja(classInfo);
	generateServiceCppByJinja(classInfo);
}

void FileGenerator::generateServiceHeaderByJinja(std::shared_ptr<ClassInfo>& classInfo)
{
	jinja2::ValuesMap params{ };
	params.emplace("service_name", mClassName);
	params.emplace("adapter_name", mAdapterName);

	jinja2::ValuesList methodList{};
	jinja2::ValuesList argList{};
	jinja2::ValuesList returnList{};
	jinja2::ValuesList keywordList{};
	for (const auto& method : classInfo->methodList)
	{
		jinja2::ValuesMap args{};
		jinja2::ValuesList arg;
		std::string argString;
		const auto argSize = method.methodArgs.size();
		for (int i = 0; i < argSize; i++)
		{
			arg.push_back(method.methodArgs.at(i));

		}
		args.emplace(method.methodName, arg);
		std::string strKeyword;
		if (method.isConst)
		{
			strKeyword += "const";
		}
		if (method.isVirtual)
		{
			if (!strKeyword.empty())
				strKeyword += ",";
			strKeyword += "override";
		}
		keywordList.push_back(strKeyword);
		methodList.push_back(method.methodName);
		returnList.push_back(method.methodReturnType);
		argList.push_back(args);
	}

	params.emplace("method_list", methodList);
	params.emplace("arg_list", argList);
	params.emplace("return_list", returnList);
	params.emplace("keyword_list", keywordList);

	RenderFile("ServiceHeader.tpl", mOutputDir + "\\Mock" + mClassName + ".h", params);
}

void FileGenerator::generateServiceCppByJinja(std::shared_ptr<ClassInfo>& classInfo)
{
	jinja2::ValuesMap params{ };
	params.emplace("service_name", mClassName);
	params.emplace("adapter_name", mAdapterName);

	jinja2::ValuesList argList{};
	jinja2::ValuesList methodList{};

	for (const auto& method : classInfo->methodList)
	{
		std::string argString;
		const auto argSize = method.methodArgs.size();
		for (int i = 0; i < argSize; i++)
		{
			argString += "_";
			if (i != argSize - 1)
			{
				argString += ",";
			}
		}
		methodList.push_back(method.methodName);
		argList.push_back(argString);
	}

	params.emplace("method_list", methodList);
	params.emplace("arg_list", argList);

	RenderFile("ServiceCpp.tpl", mOutputDir + "\\Mock" + mClassName + ".cpp", params);
}

void FileGenerator::RenderFile(const std::string& input, const std::string& output, const jinja2::ValuesMap& map)
{
	jinja2::TemplateEnv env{};
	env.GetSettings().lstripBlocks = false;
	env.GetSettings().trimBlocks = false;
	jinja2::Template tpl(&env);
	tpl.LoadFromFile(input);
	std::filesystem::path path{ output };
	std::ofstream ofs(path);
	tpl.Render(ofs, map);
}

void FileGenerator::generateAdapterHeaderByJinja(std::shared_ptr<ClassInfo>& classInfo)
{

}

void FileGenerator::generateAdapterCppByJinja(std::shared_ptr<ClassInfo>& classInfo)
{
	jinja2::ValuesMap params{ };
	params.emplace("service_name", mClassName);
	params.emplace("adapter_name", mAdapterName);

	jinja2::ValuesList argList{};
	jinja2::ValuesList methodList{};

	for (const auto& method : classInfo->methodList)
	{
		std::string argString;
		const auto argSize = method.methodArgs.size();
		for (int i = 0; i < argSize; i++)
		{
			argString += "_";
			if (i != argSize - 1)
			{
				argString += ",";
			}
		}
		methodList.push_back(method.methodName);
		argList.push_back(argString);
	}

	params.emplace("method_list", methodList);
	params.emplace("arg_list", argList);

	RenderFile("AdapterCpp.tpl", mOutputDir + "\\Mock" + mAdapterName + ".cpp", params);
}
