#pragma once
#include <string>
#include <libgen.h>

class Utils
{
public:
    static  std::string CXStringToString(const CXString &text)
    {
        std::string result;

        if (!text.data)
            return result;

        result = std::string(clang_getCString(text));
        clang_disposeString(text);
        return result;
    }

    static std::string CXFileToFilepath(const CXFile &file)
    {
        return CXStringToString(clang_getFileName(file));
    }

    static  std::string getCursorSpelling(const CXCursor &cursor)
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
        char* base2 = const_cast<char*>(strrchr(path, '\\'));
        if (base1 && base2)
            return ((base1 > base2) ? base1 + 1 : base2 + 1);
        else if (base1)
            return (base1 + 1);
        else if (base2)
            return (base2 + 1);

        return const_cast<char*>(path);
    }

    static char* dirname(char* path)
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
    static char* baseName(const char* path)
    {
        return basename(const_cast<char*>(path));
    }
#endif

   static std::string getCursorSource(const CXCursor &Cursor)
    {
        const CXSourceLocation Loc = clang_getCursorLocation(Cursor);
        CXFile file;
        clang_getExpansionLocation(Loc, &file, nullptr, nullptr, nullptr);
        const CXString source = clang_getFileName(file);
        if (!clang_getCString(source)) {
            clang_disposeString(source);
            return "<invalid loc>";
        } else {
            const char* b = baseName(clang_getCString(source));
            clang_disposeString(source);
            return b;
        }
    }

    static bool isForwardDeclaration(const CXCursor &cursor)
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
    
};
