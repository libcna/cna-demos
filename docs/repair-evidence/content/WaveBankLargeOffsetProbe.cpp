// SPDX-License-Identifier: MS-PL
// ISSUE-028 safeguard: lazy bank offsets must not be narrowed to signed 32-bit IO.
// Linux-only sparse-file witness; allocates only the original small bank's bytes.
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#if defined(__linux__)
#include <sys/stat.h>
#endif

namespace Microsoft::Xna::Framework::Audio
{
    struct WaveBankTestAccess
    {
        static const SoundEffect* GetSoundEffect(WaveBank& bank, unsigned short index)
        {
            return bank.GetSoundEffect(index);
        }
        static std::size_t ResidentBytes(const WaveBank& bank)
        {
            return bank.ResidentFileBytesInternal();
        }
    };
}

namespace
{
    namespace fs = std::filesystem;
    using namespace Microsoft::Xna::Framework::Audio;
    struct OwnedRoot
    {
        fs::path path = fs::temp_directory_path() / ("cna-wave-large-offset-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        OwnedRoot()
        {
            if (!fs::create_directory(path)) throw std::runtime_error("Fixture root already exists");
        }
        ~OwnedRoot() { std::error_code ignored; fs::remove_all(path, ignored); }
    };
    std::uint32_t ReadU32(const std::vector<unsigned char>& bytes, std::size_t offset)
    {
        std::uint32_t value = 0;
        for (unsigned i = 0; i < 4; ++i)
            value |= static_cast<std::uint32_t>(bytes.at(offset + i)) << (i * 8);
        return value;
    }
    void WriteU32(std::vector<unsigned char>& bytes, std::size_t offset, std::uint32_t value)
    {
        for (unsigned i = 0; i < 4; ++i)
            bytes.at(offset + i) = static_cast<unsigned char>(value >> (i * 8));
    }
}

int main(int argc, char** argv)
{
    try
    {
#if !defined(__linux__)
        throw std::runtime_error("This sparse allocation witness requires Linux stat accounting");
#endif
        if (argc != 2) throw std::runtime_error("Pass the existing absolute XACT asset directory");
        const fs::path assets = fs::absolute(argv[1]);
        const fs::path original = assets / "music.xwb";
        const auto size = fs::file_size(original);
        if (size < 52 || size > 4 * 1024 * 1024)
            throw std::runtime_error("Expected existing small version46 bank fixture");
        std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
        std::ifstream input(original, std::ios::binary);
        input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
        if (input.gcount() != static_cast<std::streamsize>(size) ||
            ReadU32(bytes, 0) != 0x444e4257 || ReadU32(bytes, 4) != 46)
            throw std::runtime_error("Unexpected XWB fixture shape");
        const auto dataOffset = ReadU32(bytes, 44), dataLength = ReadU32(bytes, 48);
        if (dataOffset < 52 || static_cast<std::uint64_t>(dataOffset) + dataLength != size)
            throw std::runtime_error("Expected terminal wave-data segment");
        constexpr std::uint32_t largeOffset = 0x80001000u;
        WriteU32(bytes, 44, largeOffset);
        OwnedRoot root;
        const fs::path large = root.path / "large-offset.xwb";
        {
            std::ofstream output(large, std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes.data()), dataOffset);
            output.seekp(static_cast<std::streamoff>(largeOffset));
            output.write(reinterpret_cast<const char*>(bytes.data() + dataOffset), dataLength);
            output.close();
            if (!output) throw std::runtime_error("Could not write owned sparse fixture");
        }
        int checks = 0, failures = 0;
        const auto check = [&](bool value, const std::string& label)
        {
            ++checks; if (!value) ++failures;
            std::cout << (value ? "PASS " : "FAIL ") << label << '\n';
        };
        check(fs::file_size(large) == static_cast<std::uint64_t>(largeOffset) + dataLength &&
                  fs::file_size(large) > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max()),
              "sparse logical size crosses signed32 boundary");
#if defined(__linux__)
        struct stat allocation{};
        if (::stat(large.c_str(), &allocation) != 0)
            throw std::runtime_error("Could not inspect owned sparse allocation");
        const auto allocated = static_cast<std::uint64_t>(allocation.st_blocks) * 512;
        check(allocated < 1024 * 1024, "sparse allocated bytes stay below one MiB");
        if (allocated >= 1024 * 1024)
            throw std::runtime_error("Filesystem did not preserve the sparse fixture");
#endif
        AudioEngine engine((assets / "demo.xgs").string());
        const auto inspect = [&](const fs::path& file, const std::string& label)
        {
            WaveBank bank(&engine, file.string(), 0, 16);
            check(bank.getIsPreparedProperty(), label + " prepared");
            check(WaveBankTestAccess::ResidentBytes(bank) < 64 * 1024,
                  label + " only small metadata is resident");
            const auto* sound = WaveBankTestAccess::GetSoundEffect(bank, 0);
            const auto ticks = sound ? sound->getDurationProperty().getTicksProperty() : 0;
            check(sound != nullptr && ticks > 0, label + " lazy PCM decodes");
            return ticks;
        };
        const auto ordinaryTicks = inspect(original, "ordinary");
        const auto largeTicks = inspect(large, "large offset");
        check(ordinaryTicks > 0 && largeTicks == ordinaryTicks,
              "large offset preserves the original decoded duration");
        std::cout << "checks=" << checks << " failed=" << failures << '\n';
        return failures ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fixture/platform error (not a pass): " << error.what() << '\n';
        return 2;
    }
}
