import os,sys
import shutil
import json
import platform

CODEGEN_EXE_PATH = ""
def init_runtime():
    global CODEGEN_EXE_PATH
    codegen_root_path = os.path.join(os.path.dirname(__file__), '../output/bin')
    codegen_root_path = os.path.abspath(codegen_root_path)
    print("code gen root path: {}".format(codegen_root_path))
    sys_type = platform.system()
    if "Windows" == sys_type:
        code_gen_sub_str = "Debug/codegen.exe"
    else:
        code_gen_sub_str = "Debug/codegen"
    
    if "Windows" == sys_type:
        code_gen_sub_str_release = "Release/codegen.exe"
    else:
        code_gen_sub_str_release = "Release/codegen"

    CODEGEN_EXE_PATH = os.path.join(codegen_root_path, code_gen_sub_str)
    if not os.path.exists(CODEGEN_EXE_PATH):
        CODEGEN_EXE_PATH = os.path.join(codegen_root_path, code_gen_sub_str_release)

    CODEGEN_EXE_PATH = os.path.abspath(CODEGEN_EXE_PATH)
    print("code gen exe path: {}".format(CODEGEN_EXE_PATH))

BUILTIN_RULE = None
tips_logger = []
def convert_all_header_file_to_json(header_dir, output_dir, config):
    global CODEGEN_EXE_PATH
    try:
        shutil.rmtree(output_dir)
    except:
        pass
    try:
        os.mkdir(output_dir)
    except:
        pass
    for r, d, f in os.walk(header_dir):
        for file in f:
            file_path = os.path.join(r, file)
            if file_path.endswith(".h") or file_path.endswith(".hpp"):
                command_line = "{} -config={} -file={} -output={}".format(CODEGEN_EXE_PATH, config, file_path, output_dir)
                print(command_line)
                os.chdir(os.path.dirname(CODEGEN_EXE_PATH))
                os.system(command_line)

def prebuild_generate_original_config(dir, originnal_template_path = None):
    global CODEGEN_EXE_PATH
    if originnal_template_path:
        config_original_template = {}
        config_class_rule = {}
        config_default_return = {}
        for r, d, f in os.walk(dir):
            for file in f:
                file_path = os.path.join(r, file)
                
                with open(file_path, "r") as f:
                    json_data = json.load(f)
                    config_class_rule[json_data["Class"]] = "unknown"
                    methods = json_data["Methods"]
                    if methods:
                        for method in methods:
                            if method["returntype"].endswith("*"):
                                config_default_return[method["returntype"]] = "nullptr"
                            else:
                                config_default_return[method["returntype"]] = "unknown"
        config_original_template["class_rule"] = config_class_rule
        config_original_template["default_return_rule"] = config_default_return
        with open(originnal_template_path, "w+") as outfile:
            json.dump(config_original_template, outfile, indent=4)

def build_relationship_from_json(dir):
    global CODEGEN_EXE_PATH
    class_dict = {}
    for r, d, f in os.walk(dir):
        for file in f:
            file_path = os.path.join(r, file)
            with open(file_path, "r") as f:
                class_name = os.path.splitext(os.path.basename(file_path))[0]
                json_data = json.load(f)
                class_dict[class_name] = json_data

    return class_dict

def builtin_rule():
    global CODEGEN_EXE_PATH
    global BUILTIN_RULE
    if BUILTIN_RULE is None:
        with open(os.path.join(os.path.dirname(__file__), 'builtin_type_rule.json'), "r") as f:
            BUILTIN_RULE = json.load(f)
    
    return BUILTIN_RULE

def build_jinja2_template_data(class_dict, class_name, custom_rule_config = None):
    global CODEGEN_EXE_PATH
    global tips_logger
    dict = {}
    if class_name in class_dict.keys():
        class_info = class_dict[class_name]
        dict["namespace_list"] = []
        dict["include_file_list"] = []
        dict["method_list"] = []
        namespace = class_info["Namespace"]
        real_class_name = namespace
        if len(real_class_name) > 0:
            real_class_name += "::"
            dict["class_name_unique"] = "{}_{}".format(namespace, class_name)
        else:
             dict["class_name_unique"] = class_name
        real_class_name += class_name
        dict["class_name"] = real_class_name
       
        #dict["namespace_list"].append(class_info["Namespace"])
        dict["include_file_list"].append(os.path.basename(class_info["File"]))
        methods = class_info["Methods"]
        if methods and len(methods) > 0:
            for item in methods:
                tmp = {}
                tmp["return"] = item["returntype"]
                tmp["name"] = item["name"]
                if "args" in item.keys():
                    tmp["args"] = item["args"]
                #default return type
                if item["returntype"].endswith("*"):#pointer
                    tmp["defaultreturn"] = "nullptr"
                else:
                    #1.find in builtin rule list
                    default_value = None
                    builtin_rule_dict = builtin_rule()
                    if item["returntype"] in builtin_rule_dict.keys():
                        default_value = builtin_rule_dict[item["returntype"]]["default_value"]
                        if default_value is None:
                            alisa = builtin_rule_dict[item["returntype"]]["alias"]
                            if len(alisa) > 0 and alisa in builtin_rule_dict.keys():
                                default_value = builtin_rule_dict[alisa]["default_value"]
                                if not default_value:
                                    tips_logger.append("can not found {} builtin type default value rule".format(item["returntype"]))

                    if default_value is not None:
                        tmp["defaultreturn"] = default_value
                    else:
                        tips_logger.append("can not found {} default value rule".format(item["returntype"]))
                        tmp["defaultreturn"] = "{}"
                                
                dict["method_list"].append(tmp)
 
    return dict


