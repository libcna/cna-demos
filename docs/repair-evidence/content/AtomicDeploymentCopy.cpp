// SPDX-License-Identifier: MS-PL
// ISSUE-047 publication seam: use the actual portable atomic-copy helper.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "CnaToolAtomicWrite.hpp"

int main(int argc, char** argv)
{
    if (argc != 2) { return 2; }
    const std::filesystem::path root(argv[1]);
    if (!root.is_absolute() || !std::filesystem::is_directory(root) ||
        !std::filesystem::is_empty(root)) { return 2; }
    const auto source = root / "source.bin";
    const auto destination = root / "destination.bin";
    const auto read = [](const std::filesystem::path& file)
    {
        std::ifstream stream(file, std::ios::binary);
        if (!stream) { throw std::runtime_error("cannot read fixture"); }
        return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(stream), {});
    };
    int failures = 0;
    const auto check = [&](const char* name, const auto& action)
    {
        try
        {
            action();
            std::cout << "PASS " << name << '\n';
        }
        catch (const std::exception& error)
        {
            ++failures;
            std::cout << "FAIL " << name << ": " << error.what() << '\n';
        }
    };
    const std::vector<std::uint8_t> bytes{0u, 255u, 42u, 17u};
    CNA::Tools::WriteFileAtomically(source, bytes);
    check("different_destination", [&]
    {
        CNA::Tools::CopyFileAtomically(source, destination);
        if (read(source) != bytes || read(destination) != bytes)
            throw std::runtime_error("different-destination bytes changed");
    });
    check("same_destination", [&]
    {
        CNA::Tools::CopyFileAtomically(source, source);
        if (read(source) != bytes) { throw std::runtime_error("self-copy bytes changed"); }
    });
    check("lexically_equivalent_destination", [&]
    {
        CNA::Tools::CopyFileAtomically(source, root / "." / "source.bin");
        if (read(source) != bytes) { throw std::runtime_error("alias-copy bytes changed"); }
    });
    CNA::Tools::WriteFileAtomically(source, {});
    check("empty_same_destination", [&]
    {
        CNA::Tools::CopyFileAtomically(source, source);
        if (!read(source).empty()) { throw std::runtime_error("empty self-copy changed"); }
    });
    check("no_temporary_leaks", [&]
    {
        for (const auto& entry : std::filesystem::directory_iterator(root))
        {
            if (entry.path() != source && entry.path() != destination)
                throw std::runtime_error("unexpected temporary file");
        }
    });
    std::cout << "FAILURES " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
