#include "FileGenerator.h"
#include <jinja2cpp/filesystem_handler.h>
#include <jinja2cpp/template_env.h>
#include <vector>
#include <fstream>
#include <filesystem>

void FileGenerator::setOutputFilePath(const std::string& path)
{
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    mOutputDir = path;
}

void FileGenerator::generateFile(const std::shared_ptr<ClassInfo>& classInfo)
{
    if (mOutputDir.empty())
        return;

    if (!classInfo)
        return;

    if (classInfo->className.empty())
        return;

    if (classInfo->methodList.empty())
        return;

    mClassName = classInfo->className.substr(1, classInfo->className.length());

    if (classInfo->className.rfind("Service") != std::string::npos) {
        // ILoginService -> LoginService

        // ILoginService -> LoginAdapter
        mAdapterName = classInfo->className.substr(1, classInfo->className.length() - 8);
        mAdapterName += "Adapter";

        generateServiceByJinja(classInfo);
    } else if (classInfo->className.rfind("Adapter") != std::string::npos) {
        // ILoginAdapter -> LoginAdapter
        mAdapterName = classInfo->className.substr(1, classInfo->className.length());
        generateAdapterByJinja(classInfo);
    } else if (classInfo->className.rfind("ViewModel") != std::string::npos) {
        generateViewModelByJinja(classInfo);
    } else{
        generateNormalByJinja(classInfo);
    }
}

void FileGenerator::generateNormalByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    generateNormalHeaderByJinja(classInfo);
    generateNormalCppByJinja(classInfo);
}

void FileGenerator::generateAdapterByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    generateAdapterHeaderByJinja(classInfo);
    generateAdapterCppByJinja(classInfo);
}

void FileGenerator::generateServiceByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    generateServiceHeaderByJinja(classInfo);
    generateServiceCppByJinja(classInfo);
}

void FileGenerator::generateServiceHeaderByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    jinja2::ValuesMap params{};

    params.emplace("ns_name", classInfo->classNameSpace);
    params.emplace("service_name", mClassName);
    params.emplace("adapter_name", mAdapterName);

    jinja2::ValuesList methodList{};
    jinja2::ValuesList argList{};
    jinja2::ValuesList returnList{};
    jinja2::ValuesList keywordList{};
    for (const auto& method : classInfo->methodList) {
        if (method.isStatic)
            continue;
        jinja2::ValuesMap args{};
        jinja2::ValuesList arg;
        const auto argSize = method.methodArgs.size();
        for (size_t i = 0; i < argSize; i++) {
            arg.push_back(method.methodArgs.at(i).type);

        }
        args.emplace(method.methodName, arg);
        std::string strKeyword;
        if (method.isConst) {
            strKeyword += "const";
        }
        if (method.isVirtual) {
            if (!strKeyword.empty())
                strKeyword += ",";
            strKeyword += "override";
        }
        keywordList.push_back(strKeyword);
        methodList.push_back(method.methodName);
        returnList.push_back(method.methodReturnInfo.type);
        argList.push_back(args);
    }

    params.emplace("method_list", methodList);
    params.emplace("arg_list", argList);
    params.emplace("return_list", returnList);
    params.emplace("keyword_list", keywordList);

    RenderFile("ServiceHeader.tpl", mOutputDir + "\\Mock" + mClassName + ".h", params);
}

void FileGenerator::generateServiceCppByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    jinja2::ValuesMap params{};
    params.emplace("ns_name", classInfo->classNameSpace);

    params.emplace("service_name", mClassName);
    params.emplace("adapter_name", mAdapterName);

    jinja2::ValuesList argList{};
    jinja2::ValuesList methodList{};

    for (const auto& method : classInfo->methodList) {
        if (method.isStatic)
            continue;
        std::string argString;
        const auto argSize = method.methodArgs.size();
        for (size_t i = 0; i < argSize; i++) {
            argString += "_";
            if (i != argSize - 1) {
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
    const std::filesystem::path path{output};
    std::ofstream ofs(path);
    tpl.Render(ofs, map);
}

void FileGenerator::generateNormalHeaderByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    jinja2::ValuesMap params{};

    params.emplace("ns_name", classInfo->classNameSpace);
    params.emplace("class_name", mClassName);

    jinja2::ValuesList methodList{};
    jinja2::ValuesList argList{};
    jinja2::ValuesList returnList{};
    jinja2::ValuesList keywordList{};
    for (const auto& method : classInfo->methodList) {
        if (method.isStatic)
            continue;
        jinja2::ValuesMap args{};
        jinja2::ValuesList arg;
        const auto argSize = method.methodArgs.size();
        for (size_t i = 0; i < argSize; i++) {
            arg.push_back(method.methodArgs.at(i).type);
        }
        args.emplace(method.methodName, arg);
        std::string strKeyword;
        if (method.isConst) {
            strKeyword += "const";
        }
        if (method.isVirtual) {
            if (!strKeyword.empty())
                strKeyword += ",";
            strKeyword += "override";
        }
        keywordList.push_back(strKeyword);
        methodList.push_back(method.methodName);
        returnList.push_back(method.methodReturnInfo.type);
        argList.push_back(args);
    }

    params.emplace("method_list", methodList);
    params.emplace("arg_list", argList);
    params.emplace("return_list", returnList);
    params.emplace("keyword_list", keywordList);

    RenderFile("NormalHeader.tpl", mOutputDir + "\\Mock" + mClassName + ".h", params);
}

void FileGenerator::generateNormalCppByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    jinja2::ValuesMap params{};
    params.emplace("ns_name", classInfo->classNameSpace);

    params.emplace("class_name", mClassName);
    params.emplace("adapter_name", mAdapterName);

    jinja2::ValuesList argList{};
    jinja2::ValuesList methodList{};

    for (const auto& method : classInfo->methodList) {
        if (method.isStatic)
            continue;
        std::string argString;
        const auto argSize = method.methodArgs.size();
        for (size_t i = 0; i < argSize; i++) {
            argString += "_";
            if (i != argSize - 1) {
                argString += ",";
            }
        }
        methodList.push_back(method.methodName);
        argList.push_back(argString);
    }

    params.emplace("method_list", methodList);
    params.emplace("arg_list", argList);

    RenderFile("NormalCpp.tpl", mOutputDir + "\\Mock" + mClassName + ".cpp", params);
}

void FileGenerator::generateViewModelByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    generateViewModelJniByJinja(classInfo);
    generateViewModelJavaByJinja(classInfo);
}

void FileGenerator::generateViewModelJniByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void FileGenerator::generateViewModelJavaByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void FileGenerator::generateAdapterHeaderByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    jinja2::ValuesMap params{};

    params.emplace("ns_name", classInfo->classNameSpace);
    params.emplace("service_name", mClassName);
    params.emplace("adapter_name", mAdapterName);

    jinja2::ValuesList methodList{};
    jinja2::ValuesList argList{};
    jinja2::ValuesList returnList{};
    jinja2::ValuesList keywordList{};
    for (const auto& method : classInfo->methodList) {
        if (method.isStatic)
            continue;
        jinja2::ValuesMap args{};
        jinja2::ValuesList arg;
        const auto argSize = method.methodArgs.size();
        for (size_t i = 0; i < argSize; i++) {
            arg.push_back(method.methodArgs.at(i).type);
        }
        args.emplace(method.methodName, arg);
        std::string strKeyword;
        if (method.isConst) {
            strKeyword += "const";
        }
        if (method.isVirtual) {
            if (!strKeyword.empty())
                strKeyword += ",";
            strKeyword += "override";
        }
        keywordList.push_back(strKeyword);
        methodList.push_back(method.methodName);
        returnList.push_back(method.methodReturnInfo.type);
        argList.push_back(args);
    }

    params.emplace("method_list", methodList);
    params.emplace("arg_list", argList);
    params.emplace("return_list", returnList);
    params.emplace("keyword_list", keywordList);

    RenderFile("AdapterHeader.tpl", mOutputDir + "\\Mock" + mAdapterName + ".h", params);
}

void FileGenerator::generateAdapterCppByJinja(const std::shared_ptr<ClassInfo>& classInfo)
{
    jinja2::ValuesMap params{};
    params.emplace("service_name", mClassName);
    params.emplace("adapter_name", mAdapterName);

    jinja2::ValuesList argList{};
    jinja2::ValuesList methodList{};

    for (const auto& method : classInfo->methodList) {
        if (method.isStatic)
            continue;
        std::string argString;
        const auto argSize = method.methodArgs.size();
        for (size_t i = 0; i < argSize; i++) {
            argString += "_";
            if (i != argSize - 1) {
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
