#pragma once
#include <cstdio>
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
#include "clang-c/Index.h"

class ClassInfoJsonDumper
{
public:
    static void parse(const CXCursor& rootCursor, std::filesystem::path& out_put_dir)
    {
        House4ClassInfos house;
        HeaderParserClientData client_data;
        client_data.p_func = ClassInfoJsonDumper::findTheRightClassInfoObject;
        client_data.p_data = &house;
        clang_visitChildren(rootCursor, HeaderParser::Parser, &client_data);

        for (const auto* item : house.class_infos) {
            std::string unique_file_name = item->name;
            if (unique_file_name.empty())
                continue;
            unique_file_name.append(".json");
            std::filesystem::path tmp = out_put_dir;
            tmp /= unique_file_name;

            ClassInfoJsonDumper::saveToFile(item, tmp.string());
        }
    }

private:
    struct House4ClassInfos
    {
        std::map<unsigned, ClassInfo*> runtime_relations_cache;
        std::set<ClassInfo*> class_infos;
        ClassInfo* class_info_for_functions = nullptr;
        ~House4ClassInfos()
        {
            runtime_relations_cache.clear();
            for (const auto& class_info : class_infos) {
                delete class_info;
            }
            class_infos.clear();
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

        auto* house = static_cast<House4ClassInfos*>(p_data);
        unsigned hash = clang_hashCursor(cursor);
        unsigned parent_hash = clang_hashCursor(parent);

        const auto& iter_parent = house->runtime_relations_cache.find(parent_hash);

        switch (kind) {
            case CXCursor_ClassDecl:
            case CXCursor_StructDecl:
            case CXCursor_ClassTemplate: {
                if (!Utils::isForwardDeclaration(cursor)) {
                    const auto& iter = house->runtime_relations_cache.find(hash);
                    if (house->runtime_relations_cache.end() != iter) {
                        client_data = iter->second;
                    } else {
                        auto* value = new ClassInfo;
                        house->class_infos.insert(value);
                        house->runtime_relations_cache.emplace(std::make_pair(hash, value));
                        client_data = value;
                    }
                }
            } break;
            case CXCursor_FunctionDecl: {
                if (nullptr == house->class_info_for_functions) {
                    house->class_info_for_functions = new ClassInfo;
                    // global api we fake a class info named VCF-CODEGEN-FAKE
                    if (nullptr != house->class_info_for_functions && CXCursor_FunctionDecl == cursor.kind) {
                        house->class_info_for_functions->name = "VCF-CODEGEN-FAKE";
                        ClassParser::VisitClassNameSpaces(cursor, house->class_info_for_functions);
                    }
                    house->class_infos.insert(house->class_info_for_functions);
                }
                client_data = house->class_info_for_functions;
            } break;
            default:
                break;
        }

        if (nullptr == client_data && house->runtime_relations_cache.end() != iter_parent) {
            house->runtime_relations_cache.emplace(std::make_pair(hash, iter_parent->second));
            client_data = iter_parent->second;
        }

        if (nullptr == client_data) {
            static ClassInfo s_classinfo;
            client_data = &s_classinfo;
        }

        return client_data;
    }

    static void saveToFile(const ClassInfo* info, const std::string& output_path)
    {
        if (nullptr == info || output_path.empty())
            return;

        Json class_info_json;
        class_info_json["Namespace"] = info->nameSpace;
        class_info_json["Class"] = info->name;
        class_info_json["File"] = info->sourceLocation;
        Json method_list_json;
        for (const auto& method : info->methodList) {
            if (method.isStatic)
                continue;
            Json method_json;
            // method base information
            method_json["name"] = method.name;
            std::string return_type =
                method.returnInfo.underlyingType.empty() ? method.returnInfo.type : method.returnInfo.underlyingType;
            if (!return_type.empty()) {
                if (method.returnInfo.isPointer) {
                    return_type.append("*");
                }
            }
            method_json["returntype"] = return_type;
            std::string strKeyword;
            if (method.isConst) {
                strKeyword += "const";
            }
            if (method.isVirtual) {
                if (!strKeyword.empty())
                    strKeyword += ",";
                strKeyword += "override";
            }
            method_json["keyword"] = strKeyword;

            // method args information
            const auto argSize = method.args.size();
            for (size_t i = 0; i < argSize; i++) {
                Json arg_json;
                arg_json["name"] = method.args.at(i).name;
                arg_json["type"] = method.args.at(i).type;
                method_json["args"].push_back(arg_json);
            }
            method_list_json.push_back(method_json);
        }
        class_info_json["Methods"] = method_list_json;

        std::string content = class_info_json.dump(2);

        const std::filesystem::path path{output_path};
        std::ofstream ofs(path);
        ofs << content;
    }
};

class ASTDumper
{
public:
    static void parse(const CXCursor& rootCursor, std::filesystem::path& out_put_dir)
    {
        std::map<unsigned, int> cursor_levels;
        HeaderParserClientData client_data;
        client_data.p_func = ASTDumper::findTheRightClassInfoObject;
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
        std::string cursor_kind_spelling = Utils::getCursorKindSpelling(cursor);
        std::string log;
        for (int index = 0; index < level; index++) log.append("\t");
        log.append("name: ");
        log.append(cursor_spelling);
        log.append("\tkind: ");
        log.append(cursor_kind_spelling);
        switch (cursor.kind) {
            case CXCursor_CXXMethod: {
                appendCXXMethodLog(cursor, log);
            } break;
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
        auto* p_cursor_levels = static_cast<std::map<unsigned, int>*>(p_data);
        unsigned hash = clang_hashCursor(cursor);
        unsigned parent_hash = clang_hashCursor(parent);
        const auto& iter = p_cursor_levels->find(hash);
        if (p_cursor_levels->end() == iter) {
            const auto& iter_parent = p_cursor_levels->find(parent_hash);
            if (p_cursor_levels->end() != iter_parent) {
                int level = iter_parent->second;
                level++;
                p_cursor_levels->emplace(std::make_pair(hash, level));
                printCXCursor(cursor, level);
            } else {
                int level = 0;
                p_cursor_levels->emplace(std::make_pair(parent_hash, level));
                printCXCursor(parent, level);
                level++;
                p_cursor_levels->emplace(std::make_pair(hash, level));
                printCXCursor(cursor, level);
            }
        }

        return &info;
    }
};