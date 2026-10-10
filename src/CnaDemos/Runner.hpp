// SPDX-License-Identifier: MS-PL
#pragma once

#include <iosfwd>
#include <span>
#include <string>
#include <vector>

#include "CnaDemos/DemoContext.hpp"
#include "CnaDemos/Registry.hpp"

namespace CnaDemos
{
    enum class RunStatus
    {
        Passed,
        Failed,
        Skipped,   ///< The environment lacks a documented requirement.
        NotRun     ///< Manual demo in an unattended batch, or standalone executable missing.
    };

    [[nodiscard]] std::string_view ToString(RunStatus status);

    /** Process exit code for a status: 0 passed, 1 failed, 77 skipped (the CTest convention). */
    [[nodiscard]] int ExitCode(RunStatus status);
    inline constexpr int SkipExitCode = 77;

    struct RunResult
    {
        std::string id;
        RunStatus status = RunStatus::NotRun;
        std::string message;
        int checks = 0;
        double seconds = 0.0;
    };

    /** Runs one demo in this process (or, for a standalone demo, as a child process). */
    RunResult RunDemo(const RegisteredDemo& demo, const RunOptions& options, std::ostream& out);

    /**
     * Runs several demos one after another and prints a one-line result for each and a
     * summary. Manual demos are listed as NOT RUN when `options.automated` is set.
     */
    std::vector<RunResult> RunBatch(std::span<const RegisteredDemo* const> demos,
                                    const RunOptions& options, std::ostream& out);

    /** Writes batch results as JSON (for CI artifacts and the coverage report). */
    void WriteResultsJson(std::span<const RunResult> results, std::ostream& out);
}
