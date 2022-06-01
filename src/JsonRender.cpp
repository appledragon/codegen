#include <clang-c/Index.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ClassInfo.h"
#include "HeaderParser.h"
#include "json.hpp"
using Json = nlohmann::json;

constexpr const char* CONFIG_FILE_KEY = "-config";
constexpr const char* IN_FILE_KEY = "-file";
constexpr const char* OUT_FILE_KEY = "-output";
std::vector<std::string> parseClangRuntimeArguments(std::map<std::string, std::string>& map_opts)
{
    std::vector<std::string> arguments;
    auto iter = map_opts.find(CONFIG_FILE_KEY);
    if (map_opts.end() != iter) {
        std::string config_content;
        Utils::readFileAllContents(iter->second.c_str(), config_content);
        do {
            if (config_content.empty())
                break;

            Json config_json;
            try {
                config_json = Json::parse(config_content);
                printf("parseClangRuntimeArguments config all_content: %s\n", config_content.c_str());
            } catch (Json::parse_error& ex) {
                break;
            }

            if (config_json.is_null()) {
                break;
            }
            Json json_configurations = config_json["configurations"];
            if (json_configurations.is_null() || !json_configurations.is_array()) {
                break;
            }

            for (Json::iterator iter = json_configurations.begin(); json_configurations.end() != iter; iter++) {
                arguments.push_back(*iter);
            }

        } while (false);
    }

    return arguments;
}

struct House4Gril
{
    std::map<std::string, ClassInfo*> runtime_relations_cache;
    std::set<ClassInfo*> girls;
    ~House4Gril()
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

    auto* house = static_cast<House4Gril*>(p_data);
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
    } else if (kind == CXCursor_ClassDecl && !Utils::isForwardDeclaration(cursor)) {
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

int clangJsonRenderMain(int argc, char** argv)
{
    if (argc < 2)
        return -1;
    std::map<std::string, std::string> map_cmd_opts;
    Utils::makeCommandLineOptsMap(argc, argv, map_cmd_opts);

    const auto& iter_file = map_cmd_opts.find(IN_FILE_KEY);
    if (map_cmd_opts.end() == iter_file) {
        return -1;
    }

    std::vector<std::string> vec_arguments = parseClangRuntimeArguments(map_cmd_opts);
    const char* defaultArguments[vec_arguments.size()];
    int args_index = 0;
    for (const auto& item : vec_arguments) {
        defaultArguments[args_index] = item.c_str();
        ++args_index;
    }

    const auto* const resolvedPath = iter_file->second.c_str();
    std::cerr << "Parsing " << resolvedPath << "...\n";

    const CXIndex index = clang_createIndex(0, 1);
    const CXTranslationUnit translation_unit = clang_parseTranslationUnit(index,
                                                                          resolvedPath,
                                                                          defaultArguments,
                                                                          static_cast<int>(vec_arguments.size()),
                                                                          nullptr,
                                                                          0,
                                                                          CXTranslationUnit_None);

    const std::shared_ptr<ClassInfo> classInfo = std::make_shared<ClassInfo>();
    const CXCursor rootCursor = clang_getTranslationUnitCursor(translation_unit);
    House4Gril house;
    HeaderParserClientData client_data;
    client_data.p_func = findTheRightClassInfoObject;
    client_data.p_data = &house;
    clang_visitChildren(rootCursor, HeaderParser::Parser, &client_data);

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);
    return 0;
}