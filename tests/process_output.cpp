// SPDX-License-Identifier: MS-PL
// The parent and child inherit a pipe. Buffered parent text must reach it before child text.
#include "CnaDemos/Process.hpp"
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string_view>

int main(int argc, char** argv)
{
    if (argc > 1 && std::string_view(argv[1]) == "--child")
    {
        std::fputs("CHILD\n", stdout);
        return 0;
    }
    std::ios::sync_with_stdio(false);
    static char buffer[4096];
    if (std::setvbuf(stdout, buffer, _IOFBF, sizeof(buffer)) != 0) return 2;
    std::cout << "PARENT_CXX\n";
    std::fputs("PARENT_C\n", stdout);
    return CnaDemos::RunProcess(std::filesystem::absolute(argv[0]), {"--child"});
}
