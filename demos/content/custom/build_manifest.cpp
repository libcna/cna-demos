// SPDX-License-Identifier: MS-PL
// Turn one successful in-memory build result into deterministic incremental-build state.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Pipeline/ContentBuildManifest.hpp"

namespace Pipeline = CNA::Content::Pipeline;

namespace
{
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void WriteBytes(const std::filesystem::path& path,
                    const std::vector<std::uint8_t>& bytes)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        output.close();
        if (!output) throw std::runtime_error("Could not write manifest demo input");
    }

    bool IsDigest(const std::string& value)
    {
        return value.size() == 64u &&
            std::all_of(value.begin(), value.end(), [](const char character)
            {
                return (character >= '0' && character <= '9') ||
                    (character >= 'a' && character <= 'f');
            });
    }

    bool RefusesSet(Pipeline::ContentBuildManifestEntry entry,
                    const std::string_view fragment)
    {
        try
        {
            Pipeline::ContentBuildManifest manifest;
            manifest.Set(std::move(entry));
        }
        catch (const std::invalid_argument& error)
        {
            return std::string(error.what()).find(fragment) != std::string::npos;
        }
        return false;
    }

    bool Refuses(const std::function<void()>& action, const std::string_view fragment)
    {
        try
        {
            action();
        }
        catch (const std::runtime_error& error)
        {
            return std::string(error.what()).find(fragment) != std::string::npos;
        }
        return false;
    }

    void BuildManifest(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(Pipeline::ContentBuildManifestVersion == 10u &&
                      std::string(Pipeline::ContentBuildManifestFileName) ==
                          ".cna-content-manifest.json" &&
                      Pipeline::ContentSha256({'a', 'b', 'c'}) ==
                          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                  "the manifest identity and standard in-memory SHA-256 digest are stable");

        const auto root = std::filesystem::temp_directory_path() /
            ("cna-demos-build-manifest-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root))
            throw std::runtime_error("Private manifest directory already exists");
        TemporaryDirectory cleanup{root};
        const auto canonical = std::filesystem::canonical(root);
        const auto sourceRoot = canonical / "source";
        const auto outputRoot = canonical / "output";
        const auto source = sourceRoot / "inputs/hero.src";
        const auto support = sourceRoot / "shared/palette.bin";
        std::filesystem::create_directories(outputRoot / "Actors");
        WriteBytes(source, {1u, 2u, 3u, 4u});
        WriteBytes(support, {5u, 6u, 7u});

        const std::vector<std::uint8_t> primaryBytes{10u, 20u, 30u};
        const std::vector<std::uint8_t> indexBytes{40u, 50u};
        Pipeline::ContentBuildResult result;
        result.source = source;
        result.logicalName = "Actors/hero";
        result.importer = {"Demo.ActorImporter", "1"};
        result.processor = {"Demo.ActorProcessor", "2"};
        result.writer = {"Demo.ActorWriter", "3"};
        result.outputFormat = Pipeline::ContentOutputFormat::Cnb;
        result.writerSchemas = {
            {42u, 1u, "Demo.Actor", {"Demo.ActorCodec", "4"}},
            {43u, 2u, "Demo.ActorIndex", {"Demo.ActorIndexCodec", "5"}},
        };
        result.parameters.Set("Quality", std::uint64_t{2u});
        result.dependencies = {
            {Pipeline::ContentDependencyKind::ContentBuild, "Textures/hero", {}},
            {Pipeline::ContentDependencyKind::SourceFile, support.generic_string(), {}},
            {Pipeline::ContentDependencyKind::PrimarySource, source.generic_string(), {}},
        };
        result.runtimeReferences = {{"Textures/hero", 77u}};
        result.deploymentFiles = {{support, "Support/palette.bin", {}}};
        result.output.bytes = primaryBytes;
        result.output.assetTypeId = 42u;
        result.output.assetTypeName = "Demo.Actor";
        result.output.assetSchemaVersion = 1u;
        result.output.additionalOutputs.push_back(
            {"Actors/hero-index", indexBytes, 43u, "Demo.ActorIndex", 2u, {}, false});

        Pipeline::ContentBuildManifestEntry entry = Pipeline::MakeContentBuildManifestEntry(
            result, sourceRoot, outputRoot, outputRoot / "Actors/hero.cnb");
        const std::vector<Pipeline::ContentDependency> expectedDependencies{
            {Pipeline::ContentDependencyKind::PrimarySource, "inputs/hero.src", {}},
            {Pipeline::ContentDependencyKind::SourceFile, "shared/palette.bin", {}},
            {Pipeline::ContentDependencyKind::ContentBuild, "Textures/hero", {}},
        };
        ctx.Check(entry.nodeId == "Actors/hero" && entry.source == "inputs/hero.src" &&
                      entry.importer == result.importer && entry.processor == result.processor &&
                      entry.writer == result.writer && entry.writerSchemas == result.writerSchemas &&
                      entry.parameters == result.parameters && entry.dependencies == expectedDependencies &&
                      entry.runtimeReferences == result.runtimeReferences,
                  "a successful result becomes a root-relative node with complete build identities and sorted dependencies");

        ctx.Check(entry.outputs.size() == 2u &&
                      entry.outputs[0] == Pipeline::ContentBuildManifestOutput{
                          "Actors/hero", "Actors/hero.cnb", 42u, 1u, "Demo.Actor", {},
                          Pipeline::ContentSha256(primaryBytes)} &&
                      entry.outputs[1] == Pipeline::ContentBuildManifestOutput{
                          "Actors/hero-index", "Actors/hero-index.cnb", 43u, 2u,
                          "Demo.ActorIndex", {}, Pipeline::ContentSha256(indexBytes)} &&
                      entry.deploymentFiles ==
                          std::vector<Pipeline::ContentBuildManifestDeploymentFile>{{
                              {}, "shared/palette.bin", "Support/palette.bin",
                              Pipeline::ContentFileSha256(support)}} &&
                      entry.directFingerprint.empty() && entry.fingerprint.empty(),
                  "primary, additional and deployed outputs receive contained paths and exact byte digests before fingerprinting");

        Pipeline::RefreshContentBuildDirectFingerprint(entry, sourceRoot);
        const auto directState = entry.fingerprintState;
        const std::string direct = entry.directFingerprint;
        const bool directDomains =
            IsDigest(directState.primarySourceBytes) &&
            IsDigest(directState.sourceDependencySet) &&
            IsDigest(directState.sourceDependencyBytes) &&
            IsDigest(directState.contentDependencySet) &&
            directState.contentDependencyFingerprints.empty() &&
            IsDigest(directState.processorParameters) &&
            IsDigest(directState.writerSchemas) &&
            IsDigest(directState.outputDefinitions) &&
            IsDigest(directState.deploymentDefinitions);
        ctx.Check(directDomains && IsDigest(direct) &&
                      Pipeline::ComputeContentBuildDirectFingerprint(entry, sourceRoot) == direct,
                  "refresh stores every direct reason domain and agrees with the side-effect-free direct computation");

        WriteBytes(support, {5u, 6u, 8u});
        auto dependencyChanged = entry;
        Pipeline::RefreshContentBuildDirectFingerprint(dependencyChanged, sourceRoot);
        ctx.Check(dependencyChanged.fingerprintState.primarySourceBytes ==
                          directState.primarySourceBytes &&
                      dependencyChanged.fingerprintState.sourceDependencySet ==
                          directState.sourceDependencySet &&
                      dependencyChanged.fingerprintState.sourceDependencyBytes !=
                          directState.sourceDependencyBytes &&
                      dependencyChanged.directFingerprint != direct,
                  "changing only a source dependency changes its byte domain and the direct fingerprint, not its identity set");
        WriteBytes(support, {5u, 6u, 7u});

        auto parameterChanged = entry;
        parameterChanged.parameters.Set("Quality", std::uint64_t{3u});
        Pipeline::RefreshContentBuildDirectFingerprint(parameterChanged, sourceRoot);
        ctx.Check(parameterChanged.fingerprintState.processorParameters !=
                          directState.processorParameters &&
                      parameterChanged.fingerprintState.writerSchemas ==
                          directState.writerSchemas && parameterChanged.directFingerprint != direct,
                  "a typed processor change is isolated in its reason domain and invalidates the direct fingerprint");

        const std::map<std::string, std::string> dependencyOne{{"Textures/hero",
                                                                std::string(64u, '1')}};
        const std::map<std::string, std::string> dependencyTwo{{"Textures/hero",
                                                                std::string(64u, '2')}};
        const bool missingDependency = Refuses(
            [&] { (void)Pipeline::ComputeContentBuildEffectiveFingerprint(entry); },
            "has no current effective fingerprint");
        Pipeline::RefreshContentBuildEffectiveFingerprint(entry, dependencyOne);
        const std::string effectiveOne = entry.fingerprint;
        const std::string dependencyDomainOne =
            entry.fingerprintState.contentDependencyFingerprints;
        Pipeline::RefreshContentBuildEffectiveFingerprint(entry, dependencyTwo);
        const std::string effectiveTwo = entry.fingerprint;
        ctx.Check(missingDependency && IsDigest(effectiveOne) && IsDigest(effectiveTwo) &&
                      direct == entry.directFingerprint && effectiveOne != effectiveTwo &&
                      dependencyDomainOne != entry.fingerprintState.contentDependencyFingerprints,
                  "effective fingerprinting requires every build dependency and changes without rewriting direct state");
        Pipeline::RefreshContentBuildEffectiveFingerprint(entry, dependencyOne);

        const auto beforeCombined = entry;
        const std::string combined = Pipeline::ComputeContentBuildFingerprint(
            entry, sourceRoot, dependencyOne);
        ctx.Check(combined == entry.fingerprint && entry == beforeCombined,
                  "the combined helper reproduces current effective state without mutating its input record");

        auto zeta = entry;
        zeta.nodeId = "Zeta/cache";
        zeta.outputs[0].logicalName = "Zeta/cache";
        zeta.outputs[0].path = "Zeta/cache.cnb";
        zeta.outputs[1].logicalName = "Zeta/cache-index";
        zeta.outputs[1].path = "Zeta/cache-index.cnb";
        Pipeline::RefreshContentBuildDirectFingerprint(zeta, sourceRoot);
        Pipeline::RefreshContentBuildEffectiveFingerprint(zeta, dependencyOne);
        Pipeline::ContentBuildManifest manifest;
        manifest.Set(zeta);
        manifest.Set(entry);
        ctx.Check(manifest.Entries().size() == 2u &&
                      manifest.Entries().begin()->first == "Actors/hero" &&
                      manifest.Find("Actors/hero") != nullptr && manifest.Find("missing") == nullptr,
                  "Set and Entries retain nodes in stable logical-name order and Find distinguishes absence");

        auto replacement = entry;
        replacement.parameters.Set("Quality", std::uint64_t{4u});
        Pipeline::RefreshContentBuildDirectFingerprint(replacement, sourceRoot);
        Pipeline::RefreshContentBuildEffectiveFingerprint(replacement, dependencyOne);
        manifest.Set(replacement);
        const auto* replaced = manifest.Find("Actors/hero");
        ctx.Check(replaced != nullptr && *replaced == replacement &&
                      manifest.Entries().size() == 2u,
                  "Set replaces the complete record under an existing node identity");
        manifest.Set(entry);

        const std::string serialized = manifest.Serialize();
        const auto parsed = Pipeline::ContentBuildManifest::Parse(serialized);
        ctx.Check(!serialized.empty() && serialized.back() == '\n' &&
                      serialized.find("CNA.ContentPipeline.Manifest") != std::string::npos &&
                      serialized.find("\"version\":10") != std::string::npos &&
                      serialized.find("\"generatedOutput\":false") != std::string::npos &&
                      serialized.find(canonical.generic_string()) == std::string::npos &&
                      parsed.Entries() == manifest.Entries() && parsed.Serialize() == serialized,
                  "serialization is deterministic, round-trips every stable field and never leaks the physical build root");

        auto unsafeSource = entry;
        unsafeSource.source = "../escape.src";
        auto missingPrimary = entry;
        missingPrimary.outputs.erase(missingPrimary.outputs.begin());
        auto wrongSchema = entry;
        wrongSchema.outputs[0].assetSchemaVersion = 99u;
        auto orphanDeployment = entry;
        orphanDeployment.dependencies.erase(
            std::remove_if(orphanDeployment.dependencies.begin(),
                           orphanDeployment.dependencies.end(),
                           [](const Pipeline::ContentDependency& dependency)
                           {
                               return dependency.kind == Pipeline::ContentDependencyKind::SourceFile;
                           }),
            orphanDeployment.dependencies.end());
        auto uppercaseDigest = entry;
        uppercaseDigest.outputs[0].sha256 = std::string(64u, 'A');
        ctx.Check(RefusesSet(std::move(unsafeSource), "source must") &&
                      RefusesSet(std::move(missingPrimary), "primary output") &&
                      RefusesSet(std::move(wrongSchema), "does not match") &&
                      RefusesSet(std::move(orphanDeployment), "not a byte-hashed") &&
                      RefusesSet(std::move(uppercaseDigest), "lowercase SHA-256"),
                  "Set refuses escaping sources, missing ownership, undeclared schemas, unhashed deployment inputs and invalid digests");

        const std::string currentVersion = "\"version\":" +
            std::to_string(Pipeline::ContentBuildManifestVersion);
        const bool malformed = Refuses(
            [] { (void)Pipeline::ContentBuildManifest::Parse("{"); }, {});
        bool incompatible = true;
        for (const unsigned version : {8u, 9u})
        {
            std::string oldVersion = serialized;
            oldVersion.replace(oldVersion.find(currentVersion), currentVersion.size(),
                               "\"version\":" + std::to_string(version));
            const bool refused = Refuses(
                [&] { (void)Pipeline::ContentBuildManifest::Parse(oldVersion); },
                "version is not supported");
            incompatible = refused && incompatible;
        }
        ctx.Check(malformed && incompatible,
                  "Parse refuses malformed or incompatible persisted cache state so a host can rebuild it");

        manifest.Clear();
        const auto cleared = Pipeline::ContentBuildManifest::Parse(manifest.Serialize());
        ctx.Check(manifest.Entries().empty() && cleared.Entries().empty(),
                  "Clear produces a valid empty manifest rather than retaining stale ownership");
    }
}

CNA_DEMOS_CONSOLE(BuildManifest, {
    .id = "content.custom.build_manifest",
    .title = "Persist content build state (CNAEXT)",
    .description = "Turns one multi-output build result into root-relative manifest state, hashes direct and effective inputs, round-trips deterministic JSON, replaces and clears nodes, and checks ownership validation.",
    .apis = {"CNA::Content::Pipeline::ContentBuildManifestVersion",
             "CNA::Content::Pipeline::ContentBuildManifestFileName",
             "CNA::Content::Pipeline::ContentBuildManifestOutput",
             "CNA::Content::Pipeline::ContentBuildManifestDeploymentFile",
             "CNA::Content::Pipeline::ContentBuildFingerprintState",
             "CNA::Content::Pipeline::ContentBuildManifestEntry",
             "CNA::Content::Pipeline::ContentBuildManifest::Parse",
             "CNA::Content::Pipeline::ContentBuildManifest::Serialize",
             "CNA::Content::Pipeline::ContentBuildManifest::Find",
             "CNA::Content::Pipeline::ContentBuildManifest::Set",
             "CNA::Content::Pipeline::ContentBuildManifest::Clear",
             "CNA::Content::Pipeline::ContentBuildManifest::Entries",
             "CNA::Content::Pipeline::ContentSha256",
             "CNA::Content::Pipeline::ContentFileSha256",
             "CNA::Content::Pipeline::ComputeContentBuildDirectFingerprint",
             "CNA::Content::Pipeline::RefreshContentBuildDirectFingerprint",
             "CNA::Content::Pipeline::ComputeContentBuildEffectiveFingerprint",
             "CNA::Content::Pipeline::RefreshContentBuildEffectiveFingerprint",
             "CNA::Content::Pipeline::ComputeContentBuildFingerprint",
             "CNA::Content::Pipeline::MakeContentBuildManifestEntry"},
    .tags = {"content", "pipeline", "manifest", "incremental", "hashing", "cnaext"},
    .restrictions = "Uses original temporary source/support files and in-memory output bytes, then removes them. Existing compiler demos cover publication, skip, graph scheduling and clean; no concurrent publication or crash-consistency claim.",
})
