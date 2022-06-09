#pragma once

#include <string>
#ifdef __APPLE__
#include <libgen.h>
#endif
#include <clang-c/Index.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <variant>

#include "AccessSpecifier.h"
#include "json.hpp"
using Json = nlohmann::json;

//#define DEBUGLOG_ENABLE 
class Utils
{
public:
    using DefaultValueType = std::variant<bool, int, uint64_t, uint32_t, double, float, char, char*, std::string>;
    static std::string CXStringToStdString(const CXString& text)
    {
        std::string result;

        if (text.data == nullptr)
            return result;

        result = std::string(clang_getCString(text));
        clang_disposeString(text);
        return result;
    }

    static std::string CXFileToStdString(const CXFile& file)
    {
        return CXStringToStdString(clang_getFileName(file));
    }

    static std::string getCursorKindSpelling(const CXCursor& cursor)
    {
        const CXCursorKind kind = clang_getCursorKind(cursor);
        const CXString cursorKindSpelling = clang_getCursorKindSpelling(kind);
        std::string result = clang_getCString(cursorKindSpelling);

        clang_disposeString(cursorKindSpelling);
        return result;
    }

    static std::string getCursorSpelling(const CXCursor& cursor)
    {
        const CXString cursorSpelling = clang_getCursorSpelling(cursor);
        std::string result = clang_getCString(cursorSpelling);

        clang_disposeString(cursorSpelling);
        return result;
    }

    static bool isInSystemHeader(const CXCursor& cursor)
    {
        const auto location = clang_getCursorLocation(cursor);
        return clang_Location_isInSystemHeader(location) > 0;
    }

    static std::string getCursorSource(const CXCursor& cursor)
    {
        const CXSourceLocation Loc = clang_getCursorLocation(cursor);
        CXFile file = nullptr;
        clang_getExpansionLocation(Loc, &file, nullptr, nullptr, nullptr);
        const CXString source = clang_getFileName(file);
        if (clang_getCString(source) == nullptr) {
            clang_disposeString(source);
            return {};
        }
        const char* b = clang_getCString(source);
        clang_disposeString(source);
        return b;
    }

    static std::string getCursorTypeString(const CXType& type)
    {
        return CXStringToStdString(clang_getTypeSpelling(type));
    }

    static std::string getCursorTypeString(const CXCursor& cursor)
    {
        return CXStringToStdString(clang_getTypeSpelling(clang_getCursorType(cursor)));
    }

    static std::string getCursorNameString(const CXCursor& cursor)
    {
        return CXStringToStdString(clang_getCursorSpelling(cursor));
    }

    static bool isForwardDeclaration(const CXCursor& cursor)
    {
        const auto definition = clang_getCursorDefinition(cursor);

        if (clang_equalCursors(definition, clang_getNullCursor()) != 0U)
            return true;

        return clang_equalCursors(cursor, definition) == 0U;
    }

    using sourceFileName = std::string;
    using sourceFileFullPath = std::string;

    static std::pair<sourceFileName, sourceFileFullPath> getCursorSourceLocation(const CXCursor& cursor)
    {
        const auto referenced = clang_getCursorReferenced(cursor);
        const auto fileName = getCursorSource(referenced);

        const CXSourceRange range = clang_getCursorExtent(referenced);
        const CXSourceLocation location = clang_getRangeStart(range);
        CXFile file = nullptr;
        unsigned line = 0;
        unsigned column = 0;
        unsigned offset = 0;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        const auto fileLocation = CXFileToStdString(file);
        return {fileName, fileLocation};
    }

    static std::string getCursorUnderlyingTypeString(const CXCursor& cursor)
    {
        const auto referenced = clang_getCursorReferenced(cursor);
        const auto underlying = clang_getTypedefDeclUnderlyingType(referenced);
        return getCursorTypeString(underlying);
    }

    static bool isBuiltinType(const CXType& type)
    {
        return type.kind >= CXType_FirstBuiltin && type.kind <= CXType_LastBuiltin;
    }
    static std::string getCursorUSRString(const CXCursor& cursor)
    {
        const CXString spell = clang_getCursorUSR(cursor);
        std::string usr(clang_getCString(spell));
        clang_disposeString(spell);
        return usr;
    }

    static std::string getTypeUSRString(const CXType& type)
    {
        return getCursorUSRString(clang_getTypeDeclaration(type));
    }

    static std::string getCursorText(const CXCursor& cursor)
    {
        const CXSourceRange range = clang_getCursorExtent(cursor);
        const CXSourceLocation begin = clang_getRangeStart(range);
        const CXSourceLocation end = clang_getRangeEnd(range);
        CXFile cxFile = nullptr;
        unsigned int beginOff = 0;
        unsigned int endOff = 0;
        clang_getExpansionLocation(begin, &cxFile, nullptr, nullptr, &beginOff);
        clang_getExpansionLocation(end, nullptr, nullptr, nullptr, &endOff);
        const CXString filename = clang_getFileName(cxFile);
        unsigned int textSize = endOff - beginOff;

        std::ifstream file;
        file.open(CXStringToStdString(filename).c_str());
        if (file.good()) {
            file.seekg(beginOff, std::ios::beg);
            if (textSize > 4096) textSize = 4096;
            char buff[4096];
            file.read(buff, textSize);
            file.close();
            return {buff};
        }
        return {};
    }

    static void EvaluateDefaultValue(const CXCursor& cursor, DefaultValueType& output)
    {
        const CXType ctype = clang_getCursorType(cursor);
        /*
        // virtual void setchar(const char* name = "xxxxxxxxx") = 0;
        if (ctype.kind == CXType_ConstantArray) {
            const auto dataType = clang_getElementType(ctype);
            if (dataType.kind == CXType_Char_S) {
                // TODO need change, ugly way to read file.
                const auto value = getCursorText(cursor);
                output = const_cast<char*>(value.c_str());
            }
            return;
        }*/

        const auto dataLength = clang_Type_getSizeOf(ctype);

        const CXEvalResult res = clang_Cursor_Evaluate(cursor);
        switch (clang_EvalResult_getKind(res)) {
            case CXEval_Int: {
                if (dataLength == 1) {
                    if (ctype.kind == CXType_Char_S) {
                        // char
                        const auto ret = clang_EvalResult_getAsInt(res);
                        output = static_cast<char>(ret);
                    } else {
                        // bool
                        clang_EvalResult_getAsInt(res) == 0 ? output = false : output = true;
                    }
                } else if (dataLength <= sizeof(int)) {
                    output = clang_EvalResult_getAsInt(res);
                } else {
                    const auto ret = clang_EvalResult_getAsUnsigned(res);
                    output = static_cast<uint64_t>(ret);
                }
                break;
            }

            case CXEval_Float:
                output = clang_EvalResult_getAsDouble(res);
                break;
            case CXEval_ObjCStrLiteral:
            case CXEval_StrLiteral:
            case CXEval_CFStr:
            case CXEval_Other:
            case CXEval_UnExposed:
                break;
            default: {
                const char* val = clang_EvalResult_getAsStr(res);

                if (val != nullptr) {
                    output = clang_EvalResult_getAsStr(res);
                }
            } break;
        }
        clang_EvalResult_dispose(res);
    }

    static bool hasDefaultValue(const CXCursorKind& kind)
    {
        switch (kind) {
            case CXCursor_CharacterLiteral:
            case CXCursor_CompoundLiteralExpr:
            case CXCursor_FloatingLiteral:
            case CXCursor_ImaginaryLiteral:
            case CXCursor_IntegerLiteral:
            case CXCursor_StringLiteral:
            case CXCursor_CXXBoolLiteralExpr:
            case CXCursor_CXXNullPtrLiteralExpr:
                return true;
            default:
                return false;
        }
    }

    static void makeCommandLineOptsMap(int argc, char** argv, std::map<std::string, std::string>& map_opts)
    {
        for (int index = 0; index < argc; index++) {
            char* param = argv[index];
            if (nullptr != param) {
                std::string key;
                std::string value;
                char* delimiter = strchr(param, '=');
                if (nullptr != delimiter) {
                    const size_t size = strlen(param);
                    char* value_ptr = delimiter + 1;
                    *delimiter = 0;
                    key = param;
                    if (value_ptr - param < size) {
                        value = value_ptr;
                    }
                } else {
                    key = std::string(param);
                }
                map_opts.emplace(std::make_pair(key, value));
            }
        }
    }

    static void replaceAllSubString(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    static void readFileAllContents(const char* path, std::string& content)
    {
        if (nullptr == path)
            return;
        const std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
    }

    static AccessSpecifiers getCursorAccessSpecifier(const CXCursor& cursor)
    {
        switch (clang_getCXXAccessSpecifier(cursor)) {
            case CX_CXXInvalidAccessSpecifier:
                return AccessSpecifiers::UNKNOWN;
            case CX_CXXPublic:
                return AccessSpecifiers::PUBLIC;
            case CX_CXXProtected:
                return AccessSpecifiers::PROTECTED;
            case CX_CXXPrivate:
                return AccessSpecifiers::PRIVATE;
        }

        return AccessSpecifiers::UNKNOWN;
    }
};
