// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Runner.hpp"

#include <charconv>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <optional>
#include <ostream>

#include "CnaDemos/Catalog.hpp"
#include "CnaDemos/DemoGame.hpp"
#include "CnaDemos/Process.hpp"
#include "CNA/Logger.hpp"
#include "Microsoft/Xna/Framework/Audio/NoAudioHardwareException.hpp"

#if defined(_WIN32)
#  include <process.h>
#else
#  include <unistd.h>
#endif

namespace CnaDemos
{
    std::string_view ToString(RunStatus status)
    {
        switch (status)
        {
            case RunStatus::Passed: return "PASSED";
            case RunStatus::Failed: return "FAILED";
            case RunStatus::Skipped: return "SKIPPED";
            case RunStatus::NotRun: return "NOT RUN";
        }
        return "UNKNOWN";
    }

    int ExitCode(RunStatus status)
    {
        switch (status)
        {
            case RunStatus::Passed: return 0;
            case RunStatus::Skipped: return SkipExitCode;
            case RunStatus::Failed:
            case RunStatus::NotRun: return 1;
        }
        return 1;
    }

    namespace
    {
        std::string_view LogLevelName(CNA::LogLevel level)
        {
            switch (level)
            {
                case CNA::LogLevel::FATAL: return "fatal";
                case CNA::LogLevel::ERROR: return "error";
                case CNA::LogLevel::WARN: return "warn";
                case CNA::LogLevel::INFO: return "info";
                case CNA::LogLevel::DEBUG: return "debug";
                case CNA::LogLevel::TRACE: return "trace";
                default: return "warn";
            }
        }

        std::vector<std::string> StandaloneArguments(const DemoContext& context)
        {
            std::vector<std::string> arguments;
            if (context.Automated())
            {
                arguments.emplace_back("--automated");
            }
            const int frames = context.FrameLimit();
            if (frames > 0)
            {
                arguments.emplace_back("--frames");
                arguments.emplace_back(std::to_string(frames));
            }
            if (!context.Options().screenshotPath.empty())
            {
                arguments.emplace_back("--screenshot");
                arguments.emplace_back(context.Options().screenshotPath);
            }
            arguments.emplace_back("--log-level");
            arguments.emplace_back(LogLevelName(CNA::Logger::GetMinimumLevel()));
            return arguments;
        }

        void RunStandalone(const RegisteredDemo& demo, DemoContext& context, RunResult& result)
        {
#ifndef CNA_DEMOS_C_API_AVAILABLE
            // The c_api demos are built only when CNA has its C API (CNA_BUILD_C_API).
            if (HasNeed(demo.entry.info.needs, Need::CApi))
            {
                result.status = RunStatus::Skipped;
                result.message = "CNA was built without its C API (CNA_BUILD_C_API)";
                return;
            }
#endif
            std::filesystem::path program = ExecutableDirectory() / demo.entry.Executable();
#ifdef _WIN32
            program += ".exe";
#endif
            if (!std::filesystem::exists(program))
            {
                result.status = RunStatus::NotRun;
                result.message = "standalone executable not built: " + program.string();
                return;
            }
            const int exitCode = RunProcess(program, StandaloneArguments(context));
            if (exitCode == 0)
            {
                result.status = RunStatus::Passed;
            }
            else if (exitCode == SkipExitCode)
            {
                result.status = RunStatus::Skipped;
                result.message = "standalone demo reported a missing requirement";
            }
            else
            {
                result.status = RunStatus::Failed;
                result.message = "standalone executable exited with code " + std::to_string(exitCode);
            }
        }
    }

    namespace
    {
        // Reads the string value of "key" from the first result of a report written by
        // WriteResultsJson (whose escapes are \", \\, \n, \r, \t and \u00XX).
        std::optional<std::string> ReportString(const std::string& json, std::string_view key)
        {
            const std::string marker = "\"" + std::string(key) + "\": \"";
            std::size_t i = json.find(marker);
            if (i == std::string::npos)
            {
                return std::nullopt;
            }
            std::string value;
            for (i += marker.size(); i < json.size() && json[i] != '"'; ++i)
            {
                if (json[i] != '\\' || i + 1 >= json.size())
                {
                    value += json[i];
                    continue;
                }
                const char escaped = json[++i];
                switch (escaped)
                {
                    case 'n': value += '\n'; break;
                    case 'r': value += '\r'; break;
                    case 't': value += '\t'; break;
                    case 'u':
                    {
                        unsigned code = 0;
                        if (i + 4 < json.size())
                        {
                            std::from_chars(json.data() + i + 1, json.data() + i + 5, code, 16);
                        }
                        value += static_cast<char>(code);
                        i += 4;
                        break;
                    }
                    default: value += escaped; break;
                }
            }
            return value;
        }

        // DemoInfo::ownProcess: runs the demo as `cna-demos <id>` and takes its result from the
        // report that child process writes.
        void RunInOwnProcess(const RunOptions& options, RunResult& result, std::ostream& out)
        {
            std::filesystem::path program = ExecutableDirectory() / "cna-demos";
#ifdef _WIN32
            program += ".exe";
            const int pid = _getpid();
#else
            const int pid = static_cast<int>(getpid());
#endif
            const std::filesystem::path report = std::filesystem::temp_directory_path()
                / ("cna-demos-" + std::to_string(pid) + "-" + result.id + ".json");

            std::vector<std::string> arguments = {result.id, "--batch-child", "--report", report.string(),
                                                  "--seed", std::to_string(options.seed),
                                                  "--log-level", std::string(LogLevelName(CNA::Logger::GetMinimumLevel()))};
            if (options.automated)
            {
                arguments.emplace_back("--automated");
            }
            if (options.frames >= 0)
            {
                arguments.emplace_back("--frames");
                arguments.emplace_back(std::to_string(options.frames));
            }
            if (!options.screenshotPath.empty())
            {
                arguments.emplace_back("--screenshot");
                arguments.emplace_back(options.screenshotPath);
            }
            if (options.quiet)
            {
                arguments.emplace_back("--quiet");
            }

            out.flush();
            const int exitCode = RunProcess(program, arguments);

            std::ifstream file(report);
            const std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            std::error_code ignored;
            std::filesystem::remove(report, ignored);

            const auto status = ReportString(json, "status");
            if (!status)
            {
                result.status = RunStatus::Failed;
                result.message = "own-process run of " + result.id + " exited with code "
                    + std::to_string(exitCode) + " without a report";
                return;
            }
            result.status = *status == "PASSED"    ? RunStatus::Passed
                            : *status == "SKIPPED" ? RunStatus::Skipped
                            : *status == "NOT RUN" ? RunStatus::NotRun
                                                   : RunStatus::Failed;
            result.message = ReportString(json, "message").value_or("");
            if (const std::size_t at = json.find("\"checks\": "); at != std::string::npos)
            {
                const char* begin = json.data() + at + 10;
                std::from_chars(begin, json.data() + json.size(), result.checks);
            }
        }
    }

    RunResult RunDemo(const RegisteredDemo& demo, const RunOptions& options, std::ostream& out)
    {
        RunResult result;
        result.id = std::string(demo.entry.info.id);
        result.knownIssue = std::string(demo.entry.info.knownIssue);

        if (options.automated && demo.entry.info.automation == Automation::Manual)
        {
            result.status = RunStatus::NotRun;
            result.message = "manual demo (needs a person, hardware or a service)";
            return result;
        }

        const auto start = std::chrono::steady_clock::now();
        if (demo.entry.info.ownProcess && !options.freshProcess)
        {
            RunInOwnProcess(options, result, out);
            result.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            return result;
        }
        DemoContext context(demo.entry, options, out);
        try
        {
            CurrentContextScope scope(context);
            switch (demo.entry.kind)
            {
                case DemoKind::Console:
                    demo.console(context);
                    break;
                case DemoKind::Game:
                {
                    // XNA's "using (var game = new Game1()) game.Run();": Dispose after Run is what
                    // calls UnloadContent (through the device's disposal), so it must happen while
                    // the demo object is still alive. The game is then destroyed before the result
                    // is reported, so cleanup is part of what every run exercises.
                    std::unique_ptr<DemoGame> game = demo.game();
                    game->Run();
                    game->Dispose();
                    game.reset();
                    break;
                }
                case DemoKind::Standalone:
                    RunStandalone(demo, context, result);
                    break;
            }
            if (demo.entry.kind != DemoKind::Standalone)
            {
                result.status = context.Failed() ? RunStatus::Failed : RunStatus::Passed;
                if (context.Failed())
                {
                    result.message = context.Failures().front();
                }
            }
        }
        catch (const SkipDemo& skip)
        {
            result.status = RunStatus::Skipped;
            result.message = skip.what();
        }
        catch (const Microsoft::Xna::Framework::Audio::NoAudioHardwareException& noAudio)
        {
            // Only a demo that declared it needs audio may be skipped for lacking it; anywhere
            // else the exception is a genuine failure and is reported as one.
            if (HasNeed(demo.entry.info.needs, Need::Audio))
            {
                result.status = RunStatus::Skipped;
                result.message = std::string("no audio hardware: ") + noAudio.what();
            }
            else
            {
                result.status = RunStatus::Failed;
                result.message = std::string("NoAudioHardwareException: ") + noAudio.what();
            }
        }
        catch (const std::exception& exception)
        {
            result.status = RunStatus::Failed;
            result.message = std::string("exception: ") + exception.what();
        }
        catch (...)
        {
            result.status = RunStatus::Failed;
            result.message = "unknown exception";
        }

        result.checks = context.CheckCount();
        result.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        return result;
    }

    std::vector<RunResult> RunBatch(std::span<const RegisteredDemo* const> demos,
                                    const RunOptions& options, std::ostream& out)
    {
        std::vector<RunResult> results;
        results.reserve(demos.size());
        int counts[4] = {};
        int expectedFailures = 0;
        int unexpectedPasses = 0;

        for (const RegisteredDemo* demo : demos)
        {
            out << "== " << demo->entry.info.id << '\n';
            RunResult result = RunDemo(*demo, options, out);
            ++counts[static_cast<int>(result.status)];
            out << "   " << ToString(result.status) << " (" << std::fixed << std::setprecision(2)
                << result.seconds << " s";
            if (result.checks > 0)
            {
                out << ", " << result.checks << " checks";
            }
            out << ")";
            if (!result.message.empty())
            {
                out << ": " << result.message;
            }
            out << '\n';
            if (result.ExpectedFailure())
            {
                ++expectedFailures;
                out << "   (expected: known CNA issue " << result.knownIssue << ", docs/CNA_ISSUES.md)\n";
            }
            else if (result.UnexpectedPass())
            {
                ++unexpectedPasses;
                out << "   (UNEXPECTED PASS: known issue " << result.knownIssue
                    << " did not reproduce -- check whether CNA fixed it and update the demo)\n";
            }
            results.push_back(std::move(result));
        }

        out << "\nSummary: " << results.size() << " demo(s): "
            << counts[static_cast<int>(RunStatus::Passed)] << " passed, "
            << counts[static_cast<int>(RunStatus::Failed)] << " failed, "
            << counts[static_cast<int>(RunStatus::Skipped)] << " skipped, "
            << counts[static_cast<int>(RunStatus::NotRun)] << " not run\n";
        if (expectedFailures > 0 || unexpectedPasses > 0)
        {
            out << "Known CNA issues: " << expectedFailures << " expected failure(s), " << unexpectedPasses
                << " unexpected pass(es)\n";
        }
        for (const RunResult& result : results)
        {
            if (result.status == RunStatus::Failed && !result.ExpectedFailure())
            {
                out << "  FAILED " << result.id << ": " << result.message << '\n';
            }
            else if (result.UnexpectedPass())
            {
                out << "  UNEXPECTED PASS " << result.id << " (" << result.knownIssue << ")\n";
            }
        }
        return results;
    }

    void WriteResultsJson(std::span<const RunResult> results, std::ostream& out)
    {
        out << "{\n  \"schema\": \"cna-demos-results/1\",\n";
        out << "  \"cnaDemosVersion\": \"" << JsonEscape(CnaDemosVersion()) << "\",\n";
        out << "  \"cnaVersion\": \"" << JsonEscape(CnaVersion()) << "\",\n";
        out << "  \"results\": [";
        bool first = true;
        for (const RunResult& result : results)
        {
            out << (first ? "\n" : ",\n");
            first = false;
            out << "    {\"id\": \"" << JsonEscape(result.id) << "\", \"status\": \""
                << JsonEscape(ToString(result.status)) << "\", \"checks\": " << result.checks
                << ", \"seconds\": " << std::fixed << std::setprecision(3) << result.seconds
                << ", \"knownIssue\": \"" << JsonEscape(result.knownIssue) << "\""
                << ", \"message\": \"" << JsonEscape(result.message) << "\"}";
        }
        out << "\n  ]\n}\n";
    }
}
