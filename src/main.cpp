

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#ifdef __unix__
#include <limits.h>
#include <stdlib.h>
#endif

#include <iostream>
#include <string>
#include <type_traits>
#include "ClassInfo.h"
#include "FileGenerator.h"

#ifdef _MSC_VER
char* baseName(const char* path)
{
	char* base1 = const_cast<char*>(strrchr(path, '/'));
	char* base2 = const_cast<char*>(strrchr(path, '\\'));
	if (base1 && base2)
		return((base1 > base2) ? base1 + 1 : base2 + 1);
	else if (base1)
		return(base1 + 1);
	else if (base2)
		return(base2 + 1);

	return const_cast<char*>(path);
}

char* dirname(char* path)
{
	char* base1 = strrchr(path, '/');
	char* base2 = strrchr(path, '\\');
	if (base1 && base2)
		if (base1 > base2)
			*base1 = 0;
		else
			*base2 = 0;
	else if (base1)
		*base1 = 0;
	else if (base2)
		*base2 = 0;

	return path;
}
#else
extern char* baseName(const char*);
extern char* dirname(char*);
#endif

std::string getCursorSpelling(CXCursor cursor)
{
	const CXString cursorSpelling = clang_getCursorSpelling(cursor);
	std::string result = clang_getCString(cursorSpelling);

	clang_disposeString(cursorSpelling);
	return result;
}

/* Auxiliary function for resolving a (relative) path into an absolute path */
std::string resolvePath(const char* path)
{
	std::string resolvedPath;

#ifdef __unix__
	char* resolvedPathRaw = new char[PATH_MAX];
	char* result = realpath(path, resolvedPathRaw);

	if (result)
		resolvedPath = resolvedPathRaw;

	delete[] resolvedPathRaw;
#else
	resolvedPath = path;
#endif

	return resolvedPath;
}

void parseUsrString(const std::string& usrString, bool* isVolatile, bool* isConst, bool* isRestrict) {
	size_t bangLocation = usrString.find("#");
	if (bangLocation == std::string::npos || bangLocation == usrString.length() - 1) {
		*isVolatile = *isConst = *isRestrict = false;
		return;
	}
	bangLocation++;
	int x = usrString[bangLocation];

	*isConst = x & 0x1;
	*isVolatile = x & 0x4;
	*isRestrict = x & 0x2;
}

static const char* GetCursorSource(CXCursor Cursor) {
	const CXSourceLocation Loc = clang_getCursorLocation(Cursor);
	CXFile file;
	clang_getExpansionLocation(Loc, &file, nullptr, nullptr, nullptr);
	const CXString source = clang_getFileName(file);
	if (!clang_getCString(source)) {
		clang_disposeString(source);
		return "<invalid loc>";
	}
	else {
		const char* b = baseName(clang_getCString(source));
		clang_disposeString(source);
		return b;
	}
}

static bool isForwardDeclaration(CXCursor cursor)
{
	const auto definition = clang_getCursorDefinition(cursor);

	// If the definition is null, then there is no definition in this translation
	// unit, so this cursor must be a forward declaration.
	if (clang_equalCursors(definition, clang_getNullCursor()))
		return true;

	// If there is a definition, then the forward declaration and the definition
	// are in the same translation unit. This cursor is the forward declaration if
	// it is _not_ the definition.
	return !clang_equalCursors(cursor, definition);
}
static std::shared_ptr<ClassInfo> classInfo{nullptr};

CXChildVisitResult Parser(CXCursor cursor, CXCursor  parent , CXClientData /* clientData */)
{
	if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
		return CXChildVisit_Continue;

	const CXCursorKind kind = clang_getCursorKind(cursor);
	const auto name = getCursorSpelling(cursor);
	const CXType type = clang_getCursorType(cursor);


	if (kind == CXCursorKind::CXCursor_ClassDecl && !isForwardDeclaration(cursor)) {
		classInfo = std::make_shared<ClassInfo>();
		classInfo->className = name;
		printf("class name is:%s\n",name.c_str());
		if (clang_getCursorKind(parent) == CXCursorKind::CXCursor_Namespace)
		{
			const auto ns = getCursorSpelling(parent);
			printf("name space is:%s\n", ns.c_str());
			classInfo->classNameSpace = ns;
		}
	}
	else if( kind == CXCursorKind::CXCursor_Namespace)
	{
		printf("name space is:%s\n", name.c_str());
	}
	else  if (kind == CXCursorKind::CXCursor_CXXBaseSpecifier) {
		const enum CX_CXXAccessSpecifier access = clang_getCXXAccessSpecifier(cursor);
		const unsigned isVirtual = clang_isVirtualBase(cursor);
		const char* accessStr = nullptr;

		switch (access) {
		case CX_CXXInvalidAccessSpecifier:
			accessStr = "invalid"; break;
		case CX_CXXPublic:
			accessStr = "public"; break;
		case CX_CXXProtected:
			accessStr = "protected"; break;
		case CX_CXXPrivate:
			accessStr = "private"; break;
		}

		printf(" [name=%s access=%s isVirtual=%s]\n", name.c_str(),accessStr,
			isVirtual ? "true" : "false");
		printf(" [filelocation=%s]\n", GetCursorSource(cursor));
		const CXCursorVisitor visitor = [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult
		{
			return CXChildVisit_Continue;
		};
		
		clang_visitChildren(cursor, visitor, nullptr);

	}
	else if(kind == CXCursorKind::CXCursor_FunctionDecl)
	{
		/* Collect the template parameter kinds from the base template. */
		const int NumTemplateArgs = clang_Cursor_getNumTemplateArguments(cursor);
		if (NumTemplateArgs < 0) {
			printf(" [no template arg info]");
		}
		for (int i = 0; i < NumTemplateArgs; i++) {
			const enum CXTemplateArgumentKind TAK =
				clang_Cursor_getTemplateArgumentKind(cursor, i);
			switch (TAK) {
			case CXTemplateArgumentKind_Type:
			{
				const CXType T = clang_Cursor_getTemplateArgumentType(cursor, i);
				const CXString S = clang_getTypeSpelling(T);
				printf(" [Template arg %d: kind: %d, type: %s]",
					i, TAK, clang_getCString(S));
				clang_disposeString(S);
			}
			break;
			case CXTemplateArgumentKind_Integral:
				printf(" [Template arg %d: kind: %d, intval: %lld]",
					i, TAK, clang_Cursor_getTemplateArgumentValue(cursor, i));
				break;
			default:
				printf(" [Template arg %d: kind: %d]\n", i, TAK);
			}
		}
	}
	else if (kind == CXCursorKind::CXCursor_CXXMethod)
	{
		const char* function_name = clang_getCString(clang_getCursorSpelling(cursor));
		const char* return_type = clang_getCString(clang_getTypeSpelling(clang_getResultType(type)));
		printf("%s,%s(", return_type, function_name);

		MethodInfo method{};
		method.methodName = function_name;
		method.methodReturnType = return_type;
		/*
		CXString usr = clang_getCursorUSR(cursor);
		const char* usr_string = clang_getCString(usr);
		std::cout << usr_string << "\n";
		bool isVolatile, isConst, isRestrict;
		parseUsrString(usr_string, &isVolatile, &isConst, &isRestrict);
		printf("restrict, volatile, const: %d %d %d\n", isRestrict, isVolatile, isConst);
		clang_disposeString(usr);
		*/
		//if (isConst)
		method.isConst = clang_CXXMethod_isConst(cursor);
		method.isVirtual = clang_CXXMethod_isVirtual(cursor);
		const int num_args = clang_Cursor_getNumArguments(cursor);
		for (int i = 0; i < num_args - 1; ++i) {
			const char* arg_data_type = clang_getCString(clang_getTypeSpelling(clang_getArgType(type, i)));
			printf("%s,", arg_data_type);
			if (clang_CXXMethod_isVirtual(cursor))
			{
				method.methodArgs.emplace_back(arg_data_type);
			}
		}
		if (num_args > 0)
		{
			const char* arg_data_type = clang_getCString(clang_getTypeSpelling(clang_getArgType(type, num_args - 1)));
			printf("%s", arg_data_type);
			if (clang_CXXMethod_isVirtual(cursor))
			{
				method.methodArgs.emplace_back(arg_data_type);
			}
		}
		classInfo->methodList.emplace_back(method);
		printf(")\n");

	}
	else if ( kind == CXCursorKind::CXCursor_FunctionTemplate)
	{
		const CXSourceRange extent = clang_getCursorExtent(cursor);
		const CXSourceLocation startLocation = clang_getRangeStart(extent);
		const CXSourceLocation endLocation = clang_getRangeEnd(extent);

		unsigned int startLine = 0, startColumn = 0;
		unsigned int endLine = 0, endColumn = 0;

		clang_getSpellingLocation(startLocation, nullptr, &startLine, &startColumn, nullptr);
		clang_getSpellingLocation(endLocation, nullptr, &endLine, &endColumn, nullptr);

		std::cout << "  " << name << ": " << endLine - startLine << "\n";
	}

	else if (kind == CXCursor_ParmDecl)
	{
		CXCursorKind parentKind = clang_getCursorKind(parent);
		const auto type = clang_getCursorType(cursor);
		auto defType = clang_getTypeSpelling(type);
		std::cout << "  " << name << ":  \n";

	}

	return CXChildVisit_Recurse;
}



int main(int argc, char** argv)
{
	if (argc < 2)
		return -1;

	const auto resolvedPath = resolvePath(argv[1]);
	std::cerr << "Parsing " << resolvedPath << "...\n";

	const CXIndex index = clang_createIndex(0, 1);

	constexpr const char* defaultArguments[] = {
		"-x", "c++",
		"-std=c++17",
		"-IC:\\project\\libclang\\lib\\clang\\13.0.0\\include",
		"-IC:\\project\\common\\src",
		"-IC:\\project\\common\\vendors\\glog\\win\\x64\\Release\\include",
		"-IC:\\project\\common\\vendors\\protobuf\\include"
	};


	const CXTranslationUnit translationUnit = clang_parseTranslationUnit(index,
	                                                                     resolvedPath.c_str(),
	                                                                     defaultArguments,
	                                                                     std::extent_v<decltype(defaultArguments)>,
	                                                                     nullptr,
	                                                                     0,
	                                                                     CXTranslationUnit_None);

	const CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);
	clang_visitChildren(rootCursor, Parser, nullptr);

	clang_disposeTranslationUnit(translationUnit);
	clang_disposeIndex(index);
	FileGenerator generator;
	generator.setFilePath(R"delimiter(C:\project\common\unittests\mock\services)delimiter");
	generator.generateFile(classInfo);
	return 0;
}