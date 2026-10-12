// SPDX-License-Identifier: MS-PL
// One source owns primary/additional CNBs and a deployed support file through the real compiler.
#include <algorithm>
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
#include <tuple>
#include <utility>
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
    constexpr const char* SourceType = "CnaDemos.EncounterSource";
    constexpr const char* EncounterType = "CnaDemos.EncounterWaves";
    constexpr const char* RewardsType = "CnaDemos.EncounterRewards";
    constexpr auto DataChunk = Cnb::MakeChunkId('D', 'A', 'T', 'A');
    std::atomic<int> writes{0};
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    void WriteText(const std::filesystem::path& path, std::string_view text)
    {
        std::ofstream file(path, std::ios::binary);
        file.write(text.data(), static_cast<std::streamsize>(text.size())); file.close();
        if (!file) throw std::runtime_error("Could not write the original encounter fixture.");
    }
    std::string ReadText(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Could not open the compiler artifact.");
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
    class EncounterImporter final : public Pipeline::ContentImporter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.EncounterImporter", "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".encounter"}; }
        std::vector<std::string> OutputTypes() const override { return {SourceType}; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            std::ifstream file(context.SourcePath()); std::uint32_t waves = 0;
            if (!(file >> waves) || waves < 1 || waves > 100)
                throw std::runtime_error("An encounter needs 1..100 waves.");
            file >> std::ws;
            if (!file.eof()) throw std::runtime_error("An encounter contains one wave count.");
            return Pipeline::ContentValue::Create(SourceType, waves);
        }
    };
    class EncounterProcessor final : public Pipeline::ContentProcessor
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.EncounterProcessor", "1"}; }
        std::string InputType() const override { return SourceType; }
        std::string OutputType() const override { return EncounterType; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            if (!parameters.Values().empty()) throw std::invalid_argument("The encounter has no processor parameters.");
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            // A source support file is deployed alongside the CNBs. This native API accepts a
            // contained source, unlike the separate generated-output case in the XNA adapter demo.
            context.AddDeploymentFile(context.SourceRoot() / "briefing.txt", "support/briefing.txt");
            context.AddRuntimeReference("rewards/" + context.LogicalName(), Cnb::CnbAssetTypeIdFromName(RewardsType));
            return Pipeline::ContentValue::Create(EncounterType, input.Get<std::uint32_t>());
        }
    };
    class EncounterWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.EncounterWriter", "1"}; }
        std::string InputType() const override { return EncounterType; }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            std::vector<Pipeline::ContentWriterSchemaIdentity> schemas{
                {Cnb::CnbAssetTypeIdFromName(EncounterType), 1u, EncounterType, {"CnaDemos.EncounterCodec", "1"}},
                {Cnb::CnbAssetTypeIdFromName(RewardsType), 1u, RewardsType, {"CnaDemos.RewardsCodec", "1"}}};
            std::sort(schemas.begin(), schemas.end(), [](const auto& a, const auto& b) {
                return std::tie(a.assetTypeId, a.assetTypeName, a.assetSchemaVersion) <
                       std::tie(b.assetTypeId, b.assetTypeName, b.assetSchemaVersion);
            });
            return schemas;
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input, const std::string& name) const override
        {
            ++writes;
            const auto waves = input.Get<std::uint32_t>();
            const std::string rewardsName = "rewards/" + name;
            Cnb::CnbByteWriter encounter; encounter.WriteU32(waves);
            Cnb::CnbWriter primary(Cnb::CnbAssetTypeIdFromName(EncounterType), 1);
            primary.SetMetadata(EncounterType, name);
            primary.SetExternalReferences({{0u, Cnb::CnbAssetTypeIdFromName(RewardsType), rewardsName}});
            primary.AddChunk(DataChunk, encounter.Take(), Cnb::CnbChunkFlags::Mandatory);
            Cnb::CnbByteWriter reward; reward.WriteU32(waves * 25u);
            Cnb::CnbWriter companion(Cnb::CnbAssetTypeIdFromName(RewardsType), 1);
            companion.SetMetadata(RewardsType, rewardsName);
            companion.AddChunk(DataChunk, reward.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = primary.Build(); result.assetTypeId = Cnb::CnbAssetTypeIdFromName(EncounterType);
            result.assetTypeName = EncounterType; result.assetSchemaVersion = 1;
            Pipeline::ContentAdditionalWriteOutput extra;
            extra.logicalName = rewardsName; extra.bytes = companion.Build();
            extra.assetTypeId = Cnb::CnbAssetTypeIdFromName(RewardsType);
            extra.assetTypeName = RewardsType; extra.assetSchemaVersion = 1;
            result.additionalOutputs.push_back(std::move(extra));
            return result;
        }
    };
    std::uint32_t ReadCount(const std::filesystem::path& path, const char* type)
    {
        const auto document = Cnb::CnbDocument::ParseFile(path.string());
        document.RequireAsset(Cnb::CnbAssetTypeIdFromName(type), 1);
        auto data = document.OpenChunk(document.RequireSingle(DataChunk));
        const auto count = data.ReadU32(); data.RequireExhausted(); return count;
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
            ("cna-demos-encounter-publication-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not reserve private compiler data.");
        {
            TemporaryDirectory cleanup{directory};
            std::cout << "Private encounter compiler data: " << directory << '\n';
            const auto source = directory / "source", output = directory / "output";
            std::filesystem::create_directory(source);
            WriteText(source / "arena.encounter", "3\n");
            WriteText(source / "briefing.txt", "Protect the courtyard.\n");
            const Pipeline::ContentPipelineRegistryFactory factory = [](const Pipeline::ContentCompilerOptions&) {
                auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
                registry->RegisterImporter(std::make_shared<EncounterImporter>());
                registry->RegisterProcessor(std::make_shared<EncounterProcessor>());
                registry->RegisterWriter(std::make_shared<EncounterWriter>());
                return registry;
            };
            const std::vector<std::filesystem::path> arguments{
                "build", source, "-o", output, "--format", "cnb", "--workers", "1"};
            const auto primary = output / "arena.cnb", companion = output / "rewards/arena.cnb";
            const auto support = output / "support/briefing.txt";
            const auto manifestPath = output / Pipeline::ContentBuildManifestFileName;
            const int initial = Pipeline::RunContentCompiler(arguments, factory);
            check(initial == 0 && writes == 1 && ReadCount(primary, EncounterType) == 3 &&
                  ReadCount(companion, RewardsType) == 75 && ReadText(support) == ReadText(source / "briefing.txt"),
                  "one encounter build publishes its primary CNB, reward-table CNB and support text");
            const auto manifestText = ReadText(manifestPath);
            const auto manifest = Pipeline::ContentBuildManifest::Parse(manifestText);
            const auto* arena = manifest.Find("arena");
            bool owned = manifest.Entries().size() == 1 && arena && arena->outputs.size() == 2 &&
                arena->writerSchemas.size() == 2 && arena->deploymentFiles.size() == 1;
            if (arena)
            {
                bool primaryOwned = false, companionOwned = false;
                for (const auto& artifact : arena->outputs)
                {
                    owned &= artifact.sha256 == Pipeline::ContentFileSha256(output / artifact.path);
                    primaryOwned |= artifact.logicalName == "arena" && artifact.path == "arena.cnb" &&
                        artifact.assetTypeId == Cnb::CnbAssetTypeIdFromName(EncounterType);
                    companionOwned |= artifact.logicalName == "rewards/arena" && artifact.path == "rewards/arena.cnb" &&
                        artifact.assetTypeId == Cnb::CnbAssetTypeIdFromName(RewardsType);
                }
                owned &= primaryOwned && companionOwned;
                if (arena->deploymentFiles.size() == 1)
                    owned &= arena->deploymentFiles.front().path == "support/briefing.txt" &&
                        arena->deploymentFiles.front().sha256 == Pipeline::ContentFileSha256(support);
            }
            check(owned, "one manifest node owns both declared CNB schemas and the deployed file with actual digests");
            const auto document = Cnb::CnbDocument::ParseFile(primary.string());
            check(document.ExternalReferences().size() == 1 &&
                  document.ExternalReferences().front().logicalName == "rewards/arena" &&
                  document.ExternalReferences().front().expectedAssetTypeId == Cnb::CnbAssetTypeIdFromName(RewardsType) &&
                  arena && arena->runtimeReferences.size() == 1 &&
                  arena->runtimeReferences.front().logicalName == "rewards/arena" &&
                  arena->runtimeReferences.front().expectedAssetTypeId == Cnb::CnbAssetTypeIdFromName(RewardsType),
                  "the primary CNB references its separately published reward table, not the support text");
            const int unchanged = Pipeline::RunContentCompiler(arguments, factory);
            check(unchanged == 0 && writes == 1 && ReadText(manifestPath) == manifestText,
                  "an unchanged multi-artifact node skips its writer and retains its manifest");
            std::filesystem::remove(companion);
            const int missing = Pipeline::RunContentCompiler(arguments, factory);
            check(missing == 0 && writes == 2 && ReadCount(companion, RewardsType) == 75,
                  "deleting only the additional CNB rebuilds its owning node and restores the table");
            const auto primaryBytes = ReadText(primary), companionBytes = ReadText(companion);
            WriteText(source / "briefing.txt", "Protect the north gate.\n");
            const int editedSupport = Pipeline::RunContentCompiler(arguments, factory);
            check(editedSupport == 0 && writes == 3 && ReadText(support) == "Protect the north gate.\n" &&
                  ReadText(primary) == primaryBytes && ReadText(companion) == companionBytes,
                  "editing the deployment source rebuilds and copies it while preserving equal CNB content");
            const auto beforeFailure = ReadText(manifestPath);
            std::filesystem::remove(support);
            std::filesystem::create_directory(support);
            WriteText(support / "obstruction.txt", "Intentional destination obstruction.\n");
            WriteText(source / "arena.encounter", "4\n");
            const int blocked = Pipeline::RunContentCompiler(arguments, factory);
            check(blocked == 1 && ReadText(manifestPath) == beforeFailure &&
                  ReadText(support / "obstruction.txt") == "Intentional destination obstruction.\n",
                  "a blocked deployment destination fails without replacing the last successful manifest");
            // Individual files may already be replaced: this is not a whole-node rollback claim.
            std::cout << "After failed publication: waves=" << ReadCount(primary, EncounterType)
                      << ", rewards=" << ReadCount(companion, RewardsType) << '\n';
            std::filesystem::remove_all(support); // only the deliberately created private obstruction
            const int recovered = Pipeline::RunContentCompiler(arguments, factory);
            check(recovered == 0 && ReadCount(primary, EncounterType) == 4 && ReadCount(companion, RewardsType) == 100 &&
                  ReadText(support) == "Protect the north gate.\n" && ReadText(manifestPath) != beforeFailure,
                  "removing the obstruction permits a complete rebuild of the latest encounter and support file");
            const int afterRecovery = writes.load(); const auto recoveredManifest = ReadText(manifestPath);
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && writes == afterRecovery &&
                  ReadText(manifestPath) == recoveredManifest,
                  "the recovered complete artifact set is eligible for the next unchanged skip");
            // The fixed support destination belongs to one encounter. A second source would
            // claim that same path even though its two CNB names are distinct; the graph refuses it.
            const auto beforeConflictPrimary = ReadText(primary), beforeConflictCompanion = ReadText(companion);
            WriteText(source / "arena.encounter", "6\n"); // both nodes now have new bytes to publish
            WriteText(source / "bonus.encounter", "5\n");
            const int conflict = Pipeline::RunContentCompiler(arguments, factory);
            check(conflict == 1 && !std::filesystem::exists(output / "bonus.cnb") &&
                  !std::filesystem::exists(output / "rewards/bonus.cnb") && ReadText(manifestPath) == recoveredManifest &&
                  ReadText(primary) == beforeConflictPrimary && ReadText(companion) == beforeConflictCompanion &&
                  ReadText(support) == "Protect the north gate.\n",
                  "two nodes claiming one deployment destination are refused before publishing either node's changes");
            std::filesystem::remove(source / "bonus.encounter");
            WriteText(source / "arena.encounter", "4\n"); // restore authored input after observing graph refusal
            const int beforeRetry = writes.load();
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && writes == beforeRetry &&
                  ReadText(manifestPath) == recoveredManifest,
                  "removing the conflicting source restores the original node's unchanged skip");
            WriteText(output / "manual.txt", "Unowned design note.\n");
            const auto ownedPrimary = ReadText(primary), ownedCompanion = ReadText(companion), ownedSupport = ReadText(support);
            const auto savedManifest = directory / "saved-manifest.json";
            std::filesystem::rename(manifestPath, savedManifest);
            const int noOwnership = Pipeline::RunContentCompiler({"clean", output}, factory);
            check(noOwnership == 0 && !std::filesystem::exists(manifestPath) &&
                  ReadText(primary) == ownedPrimary && ReadText(companion) == ownedCompanion &&
                  ReadText(support) == ownedSupport && ReadText(output / "manual.txt") == "Unowned design note.\n",
                  "without an ownership manifest clean preserves compiled and support files instead of guessing by extension");
            // Restore exactly the saved ownership record only after verifying the no-manifest case.
            std::filesystem::rename(savedManifest, manifestPath);
            const int cleaned = Pipeline::RunContentCompiler({"clean", output}, factory);
            check(cleaned == 0 && !std::filesystem::exists(primary) && !std::filesystem::exists(companion) &&
                  !std::filesystem::exists(support) && !std::filesystem::exists(manifestPath) &&
                  ReadText(output / "manual.txt") == "Unowned design note.\n" &&
                  ReadText(source / "arena.encounter") == "4\n" && ReadText(source / "briefing.txt") == "Protect the north gate.\n",
                  "clean removes owned primary, additional and deployment artifacts while preserving source and unowned files");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private compiler inputs and outputs");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Encounter publication demo failed: " << error.what() << '\n';
        return 1;
    }
}
