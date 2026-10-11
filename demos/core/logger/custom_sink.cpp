// SPDX-License-Identifier: MS-PL
// CNA::Logger::SetSink: sending CNA's log (and the game's own messages) somewhere of your choice.
#include <string>
#include <string_view>
#include <vector>

#include "CNA/LogCategory.hpp"
#include "CNA/LogLevel.hpp"
#include "CNA/Logger.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    struct Entry
    {
        CNA::LogLevel level;
        CNA::LogCategory category;
        std::string text;
    };

    void CustomSink(CnaDemos::DemoContext& ctx)
    {
        // The logger is process-wide: remember the level to put it back afterwards.
        const CNA::LogLevel previous = CNA::Logger::GetMinimumLevel();
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::INFO);

        // A sink receives every message that passes the level filter, already formatted. A game
        // might write it to a file, an in-game console or a crash report.
        std::vector<Entry> captured;
        CNA::Logger::SetSink([&captured](CNA::LogLevel level, CNA::LogCategory category, std::string_view text) {
            captured.push_back({level, category, std::string(text)});
        });

        CNA::Logger::Info("level 3 loaded");
        CNA::Logger::Warn("texture missing, using a placeholder", CNA::LogCategory::RENDER);
        CNA::Logger::Error("save failed", CNA::LogCategory::SYSTEM);

        // Back to CNA's default output (the console) before anything else logs.
        CNA::Logger::ResetSink();
        CNA::Logger::SetMinimumLevel(previous);

        for (const Entry& entry : captured)
        {
            ctx.Print("sink got: {}", entry.text);
        }
        ctx.Check(captured.size() == 3, "the sink received the three messages");
        ctx.Check(captured.size() == 3 && captured[1].level == CNA::LogLevel::WARN &&
                      captured[1].category == CNA::LogCategory::RENDER,
                  "each message comes with its level and category");
        ctx.Check(!captured.empty() && captured[0].text.find("level 3 loaded") != std::string::npos,
                  "the formatted text contains the message");
    }
}

CNA_DEMOS_CONSOLE(CustomSink, {
    .id = "core.logger.custom_sink",
    .title = "Capturing CNA's log",
    .description = "Installs a CNA::Logger sink that collects messages with their level and category, logs an info, "
                   "a warning (RENDER) and an error (SYSTEM), and restores the default console output with "
                   "ResetSink.",
    .apis = {"CNA::Logger::SetSink", "CNA::Logger::ResetSink", "CNA::Logger::Info", "CNA::Logger::Warn",
             "CNA::Logger::Error", "CNA::LogLevel", "CNA::LogCategory"},
    .tags = {"logging", "diagnostics", "log", "sink", "cnaext"},
})
