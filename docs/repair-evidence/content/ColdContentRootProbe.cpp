// SPDX-License-Identifier: MS-PL
// ISSUE-014: a relative content root must not change when TitleLocation is first queried.
// Linux reproduction uses /proc/self/exe only to stage data without initializing CNA.
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "CNA/Internal/TitlePath.hpp"
#include "CNA/Platform/CurrentPlatform.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace fs = std::filesystem;
using Microsoft::Xna::Framework::Content::ContentManager;
namespace
{
    struct OwnedDirectory
    {
        fs::path path;
        explicit OwnedDirectory(fs::path value) : path(std::move(value))
        {
            if (!fs::create_directory(path)) throw std::runtime_error("Private fixture directory already exists");
        }
        ~OwnedDirectory() { std::error_code ignored; fs::remove_all(path, ignored); }
        OwnedDirectory(const OwnedDirectory&) = delete;
        OwnedDirectory& operator=(const OwnedDirectory&) = delete;
    };
    struct RestoreWorkingDirectory
    {
        fs::path previous = fs::current_path();
        ~RestoreWorkingDirectory() { std::error_code ignored; fs::current_path(previous, ignored); }
    };
    void Write(const fs::path& file)
    {
        std::ofstream stream;
        stream.exceptions(std::ios::badbit | std::ios::failbit);
        stream.open(file, std::ios::binary);
        stream << "manifest-only fixture";
    }
    std::set<std::string> Names(ContentManager& manager)
    {
        std::set<std::string> names;
        for (const auto& entry : manager.GetContentManifest()) names.insert(entry.relativePath);
        return names;
    }
}
int main()
{
    try
    {
        int checks = 0, failed = 0;
        const auto check = [&](bool condition, const char* label)
        {
            ++checks;
            if (!condition) ++failed;
            std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
        };
        check(!CNA::Platform::HasCurrentPlatform() && !CNA::Internal::TryGetTitlePath(),
              "fresh process has no platform or title override");
        const auto executableDirectory = fs::read_symlink("/proc/self/exe").parent_path();
        const std::string relativeRoot = "cna-repair-cold-content-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        OwnedDirectory title(executableDirectory / relativeRoot);
        OwnedDirectory working(fs::temp_directory_path() / relativeRoot);
        fs::create_directory(working.path / relativeRoot);
        Write(title.path / "title-only.png");
        Write(working.path / relativeRoot / "cwd-only.png");
        RestoreWorkingDirectory restore;
        fs::current_path(working.path);
        ContentManager manager(nullptr, relativeRoot);
        const auto cold = Names(manager);
        check(cold.contains("title-only"), "first manifest uses title directory without warmup");
        check(!cold.contains("cwd-only"), "first manifest excludes working-directory decoy");
        check(manager.getRootDirectoryProperty() == relativeRoot, "authored relative root is retained");
        check(Names(manager) == cold, "repeated manifest remains a cached snapshot");
        const auto titlePath = Microsoft::Xna::Framework::TitleLocation::Path();
        check(fs::equivalent(fs::path(titlePath), executableDirectory), "detected title directory is the executable directory");
        check(CNA::Platform::HasCurrentPlatform(), "title lookup initializes the platform");
        check(Names(manager) == cold, "title lookup does not silently refresh a cached manifest");
        manager.RefreshContentManifest();
        const auto warm = Names(manager);
        check(warm == std::set<std::string>{"title-only"}, "refreshed manifest contains only the title asset");
        check(warm == cold, "content root is independent of title lookup ordering");
        manager.RefreshContentManifest();
        check(Names(manager) == warm, "subsequent refresh is stable");
        ContentManager absolute(nullptr, (working.path / relativeRoot).string());
        check(Names(absolute) == std::set<std::string>{"cwd-only"}, "explicit absolute root remains authoritative");
        std::cout << "checks=" << checks << " failed=" << failed << '\n';
        return failed ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unexpected fixture error: " << error.what() << '\n';
        return 2;
    }
}
