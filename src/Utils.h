#pragma once

#include <string>
#ifdef __APPLE__
#include <libgen.h>
#endif

#include <clang-c/Index.h>
#include <cstdint>
#include <variant>

#include <map>
#include <memory>
#include <cstring>


class Utils
{
public:
    using DefaultValueType = std::variant<bool, int, uint64_t, uint32_t, double, float, char, std::string>;
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

    static std::string getCursorSpelling(const CXCursor& cursor)
    {
        const CXString cursorSpelling = clang_getCursorSpelling(cursor);
        std::string result = clang_getCString(cursorSpelling);

        clang_disposeString(cursorSpelling);
        return result;
    }

#ifdef _MSC_VER
    static char* baseName(const char* path)
    {
        char* base1 = const_cast<char*>(strrchr(path, '/'));
        if (char* base2 = const_cast<char*>(strrchr(path, '\\')); base1 && base2)
            return ((base1 > base2) ? base1 + 1 : base2 + 1);
        else {
            if (base1)
                return (base1 + 1);
            if (base2)
                return (base2 + 1);
        }

        return const_cast<char*>(path);
    }

    static char* dirname(char* path)
    {
        char* base1 = strrchr(path, '/');
        if (char* base2 = strrchr(path, '\\'); base1 && base2)
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
    static char* baseName(const char* path)
    {
        return (const_cast<char*>(path));
    }
#endif

    static std::string getCursorSource(const CXCursor& cursor)
    {
        const CXSourceLocation Loc = clang_getCursorLocation(cursor);
        CXFile file = nullptr;
        clang_getExpansionLocation(Loc, &file, nullptr, nullptr, nullptr);
        const CXString source = clang_getFileName(file);
        if (clang_getCString(source) == nullptr) {
            clang_disposeString(source);
            return "<invalid loc>";
        }
        const char* b = baseName(clang_getCString(source));
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

    static void EvaluateDefaultValue(const CXCursor& cursor, DefaultValueType& output)
    {
        CXType ctype = clang_getCursorType(cursor);

        CXEvalResult res = clang_Cursor_Evaluate(cursor);
        CXEvalResultKind kind = clang_EvalResult_getKind(res);
        switch (kind) {
            case CXEval_Int: {
                const auto dataLength = clang_Type_getSizeOf(ctype);
                if (dataLength == 1) {
                    // bool
                    clang_EvalResult_getAsInt(res) == 0 ? output = false : output = true;
                }
                else if (dataLength <= sizeof(int)) {
                    output = clang_EvalResult_isUnsignedInt(res);
                } else {
                    output = static_cast<uint64_t>(clang_EvalResult_getAsUnsigned(res));
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
            default: {
                const char* val = clang_EvalResult_getAsStr(res);

                if (val != nullptr) {
                    output = clang_EvalResult_getAsStr(res);
                }
            } break;
        }
        clang_EvalResult_dispose(res);
    }

    static bool hasDefaultValue(const CXCursorKind& kind, const CXCursor& cursor, DefaultValueType& output)
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
                EvaluateDefaultValue(cursor, output);
                return true;
            default:
                return false;
        }
        return false;
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
                    size_t size = strlen(param);
                    char* value_ptr = delimiter + 1;
                    *delimiter = 0;
                    key = param;
                    if (value_ptr - param < size) {
                        value = value_ptr;
                    }
                } else {
                    key = std::string(param);
                }
                map_opts.insert(std::make_pair(key, value));
            }
        }
    }

    static void readFileAllContents(const char* path, std::string& content)
    {
        if (nullptr == path)
            return;
        FILE* p_file = fopen(path, "r");
        if (nullptr != p_file) {
            fseek(p_file, 0, SEEK_END);
            size_t size = ftell(p_file);
            fseek(p_file, 0, SEEK_SET);
            if (0 != size) {
                content.resize(size);
                fread((void*)content.data(), 1, size, p_file);
            }
            fclose(p_file);
        }
    }
};
