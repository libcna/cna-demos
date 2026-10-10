// SPDX-License-Identifier: MS-PL
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace CnaDemos
{
    /** Directory holding the cna-demos executable (and the standalone demos and assets/). */
    [[nodiscard]] std::filesystem::path ExecutableDirectory();

    /**
     * Runs a program and waits for it. Returns its exit code, or -1 when it could not be
     * started or did not exit normally. Arguments are quoted for the platform shell.
     */
    int RunProcess(const std::filesystem::path& program, const std::vector<std::string>& arguments);
}
