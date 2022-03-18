#pragma once
#include <string>
#include "ClassInfo.h"
#include "jinja2cpp/template.h"

class FileGenerator
{

public:
	void setFilePath(const std::string& path);

	void generateFile(std::shared_ptr<ClassInfo> &classInfo);

private:
	std::string mOutputDir;
	std::string mClassName;
	std::string mAdapterName;

	void generateAdapterByJinja(std::shared_ptr<ClassInfo>& classInfo);
	void generateAdapterHeaderByJinja(std::shared_ptr<ClassInfo>& classInfo);
	void generateAdapterCppByJinja(std::shared_ptr<ClassInfo>& classInfo);

	void generateServiceByJinja(std::shared_ptr<ClassInfo>& classInfo);
	void generateServiceHeaderByJinja(std::shared_ptr<ClassInfo>& classInfo);
	void generateServiceCppByJinja(std::shared_ptr<ClassInfo>& classInfo);

	void RenderFile(const std::string& input, const std::string& output, const jinja2::ValuesMap& map);

};
