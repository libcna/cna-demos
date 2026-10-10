// SPDX-License-Identifier: MS-PL
#pragma once

#include "CnaDemos/DemoContext.hpp"

namespace CnaDemos::App
{
    /** Parses the command line and runs the requested command. Returns the process exit code. */
    int Main(int argc, char** argv);

    /** The interactive terminal browser (Category -> Subcategory -> Demo -> Run). */
    int RunBrowser(const RunOptions& options);

    /** Exit code for invalid command-line usage. */
    inline constexpr int UsageExitCode = 2;
}
