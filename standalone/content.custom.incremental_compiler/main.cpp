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
#include <utility>
#include <vector>
#include <variant>

#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/ContentBuildManifest.hpp"
#include "CNA/Content/Pipeline/ContentBuildConfiguration.hpp"
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
    void WriteFile(const std::filesystem::path& path, std::string_view text)
    {
        std::ofstream file(path, std::ios::binary);
        file.write(text.data(), static_cast<std::streamsize>(text.size())); file.close();
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
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.ScoreProcessor", "2"}; }
        std::string InputType() const override { return SourceType; }
        std::string OutputType() const override { return ScoreType; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            for (const auto& [name, value] : parameters.Values())
            {
                const auto* bonus = std::get_if<std::int64_t>(&value);
                if (name != "Bonus" || !bonus || *bonus < 0 || *bonus > 1000)
                    throw std::invalid_argument("ScoreProcessor accepts only an i64 Bonus in 0..1000.");
            }
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            ++processes;
            const auto multiplier = context.ResolveSourceDependency("multiplier.factor");
            const auto* bonus = context.Parameters().Find("Bonus");
            const int addition = bonus ? static_cast<int>(std::get<std::int64_t>(*bonus)) : 0;
            return Pipeline::ContentValue::Create(ScoreType, input.Get<int>() * ReadPositive(multiplier) + addition);
        }
    };
    class ScoreWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        explicit ScoreWriter(std::string version) : version_(std::move(version)) {}
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.ScoreWriter", version_}; }
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
    private:
        std::string version_;
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
            WriteFile(source / "arena.score", "5\n"); WriteFile(source / "menu.score", "8\n");
            WriteFile(source / "multiplier.factor", "2\n");
            std::string writerVersion = "1";
            std::shared_ptr<Pipeline::ContentPipelineRegistry> latestRegistry;
            const Pipeline::ContentPipelineRegistryFactory factory = [&](const Pipeline::ContentCompilerOptions&) {
                ++factories;
                auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
                registry->RegisterImporter(std::make_shared<ScoreImporter>());
                registry->RegisterProcessor(std::make_shared<ScoreProcessor>());
                registry->RegisterWriter(std::make_shared<ScoreWriter>(writerVersion));
                latestRegistry = registry; // retained only for the visible post-build freeze check
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
            bool lateRegistration = false;
            try { latestRegistry->RegisterWriter(std::make_shared<ScoreWriter>("late")); }
            catch (const std::logic_error& error)
            {
                lateRegistration = std::string(error.what()).find("frozen") != std::string::npos;
                std::cout << "Late registration refused: " << error.what() << '\n';
            }
            check(latestRegistry->IsFrozen() && lateRegistration,
                  "accepting a registry freezes it and later component registration is explicitly refused");
            const int unchanged = Pipeline::RunContentCompiler(arguments, factory);
            check(unchanged == 0 && factories == 2 && imports == 2 && processes == 2 && writes == 2 &&
                  ReadText(manifestPath) == firstManifestText,
                  "a fresh registry invocation skips unchanged inputs without entering any component again");
            WriteFile(source / "arena.score", "7\n");
            const int edited = Pipeline::RunContentCompiler(arguments, factory);
            check(edited == 0 && imports == 3 && processes == 3 && writes == 3 &&
                  ReadScore(output / "arena.cnb") == 14 && ReadScore(output / "menu.cnb") == 16,
                  "editing one primary source rebuilds only its node and retains the other score");
            WriteFile(source / "multiplier.factor", "3\n");
            const int dependency = Pipeline::RunContentCompiler(arguments, factory);
            check(dependency == 0 && imports == 5 && processes == 5 && writes == 5 &&
                  ReadScore(output / "arena.cnb") == 21 && ReadScore(output / "menu.cnb") == 24,
                  "changing a shared recorded source dependency rebuilds both affected nodes");
            const auto validArena = ReadText(output / "arena.cnb"), validManifest = ReadText(manifestPath);
            WriteFile(source / "arena.score", "invalid\n");
            const int failed = Pipeline::RunContentCompiler(arguments, factory);
            check(failed == 1 && imports == 6 && processes == 5 && writes == 5 &&
                  ReadText(output / "arena.cnb") == validArena && ReadText(manifestPath) == validManifest,
                  "a refused import leaves the last valid published artifact and manifest intact");
            WriteFile(source / "arena.score", "9\n");
            const int repaired = Pipeline::RunContentCompiler(arguments, factory);
            check(repaired == 0 && imports == 7 && processes == 6 && writes == 6 && ReadScore(output / "arena.cnb") == 27,
                  "repairing the source permits a subsequent successful rebuild");
            std::filesystem::remove(output / "menu.cnb");
            const int missing = Pipeline::RunContentCompiler(arguments, factory);
            check(missing == 0 && imports == 8 && processes == 7 && writes == 7 && ReadScore(output / "menu.cnb") == 24,
                  "deleting an owned output invalidates its skip and recreates the correct artifact");
            const auto beforeVersion = Pipeline::ContentBuildManifest::Parse(ReadText(manifestPath));
            const auto originalArena = ReadText(output / "arena.cnb"), originalMenu = ReadText(output / "menu.cnb");
            writerVersion = "2"; // same encoding, a new declared implementation identity
            const int versioned = Pipeline::RunContentCompiler(arguments, factory);
            const auto afterVersion = Pipeline::ContentBuildManifest::Parse(ReadText(manifestPath));
            const auto* oldArena = beforeVersion.Find("arena"); const auto* newArena = afterVersion.Find("arena");
            check(versioned == 0 && imports == 10 && processes == 9 && writes == 9 && oldArena && newArena &&
                  newArena->writer.version == "2" && oldArena->fingerprint != newArena->fingerprint &&
                  ReadText(output / "arena.cnb") == originalArena && ReadText(output / "menu.cnb") == originalMenu,
                  "a changed writer version invalidates both nodes even when its encoding produces identical bytes");
            auto tampered = originalArena; tampered.back() = static_cast<char>(tampered.back() ^ 1);
            const auto stamp = std::filesystem::last_write_time(output / "arena.cnb");
            WriteFile(output / "arena.cnb", tampered);
            std::filesystem::last_write_time(output / "arena.cnb", stamp);
            const int restored = Pipeline::RunContentCompiler(arguments, factory);
            check(restored == 0 && imports == 11 && processes == 10 && writes == 10 &&
                  ReadText(output / "arena.cnb") == originalArena && ReadText(output / "menu.cnb") == originalMenu,
                  "changed output bytes force a rebuild despite preserved file length and modification time");
            WriteFile(manifestPath, "invalid manifest\n");
            const int recovered = Pipeline::RunContentCompiler(arguments, factory);
            const auto recoveredManifest = Pipeline::ContentBuildManifest::Parse(ReadText(manifestPath));
            check(recovered == 0 && imports == 13 && processes == 12 && writes == 12 &&
                  recoveredManifest.Entries().size() == 2 && ReadScore(output / "arena.cnb") == 27 && ReadScore(output / "menu.cnb") == 24,
                  "a corrupt manifest triggers fresh node builds and is replaced by a valid manifest");
            WriteFile(output / "manual.txt", "Unowned author note.\n");
            const auto configurationPath = source / Pipeline::ContentBuildConfigurationFileName;
            const std::string configuration = R"json({"format":"CNA.ContentPipeline.Config","version":1,"assets":{"arena.score":{"parameters":{"Bonus":{"type":"i64","value":"5"}}}}})json";
            WriteFile(configurationPath, configuration);
            const auto parsedConfiguration = Pipeline::ContentBuildConfiguration::Parse(configuration, configurationPath);
            const auto* settings = parsedConfiguration.Find("arena.score");
            const auto* configuredBonus = settings ? std::get_if<std::int64_t>(settings->parameters.Find("Bonus")) : nullptr;
            const int configured = Pipeline::RunContentCompiler(arguments, factory);
            const auto configuredManifestText = ReadText(manifestPath);
            const auto configuredManifest = Pipeline::ContentBuildManifest::Parse(configuredManifestText);
            const auto* configuredArena = configuredManifest.Find("arena");
            const auto* recoveredArena = recoveredManifest.Find("arena");
            const auto* recordedBonus = configuredArena ? std::get_if<std::int64_t>(configuredArena->parameters.Find("Bonus")) : nullptr;
            check(configured == 0 && configuredBonus && *configuredBonus == 5 && recordedBonus && *recordedBonus == 5 &&
                  configuredArena && recoveredArena && configuredArena->fingerprint != recoveredArena->fingerprint &&
                  imports == 14 && processes == 13 && writes == 13 && ReadScore(output / "arena.cnb") == 32 &&
                  ReadText(output / "menu.cnb") == originalMenu,
                  "a typed per-asset configuration changes its node's parameter fingerprint and rebuilds only that score");
            WriteFile(configurationPath, " \n" + configuration + "\n");
            const int reformatted = Pipeline::RunContentCompiler(arguments, factory);
            check(reformatted == 0 && imports == 14 && processes == 13 && writes == 13 &&
                  ReadText(manifestPath) == configuredManifestText,
                  "rewriting only configuration whitespace preserves the effective parameters and skips both nodes");
            auto changedConfiguration = configuration;
            changedConfiguration.replace(changedConfiguration.find("\"5\""), 3, "\"6\"");
            WriteFile(configurationPath, changedConfiguration);
            const int changedParameter = Pipeline::RunContentCompiler(arguments, factory);
            const auto changedManifest = Pipeline::ContentBuildManifest::Parse(ReadText(manifestPath));
            const auto* changedArena = changedManifest.Find("arena");
            const auto* changedBonus = changedArena ? std::get_if<std::int64_t>(changedArena->parameters.Find("Bonus")) : nullptr;
            check(changedParameter == 0 && changedBonus && *changedBonus == 6 && configuredArena && changedArena &&
                  changedArena->fingerprint != configuredArena->fingerprint && imports == 15 && processes == 14 && writes == 14 &&
                  ReadScore(output / "arena.cnb") == 33 && ReadText(output / "menu.cnb") == originalMenu,
                  "changing the effective Bonus parameter rebuilds its score while retaining the unconfigured peer");
            std::filesystem::remove(source / "menu.score");
            const int retired = Pipeline::RunContentCompiler(arguments, factory);
            const auto retiredManifest = Pipeline::ContentBuildManifest::Parse(ReadText(manifestPath));
            check(retired == 0 && imports == 15 && processes == 14 && writes == 14 &&
                  !std::filesystem::exists(output / "menu.cnb") && retiredManifest.Entries().size() == 1 &&
                  !retiredManifest.Find("menu") && ReadScore(output / "arena.cnb") == 33 &&
                  ReadText(output / "manual.txt") == "Unowned author note.\n",
                  "removing a source collects only its proven obsolete output and retains the live node and unowned file");
            WriteFile(source / "menu.score", "8\n");
            const int returned = Pipeline::RunContentCompiler(arguments, factory);
            check(returned == 0 && imports == 16 && processes == 15 && writes == 15 &&
                  Pipeline::ContentBuildManifest::Parse(ReadText(manifestPath)).Entries().size() == 2 &&
                  ReadText(output / "menu.cnb") == originalMenu && ReadScore(output / "arena.cnb") == 33,
                  "restoring the source publishes its node again without rebuilding the retained configured score");
            const auto cleanManifest = ReadText(manifestPath), cleanArena = ReadText(output / "arena.cnb");
            WriteFile(output / "arena.cnb", "Hand-edited compiled content.\n");
            const int guarded = Pipeline::RunContentCompiler({"clean", output}, factory);
            check(guarded == 1 && ReadText(output / "arena.cnb") == "Hand-edited compiled content.\n" &&
                  ReadText(output / "menu.cnb") == originalMenu && ReadText(manifestPath) == cleanManifest,
                  "clean refuses changed owned bytes before deleting an untouched peer or the ownership manifest");
            // Restore the exact owned bytes only after observing refusal, then exercise normal clean independently.
            WriteFile(output / "arena.cnb", cleanArena);
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
