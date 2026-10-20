// SPDX-License-Identifier: MS-PL
// Reading "/key:value" launch arguments with LaunchParameters.
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/LaunchParameters.hpp"
#include "System/ArgumentException.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void LaunchParametersParse(CnaDemos::DemoContext& ctx)
    {
        // A Game parses its own command line into Game.LaunchParameters. Here the arguments are
        // given explicitly (CNA's constructor taking an argument list) to show the format.
        const std::vector<std::string> arguments = {
            "/level:3", "-difficulty:hard", "--windowed", "/level:9", "plain:value",
            "/mixed:a:b", "-Case:Upper", "-case:lower", "/empty:"};
        const LaunchParameters parameters(arguments);

        for (const auto& [key, value] : parameters)
        {
            ctx.Print("{} = {}", key, value);
        }

        // XNA trims every leading slash/dash, splits only the first colon, preserves empty values,
        // compares keys ordinally and keeps the first occurrence.
        ctx.Check(parameters.ContainsKey("level") && parameters.at("level") == "3", "level = 3 (first occurrence)");
        ctx.Check(parameters.at("difficulty") == "hard" && parameters.at("mixed") == "a:b",
                  "all leading flags are trimmed and only the first colon separates the value");
        ctx.Check(parameters.at("plain") == "value" && parameters.at("Case") == "Upper" &&
                      parameters.at("case") == "lower",
                  "flagless key/value arguments work and keys remain case-sensitive");
        ctx.Check(parameters.ContainsKey("windowed") && parameters.at("windowed").empty() &&
                      parameters.ContainsKey("empty") && parameters.at("empty").empty() &&
                      parameters.size() == 8,
                  "XNA retains both valueless flags and explicit empty values");

        Game game;
        auto& gameParameters = game.getLaunchParametersProperty();
        const auto& constGameParameters = static_cast<const Game&>(game).getLaunchParametersProperty();
        gameParameters.Add("cna-demo-probe", "visible");
        ctx.Check(&gameParameters == &constGameParameters && constGameParameters.at("cna-demo-probe") == "visible",
                  "Game exposes mutable and const views of its live launch-parameter dictionary");

        LaunchParameters manual(std::vector<std::string>{});
        manual.Add("manual", "one");
        ctx.Check(manual.ContainsKey("manual") && manual.at("manual") == "one",
                  "Add inserts a new key/value pair");
        bool duplicateRejected = false;
        try
        {
            manual.Add("manual", "two");
        }
        catch (const System::ArgumentException&)
        {
            duplicateRejected = true;
        }
        ctx.Check(duplicateRejected && manual.at("manual") == "one",
                  "Add rejects a duplicate key and preserves the original value");
        manual.Add("", "empty-key");
        manual.Add("blank", "");
        ctx.Check(manual.at("") == "empty-key" && manual.at("blank").empty(),
                  "direct dictionary insertion accepts empty keys and values");
    }
}

CNA_DEMOS_CONSOLE(LaunchParametersParse, {
    .id = "game.launch_parameters.parse",
    .title = "Launch parameters",
    .description = "Parses flagged, flagless, duplicate, case-sensitive and empty launch arguments, then exercises "
                   "the dictionary Add contract exposed through Game.LaunchParameters.",
    .apis = {"LaunchParameters::LaunchParameters", "LaunchParameters::ContainsKey", "LaunchParameters::Add",
             "Game::LaunchParameters"},
    .tags = {"command-line", "arguments", "dictionary", "validation"},
})
