// SPDX-License-Identifier: MS-PL
// ISSUE-028 follow-up: a streaming bank must keep the file selected at construction.
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Microsoft::Xna::Framework::Audio
{
    // Existing native WaveBank test seam; no production API or access change.
    struct WaveBankTestAccess
    {
        static const SoundEffect* GetSoundEffect(WaveBank& bank, unsigned short index)
        {
            return bank.GetSoundEffect(index);
        }
    };
}

namespace
{
    namespace fs = std::filesystem;
    using namespace Microsoft::Xna::Framework::Audio;

    struct OwnedRoot
    {
        fs::path path = fs::temp_directory_path() / ("cna-wave-lazy-path-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        OwnedRoot()
        {
            if (!fs::create_directory(path)) throw std::runtime_error("Fixture root already exists");
        }
        ~OwnedRoot() { std::error_code ignored; fs::remove_all(path, ignored); }
    };
    struct RestoreWorkingDirectory
    {
        fs::path previous = fs::current_path();
        ~RestoreWorkingDirectory() { std::error_code ignored; fs::current_path(previous, ignored); }
    };
}

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2) throw std::runtime_error("Pass the existing absolute XACT asset directory");
        const fs::path assets = fs::absolute(argv[1]);
        OwnedRoot root;
        RestoreWorkingDirectory restoreWorking;
        const auto original = root.path / "original";
        const auto empty = root.path / "empty";
        const auto decoy = root.path / "decoy";
        fs::create_directory(original);
        fs::create_directory(empty);
        fs::create_directory(decoy);
        fs::copy_file(assets / "music.xwb", original / "music.xwb");
        {
            std::ofstream wrong(decoy / "music.xwb", std::ios::binary);
            wrong << "not a wave bank";
            if (!wrong) throw std::runtime_error("Could not create owned decoy");
        }
        int checks = 0, failures = 0;
        const auto check = [&](bool condition, const std::string& label)
        {
            ++checks;
            if (!condition) ++failures;
            std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
        };
        for (const auto& mode : {"relative_same_cwd", "absolute_changed_cwd",
                                "relative_changed_cwd", "relative_decoy_cwd"})
        {
            fs::current_path(original);
            AudioEngine engine((assets / "demo.xgs").string());
            const bool absolute = std::string(mode) == "absolute_changed_cwd";
            WaveBank bank(&engine, absolute ? (original / "music.xwb").string() : "music.xwb", 0, 16);
            check(bank.getIsPreparedProperty(), std::string(mode) + " prepared before cwd change");
            if (std::string(mode) == "relative_decoy_cwd") fs::current_path(decoy);
            else if (std::string(mode) != "relative_same_cwd") fs::current_path(empty);
            const SoundEffect* sound = WaveBankTestAccess::GetSoundEffect(bank, 0);
            check(sound != nullptr && sound->getDurationProperty().getTicksProperty() > 0,
                  std::string(mode) + " lazily reads the original bank entry");
        }
        std::cout << "checks=" << checks << " failed=" << failures << '\n';
        return failures ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unexpected fixture error: " << error.what() << '\n';
        return 2;
    }
}
