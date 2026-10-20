// SPDX-License-Identifier: MS-PL
// The four write-only preservation checks from FileShareEdgesOracle.cs.
#include "System/IO/FileStream.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace System::IO;
int main(int argc, char** argv)
{
    if (argc != 2 || std::filesystem::exists(argv[1])) return 2;
    int checks = 0, failed = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failed; std::cout << "FAIL " << name << '\n'; }
    };
    for (const FileMode mode : {FileMode::Open, FileMode::OpenOrCreate})
    {
        {
            std::ofstream seed(argv[1], std::ios::binary | std::ios::trunc);
            if (!seed) return 2;
            const char bytes[]{1, 2, 3, 4};
            seed.write(bytes, 4);
        }
        {
            FileStream writer(argv[1], mode, FileAccess::Write);
            check(writer.getLengthProperty() == 4 && writer.getPositionProperty() == 0,
                  "write-only open preserves length and starts at zero");
            writer.WriteByte(9);
        }
        std::array<char, 5> bytes{};
        std::ifstream reader(argv[1], std::ios::binary);
        reader.read(bytes.data(), bytes.size());
        check(reader.gcount() == 4 && bytes[0] == 9 && bytes[1] == 2 && bytes[3] == 4,
              "write-only overwrite does not truncate remaining bytes");
    }
    std::filesystem::remove(argv[1]); // Only the exact newly created fixture.
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed ? 1 : 0;
}
