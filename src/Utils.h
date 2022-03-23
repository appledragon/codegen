#pragma once
#include <string>

class Utils
{
public:
    static std::string cXStringToStdString(const CXString& text)
    {
        std::string result;

        if (!text.data)
            return result;

        result = std::string(clang_getCString(text));
        clang_disposeString(text);
        return result;
    }

    static std::string cXFileToStdString(const CXFile& file)
    {
        return cXStringToStdString(clang_getFileName(file));
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
        if (char * base2 = const_cast<char*>(strrchr(path, '\\')); base1 && base2)
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
        if (char * base2 = strrchr(path, '\\'); base1 && base2)
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
#include <libgen.h>
    static char* baseName(const char* path)
    {
        return basename(const_cast<char*>(path));
    }
#endif

    static std::string getCursorSource(const CXCursor& cursor)
    {
        const CXSourceLocation Loc = clang_getCursorLocation(cursor);
        CXFile file;
        clang_getExpansionLocation(Loc, &file, nullptr, nullptr, nullptr);
        if (const CXString source = clang_getFileName(file); !clang_getCString(source)) {
            clang_disposeString(source);
            return "<invalid loc>";
        } else {
            const char* b = baseName(clang_getCString(source));
            clang_disposeString(source);
            return b;
        }
    }

    static std::string getCursorTypeString(const CXType& type)
    {
        return cXStringToStdString(clang_getTypeSpelling(type));
    }

    static std::string getCursorTypeString(const CXCursor& cursor)
    {
        return cXStringToStdString(clang_getTypeSpelling(clang_getCursorType(cursor)));
    }

    static std::string getCursorNameString(const CXCursor& cursor)
    {
        return cXStringToStdString(clang_getCursorSpelling(cursor));
    }

    static bool isForwardDeclaration(const CXCursor& cursor)
    {
        const auto definition = clang_getCursorDefinition(cursor);

        if (clang_equalCursors(definition, clang_getNullCursor()))
            return true;

        return !clang_equalCursors(cursor, definition);
    }

    using sourceFileName = std::string;
    using sourceFileFullPath = std::string;

    static std::pair<sourceFileName, sourceFileFullPath> getCursorSourceLocation(const CXCursor& cursor)
    {
        const auto referenced = clang_getCursorReferenced(cursor);
        const auto fileName = getCursorSource(referenced);

        const CXSourceRange range = clang_getCursorExtent(referenced);
        const CXSourceLocation location = clang_getRangeStart(range);
        CXFile file;
        unsigned line, column, offset;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        const auto fileLocation = cXFileToStdString(file);
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
};
