// SPDX-License-Identifier: MS-PL
// Reading "/key:value" launch arguments with LaunchParameters.
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/LaunchParameters.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void LaunchParametersParse(CnaDemos::DemoContext& ctx)
    {
        // A Game parses its own command line into Game.LaunchParameters. Here the arguments are
        // given explicitly (CNA's constructor taking an argument list) to show the format.
        const std::vector<std::string> arguments = {"/level:3", "-difficulty:hard", "--windowed", "/level:9"};
        const LaunchParameters parameters(arguments);

        for (const auto& [key, value] : parameters)
        {
            ctx.Print("{} = {}", key, value);
        }

        // "/key:value" and "-key:value" are accepted; arguments without a value are ignored, and
        // the first occurrence of a key wins.
        ctx.Check(parameters.ContainsKey("level") && parameters.at("level") == "3", "level = 3 (first occurrence)");
        ctx.Check(parameters.at("difficulty") == "hard", "a leading '-' works like '/'");
        ctx.Check(!parameters.ContainsKey("windowed"), "an argument without ':value' is not a parameter");
        ctx.Check(parameters.size() == 2, "two parameters were parsed");
    }
}

CNA_DEMOS_CONSOLE(LaunchParametersParse, {
    .id = "game.launch_parameters.parse",
    .title = "Launch parameters",
    .description = "Parses /key:value command-line arguments with LaunchParameters, the dictionary a Game exposes as "
                   "Game.LaunchParameters.",
    .apis = {"LaunchParameters::LaunchParameters", "LaunchParameters::ContainsKey", "Game::LaunchParameters"},
    .tags = {"command-line", "arguments"},
})
