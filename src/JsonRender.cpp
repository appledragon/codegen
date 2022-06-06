#include <clang-c/Index.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

#include "ClassInfo.h"
#include "HeaderParser.h"
#include "json.hpp"
#include "ClassInfoJsonDump.h"

using Json = nlohmann::json;

constexpr const char* CONFIG_FILE_KEY = "-config";
constexpr const char* IN_FILE_KEY = "-file";
constexpr const char* OUT_FILE_KEY = "-output";
constexpr const char* CHECK_DEPS = "-checkdeps";

std::vector<std::string> parseClangRuntimeArguments(std::map<std::string, std::string>& map_opts)
{
    std::vector<std::string> arguments;
    const auto iter = map_opts.find(CONFIG_FILE_KEY);
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

            for (auto& json_configuration : json_configurations) {
                arguments.push_back(json_configuration);
            }

        } while (false);
    }

    return arguments;
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
    std::filesystem::path out_put_dir = std::filesystem::current_path();
    const auto& iter_output_dir = map_cmd_opts.find(OUT_FILE_KEY);
    if (map_cmd_opts.end() != iter_output_dir) {
        out_put_dir = iter_output_dir->second.c_str();
    } 
    
    std::vector<std::string> vec_arguments = parseClangRuntimeArguments(map_cmd_opts);

    std::vector<char*> defaultArguments{};
    defaultArguments.reserve(vec_arguments.size());

    for (auto& s : vec_arguments) defaultArguments.push_back(s.data());

    const auto* const resolvedPath = iter_file->second.c_str();
    std::cerr << "Parsing " << resolvedPath << "...\n";
    
    const CXIndex index = clang_createIndex(0, 1);
    const char *const *command_line_args = defaultArguments.data();
    const CXTranslationUnit translation_unit = clang_parseTranslationUnit(index,
                                                                          resolvedPath,
                                                                          command_line_args,
                                                                          static_cast<int>(defaultArguments.size()),
                                                                          nullptr,
                                                                          0,
                                                                          CXTranslationUnit_None);

    const CXCursor rootCursor = clang_getTranslationUnitCursor(translation_unit);

    const auto& iter_checkdeps = map_cmd_opts.find(CHECK_DEPS);
    if (map_cmd_opts.end() != iter_checkdeps) {
        ClassDepsJsonDumper::parse(rootCursor, out_put_dir);
    }else {
        ClassInfoJsonDumper::parse(rootCursor, out_put_dir);
    }   

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return 0;
}