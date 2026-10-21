// SPDX-License-Identifier: MS-PL
// ISSUE-028 follow-up: renaming a path must not replace an already-open streaming bank.
// Intended for platforms allowing rename of an open file; no Windows runtime claim.
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
        fs::path path = fs::temp_directory_path() / ("cna-wave-retained-file-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        OwnedRoot()
        {
            if (!fs::create_directory(path)) throw std::runtime_error("Fixture root already exists");
        }
        ~OwnedRoot() { std::error_code ignored; fs::remove_all(path, ignored); }
    };
}

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2) throw std::runtime_error("Pass the existing absolute XACT asset directory");
        const fs::path assets = fs::absolute(argv[1]);
        OwnedRoot root;
        int checks = 0, failures = 0;
        const auto check = [&](bool condition, const std::string& label)
        {
            ++checks;
            if (!condition) ++failures;
            std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
        };
        for (const auto& mode : {"unchanged_absolute", "renamed_absolute", "replaced_absolute"})
        {
            const fs::path folder = root.path / mode;
            fs::create_directory(folder);
            const fs::path original = folder / "music.xwb";
            fs::copy_file(assets / "music.xwb", original);
            AudioEngine engine((assets / "demo.xgs").string());
            WaveBank bank(&engine, original.string(), 0, 16);
            check(bank.getIsPreparedProperty(), std::string(mode) + " prepared before path change");
            if (std::string(mode) != "unchanged_absolute")
            {
                fs::rename(original, folder / "retained.xwb");
                if (std::string(mode) == "replaced_absolute")
                {
                    std::ofstream decoy(original, std::ios::binary);
                    decoy << "not the originally opened wave bank";
                    if (!decoy) throw std::runtime_error("Could not write owned replacement");
                }
            }
            const SoundEffect* sound = WaveBankTestAccess::GetSoundEffect(bank, 0);
            check(sound != nullptr && sound->getDurationProperty().getTicksProperty() > 0,
                  std::string(mode) + " lazily reads the originally opened file");
        }
        std::cout << "checks=" << checks << " failed=" << failures << '\n';
        return failures ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fixture/platform error (not a pass): " << error.what() << '\n';
        return 2;
    }
}
