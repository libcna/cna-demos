// SPDX-License-Identifier: MS-PL
// Embed CNA's actual content compiler: callbacks show when its manifest skips or rebuilds work.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/ContentBuildManifest.hpp"
#include "CNA/Content/Pipeline/ContentCompiler.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;
namespace
{
    constexpr const char* SourceType = "CnaDemos.ScoreSource";
    constexpr const char* ScoreType = "CnaDemos.CompiledScore";
    constexpr auto DataChunk = Cnb::MakeChunkId('D', 'A', 'T', 'A');
    std::atomic<int> imports{0}, processes{0}, writes{0}, factories{0};
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    int ReadPositive(const std::filesystem::path& path)
    {
        std::ifstream file(path); int value = 0;
        if (!(file >> value) || value < 1 || value > 1000)
            throw std::runtime_error("Expected one score or multiplier in 1..1000: " + path.filename().string());
        file >> std::ws;
        if (!file.eof()) throw std::runtime_error("Unexpected score fields.");
        return value;
    }
    void WriteText(const std::filesystem::path& path, std::string_view text)
    {
        std::ofstream file(path); file << text; file.close();
        if (!file) throw std::runtime_error("Could not write the original demo input.");
    }
    std::string ReadText(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Could not read the compiler artifact.");
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
    class ScoreImporter final : public Pipeline::ContentImporter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.ScoreImporter", "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".score"}; }
        std::vector<std::string> OutputTypes() const override { return {SourceType}; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            ++imports;
            return Pipeline::ContentValue::Create(SourceType, ReadPositive(context.SourcePath()));
        }
    };
    class ScoreProcessor final : public Pipeline::ContentProcessor
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.ScoreProcessor", "1"}; }
        std::string InputType() const override { return SourceType; }
        std::string OutputType() const override { return ScoreType; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            if (!parameters.Values().empty()) throw std::invalid_argument("ScoreProcessor accepts no parameters.");
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            ++processes;
            const auto multiplier = context.ResolveSourceDependency("multiplier.factor");
            return Pipeline::ContentValue::Create(ScoreType, input.Get<int>() * ReadPositive(multiplier));
        }
    };
    class ScoreWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.ScoreWriter", "1"}; }
        std::string InputType() const override { return ScoreType; }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(ScoreType), 1u, ScoreType, {"CnaDemos.ScoreCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input, const std::string& name) const override
        {
            ++writes;
            Cnb::CnbByteWriter data; data.WriteU32(static_cast<std::uint32_t>(input.Get<int>()));
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(ScoreType), 1);
            writer.SetMetadata(ScoreType, name); writer.AddChunk(DataChunk, data.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build(); result.assetTypeId = Cnb::CnbAssetTypeIdFromName(ScoreType);
            result.assetTypeName = ScoreType; result.assetSchemaVersion = 1;
            return result;
        }
    };
    std::uint32_t ReadScore(const std::filesystem::path& path)
    {
        const auto document = Cnb::CnbDocument::ParseFile(path.string());
        document.RequireAsset(Cnb::CnbAssetTypeIdFromName(ScoreType), 1);
        auto data = document.OpenChunk(document.RequireSingle(DataChunk));
        const auto score = data.ReadU32(); data.RequireExhausted(); return score;
    }
}
int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = passed && condition;
        };
        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-incremental-scores-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not reserve a private compiler workspace.");
        {
            TemporaryDirectory cleanup{directory};
            std::cout << "Private compiler data: " << directory << '\n';
            const auto source = directory / "source", output = directory / "output";
            std::filesystem::create_directory(source);
            WriteText(source / "arena.score", "5\n"); WriteText(source / "menu.score", "8\n");
            WriteText(source / "multiplier.factor", "2\n");
            const Pipeline::ContentPipelineRegistryFactory factory = [](const Pipeline::ContentCompilerOptions&) {
                ++factories;
                auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
                registry->RegisterImporter(std::make_shared<ScoreImporter>());
                registry->RegisterProcessor(std::make_shared<ScoreProcessor>());
                registry->RegisterWriter(std::make_shared<ScoreWriter>());
                return registry;
            };
            const std::vector<std::filesystem::path> arguments{
                "build", source, "-o", output, "--format", "cnb", "--workers", "1"};
            const auto manifestPath = output / Pipeline::ContentBuildManifestFileName;
            const int initial = Pipeline::RunContentCompiler(arguments, factory);
            check(initial == 0 && imports == 2 && processes == 2 && writes == 2 &&
                  ReadScore(output / "arena.cnb") == 10 && ReadScore(output / "menu.cnb") == 16,
                  "the real compiler discovers two original score sources and publishes their processed CNB values");
            const auto firstManifestText = ReadText(manifestPath);
            const auto firstManifest = Pipeline::ContentBuildManifest::Parse(firstManifestText);
            const auto* arena = firstManifest.Find("arena");
            check(firstManifest.Entries().size() == 2 && arena && arena->outputs.size() == 1 &&
                  arena->outputs.front().sha256 == Pipeline::ContentFileSha256(output / "arena.cnb"),
                  "the actual compiler manifest records both nodes and the published arena artifact's digest");
            const int unchanged = Pipeline::RunContentCompiler(arguments, factory);
            check(unchanged == 0 && factories == 2 && imports == 2 && processes == 2 && writes == 2 &&
                  ReadText(manifestPath) == firstManifestText,
                  "a fresh registry invocation skips unchanged inputs without entering any component again");
            WriteText(source / "arena.score", "7\n");
            const int edited = Pipeline::RunContentCompiler(arguments, factory);
            check(edited == 0 && imports == 3 && processes == 3 && writes == 3 &&
                  ReadScore(output / "arena.cnb") == 14 && ReadScore(output / "menu.cnb") == 16,
                  "editing one primary source rebuilds only its node and retains the other score");
            WriteText(source / "multiplier.factor", "3\n");
            const int dependency = Pipeline::RunContentCompiler(arguments, factory);
            check(dependency == 0 && imports == 5 && processes == 5 && writes == 5 &&
                  ReadScore(output / "arena.cnb") == 21 && ReadScore(output / "menu.cnb") == 24,
                  "changing a shared recorded source dependency rebuilds both affected nodes");
            const auto validArena = ReadText(output / "arena.cnb"), validManifest = ReadText(manifestPath);
            WriteText(source / "arena.score", "invalid\n");
            const int failed = Pipeline::RunContentCompiler(arguments, factory);
            check(failed == 1 && imports == 6 && processes == 5 && writes == 5 &&
                  ReadText(output / "arena.cnb") == validArena && ReadText(manifestPath) == validManifest,
                  "a refused import leaves the last valid published artifact and manifest intact");
            WriteText(source / "arena.score", "9\n");
            const int repaired = Pipeline::RunContentCompiler(arguments, factory);
            check(repaired == 0 && imports == 7 && processes == 6 && writes == 6 && ReadScore(output / "arena.cnb") == 27,
                  "repairing the source permits a subsequent successful rebuild");
            std::filesystem::remove(output / "menu.cnb");
            const int missing = Pipeline::RunContentCompiler(arguments, factory);
            check(missing == 0 && imports == 8 && processes == 7 && writes == 7 && ReadScore(output / "menu.cnb") == 24,
                  "deleting an owned output invalidates its skip and recreates the correct artifact");
            WriteText(output / "manual.txt", "Unowned author note.\n");
            const int cleaned = Pipeline::RunContentCompiler({"clean", output}, factory);
            check(cleaned == 0 && !std::filesystem::exists(output / "arena.cnb") && !std::filesystem::exists(output / "menu.cnb") &&
                  ReadText(output / "manual.txt") == "Unowned author note.\n" && std::filesystem::exists(source / "arena.score"),
                  "compiler clean removes its owned compiled artifacts while preserving unowned files and authored sources");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private sources, outputs and remaining author note");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Incremental content compiler failed: " << error.what() << '\n';
        return 1;
    }
}
