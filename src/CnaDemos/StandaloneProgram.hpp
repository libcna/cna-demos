// SPDX-License-Identifier: MS-PL
#pragma once
// What the C++ standalone demos that are not DemoGames share: the arguments the runner passes,
// the log level, a log with checks, and a screenshot of the back buffer. Header-only; a program
// that uses RunProcess/ExecutableDirectory links cna_demos_framework (only Process.cpp is pulled
// in). The runner passes [--automated] [--frames N] [--screenshot PATH] [--log-level LEVEL].

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "CNA/Logger.hpp"
#include "CnaDemos/Process.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

namespace CnaDemos::Standalone
{
    /** The runner's arguments, plus whatever else the program was given (in order). */
    struct Arguments
    {
        bool automated = false;
        std::string screenshot;
        std::string logLevel = "warn";
        std::vector<std::string> other;
    };

    /** Parses the runner's arguments and applies --log-level. --frames is accepted and ignored. */
    inline Arguments ParseArguments(int argc, char** argv)
    {
        Arguments arguments;
        for (int i = 1; i < argc; ++i)
        {
            const std::string_view argument(argv[i]);
            if (argument == "--automated")
                arguments.automated = true;
            else if (argument == "--frames" && i + 1 < argc)
                ++i;
            else if (argument == "--screenshot" && i + 1 < argc)
                arguments.screenshot = argv[++i];
            else if (argument == "--log-level" && i + 1 < argc)
                arguments.logLevel = argv[++i];
            else
                arguments.other.emplace_back(argument);
        }
        const std::string& level = arguments.logLevel;
        CNA::Logger::SetMinimumLevel(level == "fatal"   ? CNA::LogLevel::FATAL
                                     : level == "error" ? CNA::LogLevel::ERROR
                                     : level == "info"  ? CNA::LogLevel::INFO
                                     : level == "debug" || level == "trace" ? CNA::LogLevel::DEBUG
                                                                            : CNA::LogLevel::WARN);
        return arguments;
    }

    /** This program's own executable (to start it again in another role). */
    inline std::filesystem::path ThisProgram(const std::string& name)
    {
#ifdef _WIN32
        return ExecutableDirectory() / (name + ".exe");
#else
        return ExecutableDirectory() / name;
#endif
    }

    /** Lines printed with the process's role in front, kept for drawing, and counted checks. */
    class Report
    {
    public:
        explicit Report(std::string role) : role_(std::move(role)) {}

        void Log(const std::string& line)
        {
            std::cout << role_ << ": " << line << '\n' << std::flush;
            lines_.push_back(line);
        }

        void Check(bool passed, const std::string& what)
        {
            std::cout << role_ << (passed ? ":   check: " : ":   CHECK FAILED: ") << what << '\n' << std::flush;
            failures_ += passed ? 0 : 1;
        }

        [[nodiscard]] const std::vector<std::string>& Lines() const { return lines_; }
        [[nodiscard]] int Failures() const { return failures_; }

    private:
        std::string role_;
        std::vector<std::string> lines_;
        int failures_ = 0;
    };

    /** Saves the back buffer as PNG (the device must use the HiDef profile). */
    inline void SaveScreenshot(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device, const std::string& path)
    {
        using Microsoft::Xna::Framework::Color;
        const auto& parameters = device.getPresentationParametersProperty();
        const int width = parameters.getBackBufferWidthProperty();
        const int height = parameters.getBackBufferHeightProperty();
        std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
        device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
        for (Color& pixel : pixels)
        {
            pixel.setAProperty(255); // a window shows RGB only
        }
        Microsoft::Xna::Framework::Graphics::Texture2D image(device, width, height);
        image.SetData(pixels.data(), static_cast<int>(pixels.size()));
        image.SaveAsPng(path);
        std::cout << "  screenshot: " << path << '\n';
    }
}
