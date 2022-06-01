#include <iostream>
#include <string>

int clangJinjaRenderMain(int argc, char** argv);
int clangJsonRenderMain(int argc, char** argv);
int main(int argc, char** argv)
{
#if (defined AST_DUMP_JSON)
    return clangJsonRenderMain(argc, argv);
#else
    return clangJinjaRenderMain(argc, argv);
#endif
}
