// SPDX-License-Identifier: MS-PL
// Audit CNA's public text and binary FBX reader.
#include <array>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "CNA/Content/Pipeline/FbxFileReader.hpp"

namespace
{
    namespace Pipeline = CNA::Content::Pipeline;

    std::vector<std::uint8_t> Bytes(const std::string_view text)
    {
        return {text.begin(), text.end()};
    }

    template<typename T>
    void Put(std::vector<std::uint8_t>& bytes, const T& value)
    {
        const auto* raw = reinterpret_cast<const std::uint8_t*>(&value);
        bytes.insert(bytes.end(), raw, raw + sizeof(value));
    }

    void Patch32(std::vector<std::uint8_t>& bytes, const std::size_t offset,
                 const std::uint32_t value)
    {
        std::memcpy(bytes.data() + offset, &value, sizeof(value));
    }

    std::vector<std::uint8_t> MakeBinaryFixture()
    {
        std::vector<std::uint8_t> bytes = Bytes("Kaydara FBX Binary  ");
        bytes.insert(bytes.end(), {0u, 0x1Au, 0u});
        Put(bytes, std::uint32_t{7400u});

        const std::size_t record = bytes.size();
        Put(bytes, std::uint32_t{0u}); // Patched absolute end offset.
        Put(bytes, std::uint32_t{4u});
        Put(bytes, std::uint32_t{0u}); // Property byte count is advisory to this reader.
        bytes.push_back(4u);
        bytes.insert(bytes.end(), {'N', 'o', 'd', 'e'});

        bytes.push_back('D');
        Put(bytes, 4.5);
        bytes.push_back('S');
        Put(bytes, std::uint32_t{6u});
        bytes.insert(bytes.end(), {'b', 'i', 'n', 'a', 'r', 'y'});

        bytes.push_back('d');
        Put(bytes, std::uint32_t{3u});
        Put(bytes, std::uint32_t{0u});
        Put(bytes, std::uint32_t{0u});
        for (const double value : {1.0, 2.0, 3.0}) Put(bytes, value);

        constexpr std::array<std::uint8_t, 22> compressed{
            0x78u, 0x9Cu, 0x63u, 0x60u, 0x00u, 0x81u, 0x2Fu, 0xF6u,
            0x60u, 0x8Au, 0x81u, 0xE5u, 0x00u, 0x84u, 0x56u, 0x72u,
            0x00u, 0x00u, 0x1Cu, 0xDBu, 0x02u, 0x5Au};
        bytes.push_back('d');
        Put(bytes, std::uint32_t{3u});
        Put(bytes, std::uint32_t{1u});
        Put(bytes, static_cast<std::uint32_t>(compressed.size()));
        bytes.insert(bytes.end(), compressed.begin(), compressed.end());

        Patch32(bytes, record, static_cast<std::uint32_t>(bytes.size()));
        return bytes;
    }

    std::optional<Pipeline::FbxFileError> ErrorFrom(
        const std::span<const std::uint8_t> bytes,
        const Pipeline::FbxFileLimits& limits = {})
    {
        try
        {
            (void) Pipeline::ReadFbxFile(bytes, limits);
        }
        catch (const Pipeline::FbxFileException& error)
        {
            return error.Error();
        }
        return std::nullopt;
    }

    int Run()
    {
        bool passed = true;
        const auto check = [&](const bool condition, const std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        const Pipeline::FbxNode safeNode;
        const Pipeline::FbxFile safeFile;
        const Pipeline::FbxFileLimits defaults;
        check(safeNode.name.empty() && safeNode.properties.empty() && safeNode.children.empty()
                  && safeFile.version == 0u && !safeFile.binary && safeFile.nodes.empty()
                  && defaults.maximumBytes == 512u * 1024u * 1024u
                  && defaults.maximumDepth == 64u && defaults.maximumNodes == 4000000u
                  && defaults.maximumArrayLength == 64u * 1024u * 1024u
                  && defaults.maximumDecompressedBytes == 512u * 1024u * 1024u
                  && defaults.maximumStringLength == 1024u * 1024u,
              "node, file and all six resource ceilings expose their safe documented defaults");

        const auto textBytes = Bytes(
            "; FBX 7.4.0 project file\n"
            "FBXHeaderExtension: {\n"
            "  FBXHeaderVersion: 1003\n"
            "  FBXVersion: 7400\n"
            "}\n"
            "Objects: 7, \"scene\", Bare, *3 { a: 1, 2, 3 } {\n"
            "  Model: 42, \"Cube\"\n"
            "}\n");
        const Pipeline::FbxFile text = Pipeline::ReadFbxFile(textBytes);
        const Pipeline::FbxNode* header = text.Find("FBXHeaderExtension");
        const Pipeline::FbxNode* objects = text.Find("Objects");
        check(text.version == 7400u && !text.binary && text.nodes.size() == 2u
                  && header != nullptr && objects != nullptr && text.Find("objects") == nullptr
                  && header->Find("FBXVersion") != nullptr && objects->Find("Model") != nullptr
                  && objects->Find("Missing") == nullptr,
              "the text reader preserves version, ordered hierarchy and exact-case top-level and child lookup");

        check(objects != nullptr && objects->properties.size() == 4u
                  && objects->Number(0u, -1.0) == 7.0
                  && objects->Number(1u, -1.0) == -1.0
                  && objects->Number(99u, -2.0) == -2.0
                  && objects->Text(1u, "missing") == "scene"
                  && objects->Text(2u, "missing") == "Bare"
                  && objects->Text(0u, "fallback") == "fallback"
                  && objects->Numbers() == std::vector<double>{7.0, 1.0, 2.0, 3.0},
              "typed accessors distinguish scalars and text, honor fallbacks and flatten scalar/array numbers in order");

        const Pipeline::FbxFile binary = Pipeline::ReadFbxFile(MakeBinaryFixture());
        const Pipeline::FbxNode* binaryNode = binary.Find("Node");
        check(binary.version == 7400u && binary.binary && binary.nodes.size() == 1u
                  && binaryNode != nullptr && binaryNode->properties.size() == 4u
                  && std::holds_alternative<double>(binaryNode->properties[0])
                  && std::holds_alternative<std::string>(binaryNode->properties[1])
                  && std::holds_alternative<std::vector<double>>(binaryNode->properties[2])
                  && binaryNode->Text(1u) == "binary"
                  && binaryNode->Numbers()
                      == std::vector<double>{4.5, 1.0, 2.0, 3.0, 1.25, -2.5, 9.0},
              "the binary reader unifies scalar, string, plain-array and zlib-array properties in declared order");

        const std::array errorValues{
            Pipeline::FbxFileError::CannotInitialize,
            Pipeline::FbxFileError::NotFbx,
            Pipeline::FbxFileError::ParseError,
            Pipeline::FbxFileError::Unsupported,
        };
        bool exceptionValues = true;
        for (const Pipeline::FbxFileError value : errorValues)
        {
            const Pipeline::FbxFileException error(value, "specific detail");
            exceptionValues = exceptionValues && error.Error() == value
                && std::string_view(error.what()) == "specific detail";
        }
        check(exceptionValues,
              "all four public error values retain their exact identity and owned diagnostic detail");

        const std::vector<std::uint8_t> empty;
        check(ErrorFrom(empty) == Pipeline::FbxFileError::CannotInitialize
                  && ErrorFrom(Bytes("xof 0303txt 0032")) == Pipeline::FbxFileError::NotFbx
                  && ErrorFrom(Bytes("ordinary unrelated bytes"))
                      == Pipeline::FbxFileError::CannotInitialize
                  && ErrorFrom(Bytes("; FBX\nFBXVersion: 7400\nBroken"))
                      == Pipeline::FbxFileError::ParseError,
              "empty, recognized foreign, unrecognized and malformed FBX inputs keep distinct public failures");

        Pipeline::FbxFileLimits tight = defaults;
        tight.maximumBytes = 16u;
        const bool byteLimit = ErrorFrom(textBytes, tight) == Pipeline::FbxFileError::ParseError;
        tight = defaults;
        tight.maximumDepth = 0u;
        const bool depthLimit = ErrorFrom(Bytes(
            "; FBX\nFBXVersion: 7400\nRoot: { Child: { } }\n"), tight)
            == Pipeline::FbxFileError::ParseError;
        tight = defaults;
        tight.maximumNodes = 1u;
        const bool nodeLimit = ErrorFrom(Bytes(
            "; FBX\nFBXVersion: 7400\nFirst: 1\nSecond: 2\n"), tight)
            == Pipeline::FbxFileError::ParseError;
        tight = defaults;
        tight.maximumArrayLength = 2u;
        const bool arrayLimit = ErrorFrom(Bytes(
            "; FBX\nFBXVersion: 7400\nValues: *3 { a: 1,2,3 }\n"), tight)
            == Pipeline::FbxFileError::ParseError;
        tight = defaults;
        tight.maximumDecompressedBytes = 16u;
        const bool decompressedLimit = ErrorFrom(MakeBinaryFixture(), tight)
            == Pipeline::FbxFileError::ParseError;
        tight = defaults;
        tight.maximumStringLength = 3u;
        const bool stringLimit = ErrorFrom(Bytes(
            "; FBX\nFBXVersion: 7400\nLonger: 1\n"), tight)
            == Pipeline::FbxFileError::ParseError;
        check(byteLimit && depthLimit && nodeLimit && arrayLimit && decompressedLimit
                  && stringLimit,
              "each caller-selected byte, depth, node, array, decompressed-byte and string ceiling refuses before overuse");

        std::cout << "Read text and binary FBX node trees with plain and compressed arrays.\n";
        return passed ? 0 : 1;
    }
}

int main()
{
    try
    {
        return Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "FBX file-reader audit failed: " << error.what() << '\n';
        return 1;
    }
}
