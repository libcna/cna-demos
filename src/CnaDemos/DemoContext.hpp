// SPDX-License-Identifier: MS-PL
#pragma once

#include <cstdint>
#include <format>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CnaDemos
{
    struct DemoEntry;

    /** Thrown (via DemoContext::Skip) when the environment lacks something the demo needs. */
    class SkipDemo : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    /** Options of one demo run, decided by the command line or the browser. */
    struct RunOptions
    {
        /** Unattended: bounded frames, no waiting for input. */
        bool automated = false;
        /** Frames to draw before exiting; -1 = metadata/default when automated, unlimited otherwise. */
        int frames = -1;
        /** Seed for any randomness a demo uses. */
        std::uint32_t seed = 12345;
        /** When set, the last drawn frame is saved here as PNG (Game demos only). */
        std::string screenshotPath;
        /** Suppress the demo's own informational output. */
        bool quiet = false;
        /**
         * This process was started to run this one demo (`cna-demos <id>`), so a demo whose
         * metadata asks for its own process (DemoInfo::ownProcess) runs here directly.
         */
        bool freshProcess = false;
    };

    /**
     * What a running demo can ask the framework for: options, an output stream, a seed, and a
     * place to record checks. Console demos receive it as their argument; Game demos reach it
     * through DemoGame::Context().
     */
    class DemoContext
    {
    public:
        DemoContext(const DemoEntry& entry, RunOptions options, std::ostream& out);

        DemoContext(const DemoContext&) = delete;
        DemoContext& operator=(const DemoContext&) = delete;

        [[nodiscard]] const DemoEntry& Entry() const { return entry_; }
        [[nodiscard]] const RunOptions& Options() const { return options_; }
        [[nodiscard]] bool Automated() const { return options_.automated; }
        [[nodiscard]] std::uint32_t Seed() const { return options_.seed; }

        /** Frames to draw before exiting, or 0 for "until the user closes the demo". */
        [[nodiscard]] int FrameLimit() const;

        /** The demo's output stream (a null sink with --quiet). */
        [[nodiscard]] std::ostream& Out();

        /** Formats one line of output: ctx.Print("Length = {}", v.Length()). */
        template <class... Args>
        void Print(std::format_string<Args...> format, Args&&... args)
        {
            Out() << std::format(format, std::forward<Args>(args)...) << '\n';
        }

        /**
         * Records a check. A failed check makes the run FAILED (exit code 1) but does not stop
         * the demo, so every failing expectation is reported, not only the first.
         */
        bool Check(bool condition, std::string_view expectation);

        /**
         * Records a check that `actual` is within `tolerance` of `expected` (floating-point
         * results). The message is extended with both values when the check fails.
         */
        bool CheckNear(double actual, double expected, double tolerance, std::string_view expectation);

        /** Records a failure with an explanation. */
        void Fail(std::string_view reason);

        /** Ends the demo as SKIPPED: the environment lacks a documented requirement. */
        [[noreturn]] void Skip(std::string_view reason);

        [[nodiscard]] bool Failed() const { return !failures_.empty(); }
        [[nodiscard]] int CheckCount() const { return checks_; }
        [[nodiscard]] const std::vector<std::string>& Failures() const { return failures_; }

        /** The context of the demo currently running on this thread (set by the runner). */
        [[nodiscard]] static DemoContext& Current();

    private:
        friend class CurrentContextScope;

        const DemoEntry& entry_;
        RunOptions options_;
        std::ostream& out_;
        int checks_ = 0;
        std::vector<std::string> failures_;
    };

    /** Installs a context as DemoContext::Current() for the lifetime of the scope. */
    class CurrentContextScope
    {
    public:
        explicit CurrentContextScope(DemoContext& context);
        ~CurrentContextScope();

        CurrentContextScope(const CurrentContextScope&) = delete;
        CurrentContextScope& operator=(const CurrentContextScope&) = delete;

    private:
        DemoContext* previous_;
    };
}
