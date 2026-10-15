// SPDX-License-Identifier: MS-PL
// Audit CNA's public text and binary DirectX .x reader.
#include <array>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "CNA/Content/Pipeline/DirectXFileReader.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace
{
    namespace Pipeline = CNA::Content::Pipeline;
    using Microsoft::Xna::Framework::TitleLocation;

    std::vector<std::uint8_t> ReadBytes(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        const std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                                      std::istreambuf_iterator<char>());
        return {bytes.begin(), bytes.end()};
    }

    std::vector<std::uint8_t> Bytes(const std::string_view text)
    {
        return {text.begin(), text.end()};
    }

    const Pipeline::DirectXFileObject* FindChild(
        const Pipeline::DirectXFileObject& parent, const std::string_view type,
        const std::string_view name = {})
    {
        for (const auto& child : parent.children)
        {
            if (child.TypeIs(type) && (name.empty() || child.name == name)) return &child;
        }
        return nullptr;
    }

    void Put16(std::vector<std::uint8_t>& bytes, const std::uint16_t value)
    {
        bytes.push_back(static_cast<std::uint8_t>(value & 0xFFu));
        bytes.push_back(static_cast<std::uint8_t>(value >> 8u));
    }

    void Put32(std::vector<std::uint8_t>& bytes, const std::uint32_t value)
    {
        for (unsigned int shift = 0u; shift < 32u; shift += 8u)
            bytes.push_back(static_cast<std::uint8_t>(value >> shift));
    }

    void PutName(std::vector<std::uint8_t>& bytes, const std::string_view text)
    {
        Put16(bytes, 1u);
        Put32(bytes, static_cast<std::uint32_t>(text.size()));
        bytes.insert(bytes.end(), text.begin(), text.end());
    }

    std::vector<std::uint8_t> MakeBinaryFixture()
    {
        std::vector<std::uint8_t> bytes = Bytes("xof 0303bin 0032");

        Put16(bytes, 31u); // template Vector { <guid> FLOAT x; array FLOAT more[3]; }
        PutName(bytes, "Vector");
        Put16(bytes, 10u);
        Put16(bytes, 5u);
        bytes.insert(bytes.end(), 16u, 0u);
        Put16(bytes, 42u);
        PutName(bytes, "x");
        Put16(bytes, 20u);
        Put16(bytes, 52u);
        Put16(bytes, 42u);
        PutName(bytes, "more");
        Put16(bytes, 14u);
        Put16(bytes, 3u);
        Put32(bytes, 3u);
        Put16(bytes, 15u);
        Put16(bytes, 20u);
        Put16(bytes, 11u);

        PutName(bytes, "Vector");
        PutName(bytes, "Here");
        Put16(bytes, 10u);
        Put16(bytes, 7u);
        Put32(bytes, 3u);
        for (const float value : {1.0f, 2.0f, 3.0f})
        {
            std::uint32_t raw = 0u;
            std::memcpy(&raw, &value, sizeof(raw));
            Put32(bytes, raw);
        }
        Put16(bytes, 11u);
        return bytes;
    }

    std::optional<Pipeline::DirectXFileError> ErrorFrom(
        const std::span<const std::uint8_t> bytes,
        const Pipeline::DirectXFileLimits& limits = {})
    {
        try
        {
            (void) Pipeline::ReadDirectXFile(bytes, limits);
        }
        catch (const Pipeline::DirectXFileException& error)
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

        const Pipeline::DirectXFileObject safeObject;
        const Pipeline::DirectXFile safeFile;
        const Pipeline::DirectXFileLimits defaults;
        check(safeObject.type.empty() && safeObject.name.empty() && safeObject.numbers.empty()
                  && safeObject.strings.empty() && safeObject.children.empty()
                  && safeObject.references.empty() && safeFile.encoding.empty()
                  && safeFile.objects.empty() && defaults.maximumBytes == 256u * 1024u * 1024u
                  && defaults.maximumDepth == 64u && defaults.maximumObjects == 1000000u
                  && defaults.maximumNumbersPerObject == 64u * 1024u * 1024u
                  && defaults.maximumStringLength == 65536u,
              "object, file and all five resource ceilings expose their safe documented defaults");

        const auto asset = std::filesystem::path(TitleLocation::Path()) / "assets" / "models"
            / "directx_twin_panels.x";
        const Pipeline::DirectXFile text = Pipeline::ReadDirectXFile(ReadBytes(asset));
        const Pipeline::DirectXFileObject* rig =
            text.objects.size() == 1u ? &text.objects.front() : nullptr;
        check(text.encoding == "txt" && rig != nullptr && rig->TypeIs("frame")
                  && !rig->TypeIs("frames") && rig->name == "PanelRig"
                  && rig->children.size() == 3u,
              "the generated text fixture yields one case-insensitive named frame and its three ordered children");

        const Pipeline::DirectXFileObject* left =
            rig != nullptr ? FindChild(*rig, "FRAME", "LeftMount") : nullptr;
        const Pipeline::DirectXFileObject* right =
            rig != nullptr ? FindChild(*rig, "Frame", "RightMount") : nullptr;
        const Pipeline::DirectXFileObject* leftMesh =
            left != nullptr ? FindChild(*left, "mesh", "LeftPanel") : nullptr;
        const Pipeline::DirectXFileObject* rightMesh =
            right != nullptr ? FindChild(*right, "MESH", "RightPanel") : nullptr;
        check(left != nullptr && right != nullptr && left->children.size() == 2u
                  && right->children.size() == 2u && leftMesh != nullptr && rightMesh != nullptr
                  && leftMesh->numbers.size() == 22u && rightMesh->numbers.size() == 22u
                  && leftMesh->numbers[1] == -0.25 && rightMesh->numbers[3] == -0.5,
              "nested frame and mesh objects preserve authored names, order and flattened numeric payloads");

        const auto carriers = Bytes(
            "xof 0303txt 0032\n"
            "template Ignored { <00000000-0000-0000-0000-000000000000> DWORD value; }\n"
            "Material Holder { \"texture.png\"; { SharedMaterial } Child C { 1; -2.5; } }\n");
        const Pipeline::DirectXFile carrierFile = Pipeline::ReadDirectXFile(carriers);
        const auto& holder = carrierFile.objects.front();
        check(carrierFile.objects.size() == 1u && holder.type == "Material"
                  && holder.name == "Holder"
                  && holder.strings == std::vector<std::string>{"texture.png"}
                  && holder.references == std::vector<std::string>{"SharedMaterial"}
                  && holder.children.size() == 1u && holder.children[0].type == "Child"
                  && holder.children[0].name == "C"
                  && holder.children[0].numbers == std::vector<double>{1.0, -2.5},
              "text templates are omitted while strings, references and nested numeric objects retain exact order");

        const Pipeline::DirectXFile binary = Pipeline::ReadDirectXFile(MakeBinaryFixture());
        check(binary.encoding == "bin" && binary.objects.size() == 1u
                  && binary.objects[0].type == "Vector" && binary.objects[0].name == "Here"
                  && binary.objects[0].numbers == std::vector<double>{1.0, 2.0, 3.0},
              "a binary token stream skips its inline array template and yields the same flattened object model");

        const std::array errorCases{
            std::pair{Pipeline::DirectXFileError::BadFile, "D3DXFERR_BADFILE"},
            std::pair{Pipeline::DirectXFileError::BadFileType, "D3DXFERR_BADFILETYPE"},
            std::pair{Pipeline::DirectXFileError::BadFileVersion, "D3DXFERR_BADFILEVERSION"},
            std::pair{Pipeline::DirectXFileError::ParseError, "D3DXFERR_PARSEERROR"},
        };
        bool exceptionValues = true;
        for (const auto& [value, code] : errorCases)
        {
            const Pipeline::DirectXFileException error(value, "specific detail");
            exceptionValues = exceptionValues && error.Error() == value
                && std::string_view(error.CodeName()) == code
                && std::string_view(error.what()) == "specific detail";
        }
        check(exceptionValues,
              "all four error values retain their D3DX code names and owned diagnostic detail");

        const std::vector<std::uint8_t> empty;
        check(ErrorFrom(empty) == Pipeline::DirectXFileError::BadFile
                  && ErrorFrom(Bytes("not an x file")) == Pipeline::DirectXFileError::BadFileType
                  && ErrorFrom(Bytes("xof 9999txt 0032"))
                      == Pipeline::DirectXFileError::BadFileVersion
                  && ErrorFrom(Bytes("xof 0303tzip0032"))
                      == Pipeline::DirectXFileError::BadFileVersion
                  && ErrorFrom(Bytes("xof 0303txt 0032 Frame Open {"))
                      == Pipeline::DirectXFileError::ParseError,
              "empty, foreign, unknown-version, compressed and truncated inputs keep distinct public failures");

        Pipeline::DirectXFileLimits tight = defaults;
        tight.maximumBytes = 16u;
        const bool byteLimit = ErrorFrom(carriers, tight) == Pipeline::DirectXFileError::ParseError;
        tight = defaults;
        tight.maximumDepth = 0u;
        const bool depthLimit = ErrorFrom(Bytes("xof 0303txt 0032 Root { Child { } }"), tight)
            == Pipeline::DirectXFileError::ParseError;
        tight = defaults;
        tight.maximumObjects = 1u;
        const bool objectLimit = ErrorFrom(Bytes("xof 0303txt 0032 A { } B { }"), tight)
            == Pipeline::DirectXFileError::ParseError;
        tight = defaults;
        tight.maximumNumbersPerObject = 1u;
        const bool numberLimit = ErrorFrom(Bytes("xof 0303txt 0032 A { 1; 2; }"), tight)
            == Pipeline::DirectXFileError::ParseError;
        tight = defaults;
        tight.maximumStringLength = 3u;
        const bool stringLimit = ErrorFrom(Bytes("xof 0303txt 0032 Longer { }"), tight)
            == Pipeline::DirectXFileError::ParseError;
        check(byteLimit && depthLimit && objectLimit && numberLimit && stringLimit,
              "each caller-selected byte, depth, object, number and string ceiling refuses before overuse");

        check(ErrorFrom(Bytes("xof 0303bin 0064"))
                  == Pipeline::DirectXFileError::BadFileVersion,
              "binary 64-bit-float input is explicitly refused instead of being silently misread");

        std::cout << "Read text and binary DirectX object trees with bounded public failures.\n";
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
        std::cerr << "DirectX file-reader audit failed: " << error.what() << '\n';
        return 1;
    }
}
