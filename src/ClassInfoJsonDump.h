#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
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

            ClassInfoJsonDumper::saveToFile(item, tmp.string());
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

    static void saveToFile(const ClassInfo* info, const std::string& output_path)
    {
        if (nullptr == info || output_path.empty())
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
            std::string return_info = method.methodReturnInfo.type;
            if (!return_info.empty()) {
                if (method.methodReturnInfo.isPointer) {
                    return_info.append("*");
                }
            }
            returnList.push_back(return_info);
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

class ASTTreeDumper
{
public:

    static void parse(CXCursor rootCursor, std::filesystem::path& out_put_dir)
    {
        std::map<unsigned, int > cursor_levels;
        HeaderParserClientData client_data;
        client_data.p_func = ASTTreeDumper::findTheRightClassInfoObject;
        client_data.p_data = &cursor_levels;
        clang_visitChildren(rootCursor, HeaderParser::Parser, &client_data);
    }

private:
    static void appendCXXMethodLog(const CXCursor& cursor, std::string& log)
    {

    }

    static void printCXCursor(const CXCursor& cursor, int level)
    {
        std::string cursor_spelling = Utils::getCursorSpelling(cursor);
        std::string cursor_kind_spelling =  Utils::getCursorKindSpelling(cursor);
        std::string log;
        for (int index = 0; index < level; index++)
            log.append("\t");
        log.append("name: ");
        log.append(cursor_spelling);
        log.append("\tkind: ");
        log.append(cursor_kind_spelling);
        switch (cursor.kind)
        {
            case CXCursor_CXXMethod:
            {
                appendCXXMethodLog(cursor, log);
            }
            break;
            default:
            break;
        }
        log.append("\n");
        std::cout << log;
    }

    static CXClientData findTheRightClassInfoObject(CXCursor cursor, CXCursor parent, void* p_data)
    {
        if (nullptr == p_data) {
            return nullptr;
        }

        static ClassInfo info;
        auto* p_cursor_levels = static_cast<std::map<unsigned, int >* >(p_data);
        unsigned hash = clang_hashCursor(cursor);
        unsigned parent_hash = clang_hashCursor(parent);
        const auto& iter = p_cursor_levels->find(hash);
        if (p_cursor_levels->end() == iter){
            const auto& iter_parent = p_cursor_levels->find(parent_hash);
            if (p_cursor_levels->end() != iter_parent)
            {
                int level = iter_parent->second;
                level++;
                p_cursor_levels->insert(std::make_pair(hash, level));
                printCXCursor(cursor, level);
            }
            else {
                int level = 0;
                p_cursor_levels->insert(std::make_pair(parent_hash, level));
                printCXCursor(parent, level);
                level++;
                p_cursor_levels->insert(std::make_pair(hash, level));
                printCXCursor(cursor, level);
            }
        }

        return &info;
    }
};