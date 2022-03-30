#pragma once
#include <string>
#include "ClassInfo.h"
#include "jinja2cpp/template.h"

class FileGenerator
{

public:
	void setOutputFilePath(const std::string& path);
	void generateFile(const std::shared_ptr<ClassInfo>& classInfo);

private:
	std::string mOutputDir;
	std::string mClassName;
	std::string mAdapterName;

        void RenderFile(const std::string& input, const std::string& output, const jinja2::ValuesMap& map);

	void generateAdapterByJinja(const std::shared_ptr<ClassInfo>& classInfo);
	void generateAdapterHeaderByJinja(const std::shared_ptr<ClassInfo>& classInfo);
	void generateAdapterCppByJinja(const std::shared_ptr<ClassInfo>& classInfo);

	void generateServiceByJinja(const std::shared_ptr<ClassInfo>& classInfo);
	void generateServiceHeaderByJinja(const std::shared_ptr<ClassInfo>& classInfo);
	void generateServiceCppByJinja(const std::shared_ptr<ClassInfo>& classInfo);


	void generateNormalByJinja(const std::shared_ptr<ClassInfo>& classInfo);
        void generateNormalHeaderByJinja(const std::shared_ptr<ClassInfo>& classInfo);
        void generateNormalCppByJinja(const std::shared_ptr<ClassInfo>& classInfo);

        void generateViewModelByJinja(const std::shared_ptr<ClassInfo>& classInfo);
        void generateViewModelJniByJinja(const std::shared_ptr<ClassInfo>& classInfo);
        void generateViewModelJavaByJinja(const std::shared_ptr<ClassInfo>& classInfo);
};
