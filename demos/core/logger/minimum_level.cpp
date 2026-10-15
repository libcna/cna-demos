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
        CNA::Logger::Experiment("experiment");
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
        const auto throughTrace = LogEveryLevel();
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::EXPERIMENT);
        const auto throughExperiment = LogEveryLevel();
        ctx.Print("minimum WARN: {} of 7 messages, TRACE: {} of 7, EXPERIMENT: {} of 7",
                  warnAndUp.size(), throughTrace.size(), throughExperiment.size());
        ctx.Check(warnAndUp.size() == 3, "WARN keeps fatal, error and warn");
        ctx.Check(throughTrace.size() == 6, "TRACE keeps the six production severities");
        ctx.Check(throughExperiment.size() == 7, "EXPERIMENT explicitly enables the experimental severity");

        // The *If forms log only when their condition holds -- no if-statement around the call.
        std::vector<std::string> seen;
        CNA::Logger::SetSink([&seen](CNA::LogLevel, CNA::LogCategory, std::string_view text) { seen.emplace_back(text); });
        const int framesBehind = 4;
        CNA::Logger::WarnIf("running slowly", framesBehind > 2);
        CNA::Logger::WarnIf("out of memory", false);
        CNA::Logger::ResetSink();
        ctx.Check(seen.size() == 1 && seen[0].find("running slowly") != std::string::npos,
                  "WarnIf logged only the message whose condition was true");

        // The general route accepts both a category and a condition. The severity-specific
        // ExperimentIf wrapper uses the default Application category, like the other *If forms.
        struct Delivery
        {
            CNA::LogLevel level;
            CNA::LogCategory category;
            std::string text;
        };
        std::vector<Delivery> deliveries;
        CNA::Logger::SetSink([&deliveries](CNA::LogLevel level, CNA::LogCategory category,
                                           std::string_view text) {
            deliveries.push_back({level, category, std::string(text)});
        });
        CNA::Logger::Log(CNA::LogLevel::DEBUG, "suppressed general route", CNA::LogCategory::INPUT, false);
        CNA::Logger::Log(CNA::LogLevel::DEBUG, "controller connected", CNA::LogCategory::INPUT, true);
        CNA::Logger::ExperimentIf("new renderer path", true);
        CNA::Logger::ExperimentIf("hidden experiment", false);
        CNA::Logger::ResetSink();
        ctx.Check(deliveries.size() == 2 && deliveries[0].level == CNA::LogLevel::DEBUG &&
                      deliveries[0].category == CNA::LogCategory::INPUT &&
                      deliveries[0].text.find("controller connected") != std::string::npos,
                  "the general Log route delivers its explicit level and category only when enabled");
        ctx.Check(deliveries.size() == 2 && deliveries[1].level == CNA::LogLevel::EXPERIMENT &&
                      deliveries[1].category == CNA::LogCategory::APPLICATION &&
                      deliveries[1].text.find("new renderer path") != std::string::npos,
                  "ExperimentIf uses the experimental severity and default Application category");

        CNA::Logger::SetMinimumLevel(previous);
        ctx.Check(CNA::Logger::GetMinimumLevel() == previous, "the previous minimum level is restored");
    }
}

CNA_DEMOS_CONSOLE(MinimumLevel, {
    .id = "core.logger.minimum_level",
    .title = "Log levels",
    .description = "Compares WARN, TRACE and EXPERIMENT thresholds, uses conditional severity wrappers, and sends "
                   "an explicitly categorized conditional message through the general Logger::Log route.",
    .apis = {"CNA::Logger::SetMinimumLevel", "CNA::Logger::GetMinimumLevel", "CNA::Logger::Fatal",
             "CNA::Logger::Debug", "CNA::Logger::Trace", "CNA::Logger::Experiment",
             "CNA::Logger::Log", "CNA::Logger::WarnIf", "CNA::Logger::ExperimentIf",
             "CNA::LogLevel", "CNA::LogCategory"},
    .tags = {"logging", "diagnostics", "log", "level", "cnaext"},
})
