// SPDX-License-Identifier: MS-PL
// ISSUE-106: refusal of a pre-built registry must describe the selected option.
#include "CNA/Content/Pipeline/ContentCompiler.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Pipeline = CNA::Content::Pipeline;
namespace
{
    struct Scratch
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() /
            ("cna-repair-compiler-options-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        Scratch()
        {
            if (!std::filesystem::create_directory(path))
                throw std::runtime_error("Cannot reserve private probe directory");
            std::filesystem::create_directory(path / "source");
        }
        ~Scratch() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
        Scratch(const Scratch&) = delete;
        Scratch& operator=(const Scratch&) = delete;
    };
    struct Capture
    {
        std::ostringstream text;
        std::streambuf* previous = std::cerr.rdbuf(text.rdbuf());
        ~Capture() { std::cerr.rdbuf(previous); }
        Capture() = default;
        Capture(const Capture&) = delete;
        Capture& operator=(const Capture&) = delete;
    };
}
int main()
{
    try
    {
        Scratch scratch;
        int checks = 0, failed = 0;
        const auto check = [&](bool condition, const std::string& label)
        {
            ++checks;
            if (!condition) ++failed;
            std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
        };
        const std::pair<std::string, std::filesystem::path> options[] = {
            {"--fx-compiler", "unused-fxc"},
            {"--fx-compiler-launcher", "unused-fxc-launcher"},
            {"--xma-encoder", "unused-xma"},
            {"--xma-encoder-launcher", "unused-xma-launcher"},
            {"--xma-encoder-arg", "{input}"},
            {"--font-directory", scratch.path},
            {"--build-configuration", "Debug"}
        };
        for (const auto& [option, value] : options)
        {
            const auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
            int result;
            std::string diagnostic;
            {
                Capture captured;
                result = Pipeline::RunContentCompiler(
                    {"build", scratch.path / "source", "-o", scratch.path / "output",
                     "--format", "cnb", "--workers", "1", option, value}, registry);
                diagnostic = captured.text.str();
            }
            check(result == 2, option + " rejects pre-built registry with command-line status");
            check(diagnostic.find(option) != std::string::npos, option + " diagnostic identifies selected option");
            check(diagnostic.find("ContentPipelineRegistryFactory") != std::string::npos &&
                  !registry->IsFrozen() && !std::filesystem::exists(scratch.path / "output"),
                  option + " directs caller to factory before freezing or publishing");
        }
        std::cout << "checks=" << checks << " failed=" << failed << '\n';
        return failed ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unexpected fixture error: " << error.what() << '\n';
        return 2;
    }
}
