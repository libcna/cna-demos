// SPDX-License-Identifier: MS-PL
// CNA::Logger::SetMinimumLevel and the conditional *If calls: deciding what is logged.
#include <string>
#include <string_view>
#include <vector>

#include "CNA/LogCategory.hpp"
#include "CNA/LogLevel.hpp"
#include "CNA/Logger.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    std::vector<std::string> LogEveryLevel()
    {
        std::vector<std::string> seen;
        CNA::Logger::SetSink([&seen](CNA::LogLevel, CNA::LogCategory, std::string_view text) {
            seen.emplace_back(text);
        });
        CNA::Logger::Fatal("fatal");
        CNA::Logger::Error("error");
        CNA::Logger::Warn("warn");
        CNA::Logger::Info("info");
        CNA::Logger::Debug("debug");
        CNA::Logger::Trace("trace");
        CNA::Logger::ResetSink();
        return seen;
    }

    void MinimumLevel(CnaDemos::DemoContext& ctx)
    {
        const CNA::LogLevel previous = CNA::Logger::GetMinimumLevel();

        // Levels are ordered FATAL < ERROR < WARN < INFO < DEBUG < TRACE: the minimum level keeps
        // itself and everything more severe.
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::WARN);
        const auto warnAndUp = LogEveryLevel();
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::TRACE);
        const auto everything = LogEveryLevel();
        ctx.Print("minimum WARN: {} of 6 messages, minimum TRACE: {} of 6", warnAndUp.size(), everything.size());
        ctx.Check(warnAndUp.size() == 3, "WARN keeps fatal, error and warn");
        ctx.Check(everything.size() == 6, "TRACE keeps everything");

        // The *If forms log only when their condition holds -- no if-statement around the call.
        std::vector<std::string> seen;
        CNA::Logger::SetSink([&seen](CNA::LogLevel, CNA::LogCategory, std::string_view text) { seen.emplace_back(text); });
        const int framesBehind = 4;
        CNA::Logger::WarnIf("running slowly", framesBehind > 2);
        CNA::Logger::WarnIf("out of memory", false);
        CNA::Logger::ResetSink();
        ctx.Check(seen.size() == 1 && seen[0].find("running slowly") != std::string::npos,
                  "WarnIf logged only the message whose condition was true");

        CNA::Logger::SetMinimumLevel(previous);
        ctx.Check(CNA::Logger::GetMinimumLevel() == previous, "the previous minimum level is restored");
    }
}

CNA_DEMOS_CONSOLE(MinimumLevel, {
    .id = "core.logger.minimum_level",
    .title = "Log levels",
    .description = "Logs one message at each level under CNA::Logger::SetMinimumLevel WARN and TRACE to show which "
                   "pass, and uses WarnIf to log only when a condition holds; the previous level is restored.",
    .apis = {"CNA::Logger::SetMinimumLevel", "CNA::Logger::GetMinimumLevel", "CNA::Logger::Fatal",
             "CNA::Logger::Debug", "CNA::Logger::Trace", "CNA::Logger::WarnIf", "CNA::LogLevel"},
    .tags = {"logging", "diagnostics", "log", "level", "cnaext"},
})
