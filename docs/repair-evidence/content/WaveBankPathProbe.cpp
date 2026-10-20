// SPDX-License-Identifier: MS-PL
// ISSUE-028: current streaming lookup roots, without playback or a physical audio device.
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
namespace
{
    struct OwnedRoot
    {
        fs::path path = fs::temp_directory_path() / ("cna-wave-path-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        OwnedRoot() { if (!fs::create_directory(path)) throw std::runtime_error("Fixture already exists"); }
        ~OwnedRoot() { std::error_code ignored; fs::remove_all(path, ignored); }
    };
    struct RestoreWorkingDirectory
    {
        fs::path previous = fs::current_path();
        ~RestoreWorkingDirectory() { std::error_code ignored; fs::current_path(previous, ignored); }
    };
    struct RestoreTitle
    {
        std::string previous = TitleLocation::Path();
        ~RestoreTitle() { TitleLocation::setPathProperty(previous); }
    };
}
int main(int argc, char** argv)
{
    try
    {
        if (argc != 2) throw std::runtime_error("Pass the absolute existing XACT fixture directory");
        const fs::path assets = fs::absolute(argv[1]);
        OwnedRoot root;
        const fs::path title = root.path / "title", working = root.path / "working";
        fs::create_directory(title); fs::create_directory(working);
        fs::copy_file(assets / "music.xwb", title / "asset.xwb");
        RestoreWorkingDirectory restoreWorking;
        RestoreTitle restoreTitle;
        TitleLocation::setPathProperty(title.string());
        fs::current_path(working);
        AudioEngine engine((assets / "demo.xgs").string());
        int checks = 0, failures = 0;
        const auto check = [&](bool condition, const char* label)
        {
            ++checks; if (!condition) ++failures;
            std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
        };
        const auto prepared = [&](const std::string& filename)
        {
            WaveBank bank(&engine, filename, 0, 16);
            return bank.getIsPreparedProperty();
        };
        check(prepared((title / "asset.xwb").string()), "absolute title file loads a valid streaming bank");
        check(!prepared("asset.xwb"), "relative lookup does not substitute an existing title file");
        fs::copy_file(title / "asset.xwb", working / "asset.xwb");
        check(prepared("asset.xwb"), "relative lookup finds the valid cwd bank");
        TitleLocation::setPathProperty((root.path / "unrelated-title").string());
        check(prepared("asset.xwb"), "an explicit title override does not redirect streaming lookup");
        fs::remove(working / "asset.xwb");
        check(!prepared("asset.xwb"), "removing the cwd bank makes the same relative lookup unavailable");
        check(prepared((title / "asset.xwb").string()), "absolute lookup remains independent of cwd and title overrides");
        std::cout << "checks=" << checks << " failed=" << failures << '\n';
        return failures ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unexpected fixture error: " << error.what() << '\n';
        return 2;
    }
}
