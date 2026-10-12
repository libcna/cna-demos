// SPDX-License-Identifier: MS-PL
// Build edges propagate fingerprints; runtime references alone do not request recompilation.
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/ContentBuildManifest.hpp"
#include "CNA/Content/Pipeline/ContentCompiler.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentManager;
namespace
{
    constexpr const char* SourceType = "CnaDemos.GraphBudgetSource";
    constexpr const char* RuntimeType = "CnaDemos.GraphBudget";
    constexpr auto DataChunk = Cnb::MakeChunkId('D', 'A', 'T', 'A');
    struct Source { std::uint32_t multiplier; std::string reference; bool buildDependency; };
    struct Budget { std::uint32_t multiplier; std::string reference; };
    std::atomic<int> battleWrites{0}, enemyWrites{0}, hudWrites{0};
    std::atomic<bool> observeParallelImports{false};
    std::mutex importMutex;
    std::condition_variable importsReady;
    int importArrivals = 0;
    bool importWaitExpired = false;
    std::array<int, 3> Writes() { return {battleWrites.load(), enemyWrites.load(), hudWrites.load()}; }
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    void WriteText(const std::filesystem::path& path, std::string_view text)
    {
        std::ofstream file(path, std::ios::binary);
        file.write(text.data(), static_cast<std::streamsize>(text.size())); file.close();
        if (!file) throw std::runtime_error("Could not write the original graph fixture.");
    }
    std::string ReadText(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Could not read the compiler artifact.");
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
    class BudgetImporter final : public Pipeline::ContentImporter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.BudgetImporter", "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".budget"}; }
        std::vector<std::string> OutputTypes() const override { return {SourceType}; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            if (observeParallelImports)
            {
                // The first importer cannot leave until another enters. A bounded wait makes a
                // missing second worker a failed observation rather than an indefinitely hung demo.
                std::unique_lock lock(importMutex);
                ++importArrivals;
                importsReady.notify_all();
                if (importArrivals <= 2 && !importsReady.wait_for(lock, std::chrono::seconds(5), [] {
                        return importArrivals >= 2;
                    })) importWaitExpired = true;
            }
            std::ifstream file(context.SourcePath()); Source source{}; std::string mode;
            if (!(file >> source.multiplier >> source.reference >> mode) || source.multiplier < 1 ||
                source.multiplier > 1000 || (mode != "build" && mode != "runtime"))
                throw std::runtime_error("A budget contains a multiplier in 1..1000, a reference (or -), and build/runtime mode.");
            file >> std::ws;
            if (!file.eof()) throw std::runtime_error("Unexpected budget fields.");
            if (source.reference == "-") source.reference.clear();
            source.buildDependency = mode == "build";
            return Pipeline::ContentValue::Create(SourceType, std::move(source));
        }
    };
    class BudgetProcessor final : public Pipeline::ContentProcessor
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.BudgetProcessor", "1"}; }
        std::string InputType() const override { return SourceType; }
        std::string OutputType() const override { return RuntimeType; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            if (!parameters.Values().empty()) throw std::invalid_argument("The budget has no processor parameters.");
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            const auto& source = input.Get<Source>();
            if (!source.reference.empty())
            {
                context.AddRuntimeReference(source.reference, Cnb::CnbAssetTypeIdFromName(RuntimeType));
                if (source.buildDependency) context.AddContentBuildDependency(source.reference);
            }
            // No read of the other node's source: the strong edge alone propagates its fingerprint.
            return Pipeline::ContentValue::Create(RuntimeType, Budget{source.multiplier, source.reference});
        }
    };
    class BudgetWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.BudgetWriter", "1"}; }
        std::string InputType() const override { return RuntimeType; }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(RuntimeType), 1u, RuntimeType, {"CnaDemos.BudgetCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input, const std::string& name) const override
        {
            if (name == "battle") ++battleWrites;
            if (name == "enemy") ++enemyWrites;
            if (name == "hud") ++hudWrites;
            const auto& budget = input.Get<Budget>();
            Cnb::CnbByteWriter payload; payload.WriteU32(budget.multiplier);
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(RuntimeType), 1);
            writer.SetMetadata(RuntimeType, name);
            if (!budget.reference.empty())
                writer.SetExternalReferences({{0u, Cnb::CnbAssetTypeIdFromName(RuntimeType), budget.reference}});
            writer.AddChunk(DataChunk, payload.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build(); result.assetTypeId = Cnb::CnbAssetTypeIdFromName(RuntimeType);
            result.assetTypeName = RuntimeType; result.assetSchemaVersion = 1;
            return result;
        }
    };
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
        ContentManager::RegisterCnbLoaderEXT<std::uint32_t>(Cnb::CnbAssetTypeIdFromName(RuntimeType), RuntimeType,
            [](const Cnb::CnbDocument& document, ContentManager& owner) {
                document.RequireAsset(Cnb::CnbAssetTypeIdFromName(RuntimeType), 1);
                document.RequireMandatoryChunksUnderstood(std::array{DataChunk});
                auto data = document.OpenChunk(document.RequireSingle(DataChunk));
                const auto multiplier = data.ReadU32(); data.RequireExhausted();
                const auto& references = document.ExternalReferences();
                if (references.size() > 1) throw std::runtime_error("A runtime budget has at most one referenced profile.");
                if (!references.empty() && references.front().expectedAssetTypeId != Cnb::CnbAssetTypeIdFromName(RuntimeType))
                    throw std::runtime_error("A budget reference must declare the budget schema.");
                return references.empty() ? multiplier : multiplier * owner.Load<std::uint32_t>(references.front().logicalName);
            });
        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-budget-graph-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not reserve private graph data.");
        {
            TemporaryDirectory cleanup{directory};
            std::cout << "Private budget graph data: " << directory << '\n';
            const auto source = directory / "source", output = directory / "output";
            std::filesystem::create_directory(source);
            WriteText(source / "battle.budget", "3 enemy build\n");
            WriteText(source / "enemy.budget", "7 - runtime\n");
            WriteText(source / "hud.budget", "2 enemy runtime\n");
            const Pipeline::ContentPipelineRegistryFactory factory = [](const Pipeline::ContentCompilerOptions&) {
                auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
                registry->RegisterImporter(std::make_shared<BudgetImporter>());
                registry->RegisterProcessor(std::make_shared<BudgetProcessor>());
                registry->RegisterWriter(std::make_shared<BudgetWriter>());
                return registry;
            };
            const std::vector<std::filesystem::path> arguments{
                "build", source, "-o", output, "--format", "cnb", "--workers", "1"};
            const auto manifestPath = output / Pipeline::ContentBuildManifestFileName;
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && Writes() == std::array{1, 1, 1},
                  "the compiler builds all three original budget nodes once");
            const auto initialManifestText = ReadText(manifestPath);
            const auto initial = Pipeline::ContentBuildManifest::Parse(initialManifestText);
            const auto* initialBattle = initial.Find("battle"), *initialHud = initial.Find("hud");
            bool strong = false, sourceRead = false, hudStrong = false;
            if (initialBattle) for (const auto& dependency : initialBattle->dependencies)
            {
                strong |= dependency.kind == Pipeline::ContentDependencyKind::ContentBuild && dependency.identity == "enemy";
                sourceRead |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile;
            }
            if (initialHud) for (const auto& dependency : initialHud->dependencies)
                hudStrong |= dependency.kind == Pipeline::ContentDependencyKind::ContentBuild;
            check(initial.Entries().size() == 3 && initialBattle && initialHud && strong && !sourceRead && !hudStrong &&
                  initialBattle->runtimeReferences.size() == 1 && initialHud->runtimeReferences.size() == 1,
                  "both consumers have runtime references but only battle records a strong build edge without reading enemy source");
            {
                ContentManager content(nullptr, output.string());
                check(content.Load<std::uint32_t>("battle") == 21 && content.Load<std::uint32_t>("hud") == 14,
                      "runtime loaders resolve the shared enemy profile and compute both authored budgets");
            }
            const auto originalBattle = ReadText(output / "battle.cnb"), originalHud = ReadText(output / "hud.cnb");
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && Writes() == std::array{1, 1, 1} &&
                  ReadText(manifestPath) == initialManifestText,
                  "an unchanged graph skips every writer and retains the complete manifest");
            WriteText(source / "enemy.budget", "9 - runtime\n");
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && Writes() == std::array{2, 2, 1},
                  "editing enemy rebuilds it and its strong dependent while the runtime-only consumer skips");
            const auto changedManifestText = ReadText(manifestPath);
            const auto changed = Pipeline::ContentBuildManifest::Parse(changedManifestText);
            const auto* changedBattle = changed.Find("battle"), *changedHud = changed.Find("hud");
            check(initialBattle && initialHud && changedBattle && changedHud &&
                  changedBattle->directFingerprint == initialBattle->directFingerprint &&
                  changedBattle->fingerprint != initialBattle->fingerprint &&
                  changedHud->fingerprint == initialHud->fingerprint &&
                  ReadText(output / "battle.cnb") == originalBattle && ReadText(output / "hud.cnb") == originalHud,
                  "the strong dependent's effective fingerprint changes despite identical direct inputs and output bytes");
            {
                ContentManager content(nullptr, output.string()); // a fresh runtime cache observes the published profile
                check(content.Load<std::uint32_t>("battle") == 27 && content.Load<std::uint32_t>("hud") == 18,
                      "both fresh runtime loads see the updated profile even though only the strong consumer was rebuilt");
            }
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && Writes() == std::array{2, 2, 1} &&
                  ReadText(manifestPath) == changedManifestText,
                  "persisted effective fingerprints make the updated graph stable on the next invocation");
            const auto validEnemy = ReadText(output / "enemy.cnb");
            WriteText(source / "enemy.budget", "9 battle build\n");
            check(Pipeline::RunContentCompiler(arguments, factory) == 1 && ReadText(manifestPath) == changedManifestText &&
                  ReadText(output / "battle.cnb") == originalBattle && ReadText(output / "enemy.cnb") == validEnemy &&
                  ReadText(output / "hud.cnb") == originalHud,
                  "a strong dependency cycle is refused without replacing the last valid artifacts or manifest");
            WriteText(source / "enemy.budget", "9 - runtime\n");
            const auto afterCycle = Writes();
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && Writes() == afterCycle &&
                  ReadText(manifestPath) == changedManifestText,
                  "restoring the acyclic source permits reuse of the previous complete graph");
            WriteText(source / "battle.budget", "3 missing_profile build\n");
            check(Pipeline::RunContentCompiler(arguments, factory) == 1 && ReadText(manifestPath) == changedManifestText &&
                  ReadText(output / "battle.cnb") == originalBattle && ReadText(output / "enemy.cnb") == validEnemy,
                  "a strong dependency must name a discovered primary node before new output can be published");
            WriteText(source / "battle.budget", "3 enemy build\n");
            const auto afterMissing = Writes();
            check(Pipeline::RunContentCompiler(arguments, factory) == 0 && Writes() == afterMissing &&
                  ReadText(manifestPath) == changedManifestText,
                  "repairing the missing-node reference restores unchanged graph skipping");

            const auto parallelOutput = directory / "parallel-output";
            const std::vector<std::filesystem::path> parallelArguments{
                "build", source, "-o", parallelOutput, "--format", "cnb", "--workers", "2"};
            auto expectedWrites = Writes();
            for (int& count : expectedWrites) ++count;
            observeParallelImports = true;
            const int parallel = Pipeline::RunContentCompiler(parallelArguments, factory);
            bool overlapped = false;
            {
                std::lock_guard lock(importMutex);
                overlapped = importArrivals == 3 && !importWaitExpired;
            }
            check(parallel == 0 && overlapped && Writes() == expectedWrites,
                  "two workers enter the importer concurrently and compile each original node once");
            const auto parallelManifest = parallelOutput / Pipeline::ContentBuildManifestFileName;
            check(ReadText(parallelManifest) == changedManifestText &&
                  ReadText(parallelOutput / "battle.cnb") == originalBattle &&
                  ReadText(parallelOutput / "enemy.cnb") == validEnemy && ReadText(parallelOutput / "hud.cnb") == originalHud,
                  "the two-worker build produces the same complete manifest and CNB bytes as the serial build");
            const int parallelSkip = Pipeline::RunContentCompiler(parallelArguments, factory);
            observeParallelImports = false;
            bool noNewImports = false;
            {
                std::lock_guard lock(importMutex);
                noNewImports = importArrivals == 3;
            }
            check(parallelSkip == 0 && noNewImports && Writes() == expectedWrites &&
                  ReadText(parallelManifest) == changedManifestText,
                  "an unchanged two-worker invocation skips all importers and writers without changing the manifest");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private graph sources and compiled outputs");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Build graph demo failed: " << error.what() << '\n';
        return 1;
    }
}
