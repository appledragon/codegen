#pragma once
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "ClassInfo.h"
#include "HeaderParser.h"
#include "Utils.h"
#include "jinja2cpp/filesystem_handler.h"
#include "jinja2cpp/template.h"
#include "jinja2cpp/template_env.h"
#include "json.hpp"

class ClassInfoJsonDumper
{
public:
    static void parse(CXCursor rootCursor, std::filesystem::path& out_put_dir)
    {
        House4Girls house;
        HeaderParserClientData client_data;
        client_data.p_func = ClassInfoJsonDumper::findTheRightClassInfoObject;
        client_data.p_data = &house;
        clang_visitChildren(rootCursor, HeaderParser::Parser, &client_data);

        for (const auto* item : house.girls) {
            std::string unique_file_name = item->classNameSpace;
            if (!unique_file_name.empty())
                unique_file_name.append("_");
            unique_file_name.append(item->className);
            unique_file_name.append(".json");
            std::filesystem::path tmp = out_put_dir;
            tmp /= unique_file_name;

            ClassInfoJsonDumper::saveToFile(item, tmp.c_str());
        }
    }

private:
    struct House4Girls
    {
        std::map<std::string, ClassInfo*> runtime_relations_cache;
        std::set<ClassInfo*> girls;
        ~House4Girls()
        {
            runtime_relations_cache.clear();
            for (const auto& girl : girls) {
                delete girl;
            }
            girls.clear();
        }
    };

    static CXClientData findTheRightClassInfoObject(CXCursor cursor, CXCursor parent, void* p_data)
    {
        // fix me: using unique id to built the runtime relations
        if (nullptr == p_data) {
            return nullptr;
        }
        CXClientData client_data = nullptr;
        const CXCursorKind kind = clang_getCursorKind(cursor);

        auto* house = static_cast<House4Girls*>(p_data);
        std::string parent_type = Utils::getCursorTypeString(parent);
        std::string parent_usr = Utils::getCursorUSRString(parent);
        std::string parent_key = parent_type + parent_usr;
        const auto& iter = house->runtime_relations_cache.find(parent_key);
        if (house->runtime_relations_cache.end() != iter) {
            std::string type = Utils::getCursorTypeString(cursor);
            std::string usr = Utils::getCursorUSRString(cursor);
            if (!type.empty() && !usr.empty()) {
                std::string key = type + usr;
                house->runtime_relations_cache.insert(std::make_pair(key, iter->second));
            }

            client_data = iter->second;
        } else if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl || kind == CXCursor_ClassTemplate) &&
                   !Utils::isForwardDeclaration(cursor)) {
            std::string type = Utils::getCursorTypeString(cursor);
            std::string usr = Utils::getCursorUSRString(cursor);
            std::string key = type + usr;
            auto* value = new ClassInfo;
            house->girls.insert(value);
            house->runtime_relations_cache.insert(std::make_pair(key, value));
            client_data = value;
        }

        return client_data;
    }

    static void saveToFile(const ClassInfo* info, const char* output_path)
    {
        if (nullptr == info || nullptr == output_path)
            return;

        jinja2::ValuesMap params{};

        params.emplace("Namespace_placehold", info->classNameSpace);
        params.emplace("Class_placehold", info->className);

        jinja2::ValuesList methodList{};
        jinja2::ValuesList argList{};
        jinja2::ValuesList returnList{};
        jinja2::ValuesList keywordList{};
        for (const auto& method : info->methodList) {
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

        jinja2::TemplateEnv env{};
        env.GetSettings().lstripBlocks = false;
        env.GetSettings().trimBlocks = false;
        jinja2::Template tpl(&env);
        tpl.LoadFromFile("ClassJsonInfo.tpl");
        const std::filesystem::path path{output_path};
        std::ofstream ofs(path);
        tpl.Render(ofs, params);
    }
};

class ClassDepsJsonDumper
{
public:
    struct House4Deps
    {
        std::set<std::string> forward_declarations;
        std::set<std::string> class_declarations;
        std::string file_name;
    };

    static void parse(CXCursor rootCursor, std::filesystem::path& out_put_dir)
    {
        House4Deps house;
        const CXSourceRange range = clang_getCursorExtent(rootCursor);
        const CXSourceLocation location = clang_getRangeStart(range);
        CXFile file = nullptr;
        unsigned line = 0;
        unsigned column = 0;
        unsigned offset = 0;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        house.file_name = Utils::CXStringToStdString(clang_getFileName(file));
        HeaderParserClientData client_data;
        client_data.p_func = ClassDepsJsonDumper::findTheRightClassInfoObject;
        client_data.p_data = &house;
        clang_visitChildren(rootCursor, HeaderParser::Parser, &client_data);

        if (!house.file_name.empty()){
            std::filesystem::path ori_file_path = house.file_name.c_str();
            std::string deps_file_name = ori_file_path.filename().c_str();
            deps_file_name.append("_deps.json");
            ori_file_path = out_put_dir;
            ori_file_path /= deps_file_name;
            ClassDepsJsonDumper::saveToFile(&house, ori_file_path.c_str());
        }
    }

private:
    static CXClientData findTheRightClassInfoObject(CXCursor cursor, CXCursor parent, void* p_data)
    {
        if (nullptr == p_data) {
            return nullptr;
        }

        static ClassInfo info;
        const CXCursorKind kind = clang_getCursorKind(cursor);
        auto* house = static_cast<House4Deps*>(p_data);
        if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl || kind == CXCursor_ClassTemplate)) {
            std::string tmp = Utils::getCursorSpelling(cursor);
            if (Utils::isForwardDeclaration(cursor)) {
                house->forward_declarations.insert(tmp);
            } else {
                house->class_declarations.insert(tmp);
            }
        }
        return &info;
    }

    static void saveToFile(const House4Deps* deps_info, const char* output_path)
    {
        if (nullptr == deps_info || nullptr == output_path)
            return;

        jinja2::ValuesMap params{};

        params.emplace("FileName_placehold", deps_info->file_name);

        jinja2::ValuesList forward_declarations_list{};
        for (const auto& item : deps_info->forward_declarations) {
            forward_declarations_list.push_back(item);
        }
        jinja2::ValuesList class_declarations_list{};
        for (const auto& item : deps_info->class_declarations) {
            class_declarations_list.push_back(item);
        }

        params.emplace("forward_declarations_list", forward_declarations_list);
        params.emplace("class_declarations_list", class_declarations_list);

        jinja2::TemplateEnv env{};
        env.GetSettings().lstripBlocks = false;
        env.GetSettings().trimBlocks = false;
        jinja2::Template tpl(&env);
        tpl.LoadFromFile(
            "ClassDepsJsonInfo.tpl");
        const std::filesystem::path path{output_path};
        std::ofstream ofs(path);
        tpl.Render(ofs, params);
    }
};