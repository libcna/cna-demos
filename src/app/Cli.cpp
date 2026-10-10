// SPDX-License-Identifier: MS-PL
#include "App.hpp"

#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "CNA/GraphicsRendererSelection.hpp"
#include "CNA/LogLevel.hpp"
#include "CNA/Logger.hpp"
#include "CNA/ProjectGraphicsProfile.hpp"
#include "CnaDemos/Catalog.hpp"
#include "CnaDemos/Process.hpp"
#include "CnaDemos/Registry.hpp"
#include "CnaDemos/Runner.hpp"

namespace CnaDemos::App
{
    namespace
    {
        constexpr std::string_view Usage = R"(CNA Demo Browser -- small executable demonstrations of CNA.

Usage:
  cna-demos                          Start the interactive demo browser.
  cna-demos <demo-id> [options]      Run one demo, e.g. cna-demos graphics.sprite_batch.draw_basic
  cna-demos --list [prefix]          List demos, optionally only those below a category/prefix.
  cna-demos --categories             List top-level categories with demo counts.
  cna-demos --search <text>          Search IDs, titles, descriptions, tags and APIs.
  cna-demos --info <demo-id>         Show everything known about one demo.
  cna-demos --run-category <prefix>  Run every demo below a category or ID prefix.
  cna-demos --run-tag <tag>          Run every demo carrying a tag.
  cna-demos --run-all                Run every demo, one after another.
  cna-demos --smoke                  Same as --run-all --automated.
  cna-demos --catalog-json           Print the machine-readable catalog (JSON).
  cna-demos --list-ids               Print "<id> <kind> <automation> <needs>" per demo (for scripts).
  cna-demos --validate               Check the registry: IDs, metadata, source paths, assets.
  cna-demos --renderers              List the renderers compiled into this build.
  cna-demos --version                Print versions.
  cna-demos --help                   Print this text.

Run options:
  --automated          Unattended run: bounded frames, never waits for input; manual demos
                       are listed as NOT RUN instead of being started.
  --frames <n>         Draw n frames, then exit (Game and standalone demos).
  --seed <n>           Seed for demos that use randomness (default 12345).
  --screenshot <file>  Save the last drawn frame as PNG (Game demos; combine with --frames).
  --renderer <name>    Use this renderer (equivalent to CNA_GRAPHICS_RENDERER=<name>).
  --quiet              Suppress the demos' own output (failures are still printed).
  --report <file>      Write the results of a run as JSON.
  --log-level <level>  CNA's own log output: fatal, error, warn (default), info, debug, trace.

Exit status: 0 passed, 1 failed, 2 usage error, 77 skipped (a documented requirement is
missing in this environment).
)";

        struct Arguments
        {
            std::string command;          // e.g. "--list"; empty = browser; "run" = run one demo
            std::string operand;          // demo ID, prefix, tag or search text
            RunOptions run;
            std::string renderer;
            std::string reportPath;
            std::string logLevel = "warn";
        };

        std::optional<int> ParseInt(std::string_view text)
        {
            int value = 0;
            const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (error != std::errc() || end != text.data() + text.size())
            {
                return std::nullopt;
            }
            return value;
        }

        // Returns an exit code on error, std::nullopt on success.
        std::optional<int> Parse(int argc, char** argv, Arguments& args)
        {
            static const std::map<std::string_view, bool> commands = {
                // command -> takes an operand
                {"--list", false},      {"--categories", false},   {"--search", true},
                {"--info", true},       {"--run-category", true},  {"--run-tag", true},
                {"--run-all", false},   {"--smoke", false},        {"--catalog-json", false},
                {"--list-ids", false},  {"--validate", false},     {"--renderers", false},
                {"--version", false},   {"--help", false},         {"-h", false},
            };

            for (int i = 1; i < argc; ++i)
            {
                const std::string_view arg = argv[i];
                const auto needValue = [&](std::string_view option) -> std::optional<std::string_view> {
                    if (i + 1 >= argc)
                    {
                        std::cerr << "cna-demos: " << option << " needs a value\n";
                        return std::nullopt;
                    }
                    return std::string_view(argv[++i]);
                };

                if (arg == "--automated")
                {
                    args.run.automated = true;
                }
                else if (arg == "--quiet")
                {
                    args.run.quiet = true;
                }
                else if (arg == "--frames" || arg == "--seed")
                {
                    const auto value = needValue(arg);
                    const auto number = value ? ParseInt(*value) : std::nullopt;
                    if (!number || *number < 0)
                    {
                        std::cerr << "cna-demos: " << arg << " needs a non-negative integer\n";
                        return UsageExitCode;
                    }
                    if (arg == "--frames")
                    {
                        args.run.frames = *number;
                    }
                    else
                    {
                        args.run.seed = static_cast<std::uint32_t>(*number);
                    }
                }
                else if (arg == "--screenshot" || arg == "--renderer" || arg == "--report" || arg == "--log-level")
                {
                    const auto value = needValue(arg);
                    if (!value)
                    {
                        return UsageExitCode;
                    }
                    (arg == "--screenshot" ? args.run.screenshotPath
                     : arg == "--renderer" ? args.renderer
                     : arg == "--report"   ? args.reportPath
                                           : args.logLevel) = std::string(*value);
                }
                else if (const auto command = commands.find(arg); command != commands.end())
                {
                    if (!args.command.empty())
                    {
                        std::cerr << "cna-demos: only one command may be given (" << args.command << ", "
                                  << arg << ")\n";
                        return UsageExitCode;
                    }
                    args.command = std::string(arg);
                    if (command->second)
                    {
                        const auto value = needValue(arg);
                        if (!value)
                        {
                            return UsageExitCode;
                        }
                        args.operand = std::string(*value);
                    }
                    else if (arg == "--list" && i + 1 < argc && argv[i + 1][0] != '-')
                    {
                        args.operand = argv[++i];
                    }
                }
                else if (!arg.empty() && arg.front() != '-' && args.command.empty())
                {
                    args.command = "run";
                    args.operand = std::string(arg);
                }
                else
                {
                    std::cerr << "cna-demos: unknown argument '" << arg << "' (see --help)\n";
                    return UsageExitCode;
                }
            }
            return std::nullopt;
        }

        std::optional<CNA::LogLevel> ParseLogLevel(std::string_view name)
        {
            static const std::map<std::string_view, CNA::LogLevel> levels = {
                {"fatal", CNA::LogLevel::FATAL}, {"error", CNA::LogLevel::ERROR}, {"warn", CNA::LogLevel::WARN},
                {"info", CNA::LogLevel::INFO},   {"debug", CNA::LogLevel::DEBUG}, {"trace", CNA::LogLevel::TRACE},
            };
            const auto level = levels.find(name);
            return level == levels.end() ? std::nullopt : std::optional(level->second);
        }

        void SetEnvironment(const char* name, const std::string& value)
        {
#ifdef _WIN32
            _putenv_s(name, value.c_str());
#else
            setenv(name, value.c_str(), 1);
#endif
        }

        void PrintList(const std::vector<const RegisteredDemo*>& demos)
        {
            std::size_t width = 0;
            for (const RegisteredDemo* demo : demos)
            {
                width = std::max(width, demo->entry.info.id.size());
            }
            for (const RegisteredDemo* demo : demos)
            {
                const auto& info = demo->entry.info;
                std::cout << info.id << std::string(width - info.id.size() + 2, ' ') << info.title << '\n';
            }
        }

        std::vector<const RegisteredDemo*> AllDemos()
        {
            std::vector<const RegisteredDemo*> demos;
            for (const RegisteredDemo& demo : Registry::Instance().All())
            {
                demos.push_back(&demo);
            }
            return demos;
        }

        int RunMany(const std::vector<const RegisteredDemo*>& demos, const Arguments& args)
        {
            if (demos.empty())
            {
                std::cerr << "cna-demos: no demos match\n";
                return 1;
            }
            const auto results = RunBatch(demos, args.run, std::cout);
            if (!args.reportPath.empty())
            {
                std::ofstream report(args.reportPath);
                WriteResultsJson(results, report);
            }
            for (const RunResult& result : results)
            {
                if (result.status == RunStatus::Failed)
                {
                    return 1;
                }
            }
            return 0;
        }

        int RunOne(const Arguments& args)
        {
            const RegisteredDemo* demo = Registry::Instance().Find(args.operand);
            if (demo == nullptr)
            {
                std::cerr << "cna-demos: no demo with ID '" << args.operand << "'\n";
                const auto similar = Registry::Instance().Search(args.operand);
                if (!similar.empty())
                {
                    std::cerr << "Did you mean:\n";
                    for (std::size_t i = 0; i < similar.size() && i < 8; ++i)
                    {
                        std::cerr << "  " << similar[i]->entry.info.id << '\n';
                    }
                }
                return UsageExitCode;
            }
            if (!args.run.quiet)
            {
                std::cout << "== " << demo->entry.info.id << ": " << demo->entry.info.title << '\n';
            }
            const RunResult result = RunDemo(*demo, args.run, std::cout);
            std::cout << "== " << ToString(result.status);
            if (!result.message.empty())
            {
                std::cout << ": " << result.message;
            }
            std::cout << '\n';
            if (!args.reportPath.empty())
            {
                std::ofstream report(args.reportPath);
                WriteResultsJson(std::span(&result, 1), report);
            }
            return ExitCode(result.status);
        }
    }

    int Main(int argc, char** argv)
    {
        Arguments args;
        if (const auto error = Parse(argc, argv, args))
        {
            return *error;
        }

        // CNA logs at TRACE level in debug builds; demo output stays readable unless asked for.
        const auto logLevel = ParseLogLevel(args.logLevel);
        if (!logLevel)
        {
            std::cerr << "cna-demos: unknown log level '" << args.logLevel << "' (see --help)\n";
            return UsageExitCode;
        }
        CNA::Logger::SetMinimumLevel(*logLevel);

        try
        {
            if (!args.renderer.empty())
            {
                // Must happen before any GraphicsDevice exists. Standalone demos inherit the choice
                // through CNA's own environment variable.
                CNA::GraphicsRendererSelection::SetPreferred(std::string_view(args.renderer));
                SetEnvironment("CNA_GRAPHICS_RENDERER", args.renderer);
            }
        }
        catch (const std::exception& exception)
        {
            std::cerr << "cna-demos: cannot select renderer '" << args.renderer << "': " << exception.what()
                      << "\nCompiled-in renderers:";
            for (const auto& name : CompiledRenderers())
            {
                std::cerr << ' ' << name;
            }
            std::cerr << '\n';
            return UsageExitCode;
        }

        // The project-wide profile, CNA's counterpart of an XNA project's "Graphics Profile"
        // setting. Demos run under HiDef so the whole API is available (back-buffer readback for
        // --screenshot among it); a demo about Reach selects Reach itself through
        // GraphicsDeviceManager::GraphicsProfile.
        CNA::SetProjectGraphicsProfileEXT(Microsoft::Xna::Framework::Graphics::GraphicsProfile::HiDef);

        Registry& registry = Registry::Instance();
        const std::string& command = args.command;

        if (command.empty())
        {
            return RunBrowser(args.run);
        }
        if (command == "--help" || command == "-h")
        {
            std::cout << Usage;
            return 0;
        }
        if (command == "--version")
        {
            std::cout << "cna-demos " << CnaDemosVersion() << " (CNA " << CnaVersion() << ")\n";
            return 0;
        }
        if (command == "--list")
        {
            PrintList(args.operand.empty() ? AllDemos() : registry.WithPrefix(args.operand));
            return 0;
        }
        if (command == "--list-ids")
        {
            for (const RegisteredDemo& demo : registry.All())
            {
                const auto needs = NeedNames(demo.entry.info.needs);
                std::string needList;
                for (const auto need : needs)
                {
                    needList += (needList.empty() ? "" : ",") + std::string(need);
                }
                std::cout << demo.entry.info.id << ' ' << ToString(demo.entry.kind) << ' '
                          << ToString(demo.entry.info.automation) << ' ' << (needList.empty() ? "-" : needList)
                          << '\n';
            }
            return 0;
        }
        if (command == "--categories")
        {
            for (const std::string_view category : registry.Categories())
            {
                std::cout << category << " (" << registry.WithPrefix(category).size() << ")\n";
            }
            return 0;
        }
        if (command == "--search")
        {
            const auto found = registry.Search(args.operand);
            PrintList(found);
            return found.empty() ? 1 : 0;
        }
        if (command == "--info")
        {
            const RegisteredDemo* demo = registry.Find(args.operand);
            if (demo == nullptr)
            {
                std::cerr << "cna-demos: no demo with ID '" << args.operand << "'\n";
                return UsageExitCode;
            }
            WriteInfo(*demo, std::cout);
            return 0;
        }
        if (command == "--catalog-json")
        {
            WriteCatalogJson(std::cout);
            return 0;
        }
        if (command == "--renderers")
        {
            for (const auto& name : CompiledRenderers())
            {
                std::cout << name << '\n';
            }
            return 0;
        }
        if (command == "--validate")
        {
            const auto problems = ValidateRegistry((ExecutableDirectory() / "assets").string());
            for (const std::string& problem : problems)
            {
                std::cout << "INVALID " << problem << '\n';
            }
            std::cout << registry.All().size() << " demo(s), " << problems.size() << " problem(s)\n";
            return problems.empty() ? 0 : 1;
        }
        if (command == "--run-category")
        {
            return RunMany(registry.WithPrefix(args.operand), args);
        }
        if (command == "--run-tag")
        {
            return RunMany(registry.WithTag(args.operand), args);
        }
        if (command == "--run-all" || command == "--smoke")
        {
            if (command == "--smoke")
            {
                args.run.automated = true;
            }
            return RunMany(AllDemos(), args);
        }
        if (command == "run")
        {
            return RunOne(args);
        }

        std::cerr << "cna-demos: unhandled command " << command << '\n';
        return UsageExitCode;
    }
}
