// SPDX-License-Identifier: MS-PL
// ISSUE-110: the public decompressor's exact size and ceiling also apply to None.
#include "CNA/Content/Cnb/CnbChunkCompression.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include <iostream>
#include <limits>

using namespace CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentLoadException;
namespace
{
    int checks = 0, failed = 0;
    void Check(bool condition, const char* label)
    {
        ++checks;
        if (!condition) ++failed;
        std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
    }
    void Refuses(std::span<const std::uint8_t> bytes, CnbCompression codec,
                 std::uint64_t size, std::uint64_t ceiling, const char* label)
    {
        bool exact = false;
        try { (void)DecompressCnbChunk(bytes, codec, size, ceiling, "probe chunk"); }
        catch (const ContentLoadException& error)
        {
            exact = typeid(error) == typeid(ContentLoadException) &&
                error.getMessageProperty().find("probe chunk") != std::string::npos;
        }
        catch (...) { }
        Check(exact, label);
    }
}
int main()
{
    try
    {
        const std::vector<std::uint8_t> bytes{1, 2, 3, 4};
        Check(DecompressCnbChunk({}, CnbCompression::None, 0, 0, "empty").empty(), "empty chunk fits zero ceiling");
        auto copy = DecompressCnbChunk(bytes, CnbCompression::None, 4, 4, "exact");
        Check(copy == bytes, "exact uncompressed chunk preserves every byte");
        copy[0] = 9;
        Check(bytes.front() == 1 && copy.data() != bytes.data(), "returned bytes are an independent copy");
        Refuses(bytes, CnbCompression::None, 3, 4, "declared size smaller than storage refused");
        Refuses(bytes, CnbCompression::None, 5, 5, "declared size larger than storage refused");
        Refuses(bytes, CnbCompression::None, 4, 3, "exact chunk above ceiling refused");
        Refuses({}, CnbCompression::None, 1, 1, "nonzero declaration for empty storage refused");
        Refuses(bytes, CnbCompression::None, 0, 0, "nonempty storage with zero declaration refused");
        Refuses(bytes, CnbCompression::None, std::numeric_limits<std::uint64_t>::max(),
                std::numeric_limits<std::uint64_t>::max(), "unrepresentable declaration cannot bypass exact size");
        const std::vector<std::uint8_t> historical(4096, 42);
        Refuses(historical, CnbCompression::None, 4097, 1, "historical mismatch and tiny ceiling refused");
        Refuses(bytes, static_cast<CnbCompression>(255), 4, 4, "unknown codec still gives contextual ContentLoadException");
        std::cout << "checks=" << checks << " failed=" << failed << '\n';
        return failed ? 1 : 0;
    }
    catch (const std::exception& error) { std::cerr << "Unexpected fixture error: " << error.what() << '\n'; return 2; }
}
