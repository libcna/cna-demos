// SPDX-License-Identifier: MS-PL
// A stage processor consumes a processed profile and can compile that profile as a separate asset.
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "CnaDemos/Process.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnaPipelineBridge.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/ExternalReference.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Xna = Microsoft::Xna::Framework::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentLoadException;
using Microsoft::Xna::Framework::Content::ContentManager;
namespace
{
    constexpr const char* ProfileProcessorName = "CnaDemos.NestedProfileProcessor";
    constexpr const char* StageProcessorName = "CnaDemos.NestedStageProcessor";
    constexpr const char* ChildName = "profiles/guard";
    constexpr const char* SharedTextureName = "textures/nested_shared_checker";
    constexpr auto DataChunk = Cnb::MakeChunkId('D', 'A', 'T', 'A');
    std::atomic<int> profileWrites{0}; // observation only; registered writers may serve concurrent nodes
    struct Profile
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.NestedProfile";
        std::string name;
        int health = 0;
    };
    struct TextureAsset
    {
        static constexpr std::string_view XnaTypeName =
            "Microsoft.Xna.Framework.Graphics.Texture2D";
    };
    struct StageSource
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.NestedStageSource";
        int count = 0;
        Xna::ExternalReference<Profile> profile;
        Xna::ExternalReference<TextureAsset> texture;
    };
    struct Stage
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.NestedStage";
        int healthBudget = 0;
        std::string profileAsset;
        std::string textureAsset;
    };
    using SharedProfile = std::shared_ptr<const Profile>;
    struct LoadedStage { int healthBudget = 0; SharedProfile profile; };
    std::atomic<int> profileReads{0}, stageReads{0};
    struct TemporaryOutput
    {
        std::filesystem::path path;
        ~TemporaryOutput() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    void Publish(const std::filesystem::path& root, const std::string& name, const std::vector<std::uint8_t>& bytes)
    {
        // The caller publishes the coordinator's returned bytes; this is not the CLI's atomic publisher.
        const auto path = root / (name + ".cnb");
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        file.close();
        if (!file) throw std::runtime_error("Could not publish the compiled content bytes.");
    }
    class ProfileImporter final : public Xna::ContentImporter<Profile>
    {
    public:
        Profile Import(const std::string& filename, Xna::ContentImporterContext&) override
        {
            const std::filesystem::path path(std::u8string(filename.begin(), filename.end()));
            std::ifstream file(path);
            Profile result;
            if (!(file >> result.name >> result.health) || result.health < 1 || result.health > 1000)
                throw Xna::InvalidContentException("A profile needs a name and Health in 1..1000.");
            file >> std::ws;
            if (!file.eof()) throw Xna::InvalidContentException("Unexpected profile fields.");
            return result;
        }
    };
    class StageImporter final : public Xna::ContentImporter<StageSource>
    {
    public:
        StageSource Import(const std::string& filename, Xna::ContentImporterContext&) override
        {
            const std::filesystem::path path(std::u8string(filename.begin(), filename.end()));
            std::ifstream file(path);
            StageSource result; std::string child, texture;
            if (!(file >> result.count >> child >> texture) || result.count < 1 || result.count > 100)
                throw Xna::InvalidContentException(
                    "A stage needs Count in 1..100 plus profile and texture filenames.");
            file >> std::ws;
            if (!file.eof()) throw Xna::InvalidContentException("Unexpected stage fields.");
            result.profile = Xna::ExternalReference<Profile>(child, Xna::ContentIdentity(filename));
            result.texture = Xna::ExternalReference<TextureAsset>(texture, Xna::ContentIdentity(filename));
            return result;
        }
    };
    class ProfileProcessor final : public Xna::ContentProcessor<Profile, Profile>
    {
    public:
        int getScaleProperty() const { return scale_; }
        void setScaleProperty(int value)
        {
            if (value < 1 || value > 10) throw System::ArgumentOutOfRangeException("value");
            scale_ = value;
        }
        static void DescribeParameters(Xna::ProcessorParameterBindings<ProfileProcessor>& parameters)
        {
            parameters.Add<int>("Scale", &ProfileProcessor::getScaleProperty, &ProfileProcessor::setScaleProperty);
        }
        Profile Process(const Profile& input, Xna::ContentProcessorContext&) override
        {
            return {input.name, input.health * scale_};
        }
    private:
        int scale_ = 1;
    };
    class StageProcessor final : public Xna::ContentProcessor<StageSource, Stage>
    {
    public:
        bool getUseSiblingNameProperty() const { return useSiblingName_; }
        void setUseSiblingNameProperty(bool value) { useSiblingName_ = value; }
        bool getCompileChildProperty() const { return compileChild_; }
        void setCompileChildProperty(bool value) { compileChild_ = value; }
        bool getCollideProperty() const { return collide_; }
        void setCollideProperty(bool value) { collide_ = value; }
        bool getCompileSharedTextureProperty() const { return compileSharedTexture_; }
        void setCompileSharedTextureProperty(bool value) { compileSharedTexture_ = value; }
        static void DescribeParameters(Xna::ProcessorParameterBindings<StageProcessor>& parameters)
        {
            parameters.Add<bool>("UseSiblingName", &StageProcessor::getUseSiblingNameProperty, &StageProcessor::setUseSiblingNameProperty);
            parameters.Add<bool>("CompileChild", &StageProcessor::getCompileChildProperty, &StageProcessor::setCompileChildProperty);
            parameters.Add<bool>("Collide", &StageProcessor::getCollideProperty, &StageProcessor::setCollideProperty);
            parameters.Add<bool>("CompileSharedTexture", &StageProcessor::getCompileSharedTextureProperty,
                                 &StageProcessor::setCompileSharedTextureProperty);
        }
        Stage Process(const StageSource& input, Xna::ContentProcessorContext& context) override
        {
            Xna::OpaqueDataDictionary parameters; parameters.SetValue<int>("Scale", 2);
            const auto profile = context.BuildAndLoadAsset<Profile, Profile>(input.profile, ProfileProcessorName, parameters);
            Stage result{input.count * profile.health, {}, {}};
            if (compileChild_)
            {
                const std::string requestedName = useSiblingName_ ? std::string{} : ChildName;
                const auto first = context.BuildAsset<Profile, Profile>(input.profile, ProfileProcessorName, parameters, {}, requestedName);
                const auto repeated = context.BuildAsset<Profile, Profile>(input.profile, ProfileProcessorName, parameters, {}, requestedName);
                if (first.getFilenameProperty() != repeated.getFilenameProperty())
                    throw Xna::InvalidContentException("Repeated identical child build returned a different filename.");
                // This example leaves outputDirectory empty: the returned path is a relative .cnb name.
                std::filesystem::path built(first.getFilenameProperty());
                if (built.is_absolute() || built.extension() != ".cnb")
                    throw Xna::InvalidContentException("Expected a relative compiled child name.");
                result.profileAsset = built.replace_extension().generic_string();
                if (collide_)
                {
                    parameters.SetValue<int>("Scale", 3);
                    (void)context.BuildAsset<Profile, Profile>(input.profile, ProfileProcessorName, parameters, {}, requestedName);
                }
            }
            if (compileSharedTexture_)
            {
                Xna::OpaqueDataDictionary textureParameters;
                textureParameters.SetValue<std::string>(Pipeline::TextureFormatParameter, "Color");
                textureParameters.SetValue<bool>(Pipeline::TexturePremultiplyAlphaParameter, false);
                const auto built = context.BuildAsset<TextureAsset, TextureAsset>(
                    input.texture, {}, textureParameters, {}, SharedTextureName);
                std::filesystem::path path(built.getFilenameProperty());
                result.textureAsset = path.replace_extension().generic_string();
            }
            return result;
        }
    private:
        bool compileChild_ = false, collide_ = false, useSiblingName_ = false;
        bool compileSharedTexture_ = false;
    };
    class ProfileWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.NestedProfileWriter", "1"}; }
        std::string InputType() const override { return std::string(Profile::XnaTypeName); }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(InputType()), 1u, InputType(), {"CnaDemos.NestedProfileCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input, const std::string& logicalName) const override
        {
            ++profileWrites;
            const auto& profile = input.Get<Profile>();
            Cnb::CnbByteWriter data; data.WriteString(profile.name); data.WriteU32(static_cast<std::uint32_t>(profile.health));
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(InputType()), 1);
            writer.SetMetadata(InputType(), logicalName); writer.AddChunk(DataChunk, data.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build(); result.assetTypeId = Cnb::CnbAssetTypeIdFromName(InputType());
            result.assetTypeName = InputType(); result.assetSchemaVersion = 1;
            return result;
        }
    };
    class StageWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.NestedStageWriter", "1"}; }
        std::string InputType() const override { return std::string(Stage::XnaTypeName); }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(InputType()), 1u, InputType(), {"CnaDemos.NestedStageCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input, const std::string& logicalName) const override
        {
            const auto& stage = input.Get<Stage>();
            Cnb::CnbByteWriter data; data.WriteU32(static_cast<std::uint32_t>(stage.healthBudget));
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(InputType()), 1);
            writer.SetMetadata(InputType(), logicalName);
            std::vector<Cnb::CnbExternalReference> references;
            if (!stage.profileAsset.empty())
                references.push_back({static_cast<std::uint32_t>(references.size()),
                                      Cnb::CnbAssetTypeIdFromName(std::string(Profile::XnaTypeName)),
                                      stage.profileAsset});
            if (!stage.textureAsset.empty())
                references.push_back({static_cast<std::uint32_t>(references.size()),
                                      Cnb::CnbAssetTypeId::Texture2D, stage.textureAsset});
            if (!references.empty()) writer.SetExternalReferences(std::move(references));
            writer.AddChunk(DataChunk, data.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build(); result.assetTypeId = Cnb::CnbAssetTypeIdFromName(InputType());
            result.assetTypeName = InputType(); result.assetSchemaVersion = 1;
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
            passed = condition && passed;
        };
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Xna::ContentImporterAttribute profileImporter(".profile"); profileImporter.setDefaultProcessorProperty(ProfileProcessorName);
        Xna::ContentImporterAttribute stageImporter(".stage"); stageImporter.setDefaultProcessorProperty(StageProcessorName);
        Pipeline::RegisterXnaImporter<ProfileImporter>(*registry, "CnaDemos.NestedProfileImporter", profileImporter);
        Pipeline::RegisterXnaImporter<StageImporter>(*registry, "CnaDemos.NestedStageImporter", stageImporter);
        Pipeline::RegisterXnaProcessor<ProfileProcessor>(*registry, ProfileProcessorName, Xna::ContentProcessorAttribute{});
        Pipeline::RegisterXnaProcessor<StageProcessor>(*registry, StageProcessorName, Xna::ContentProcessorAttribute{});
        Pipeline::RegisterCnjContentPipeline(*registry);
        Pipeline::RegisterTexture2DContentPipeline(*registry);
        registry->RegisterWriter(std::make_shared<ProfileWriter>()); registry->RegisterWriter(std::make_shared<StageWriter>());
        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = CnaDemos::ExecutableDirectory() / "assets/nested_builds";
        request.source = "arena.stage"; request.logicalName = "stages/arena";
        const auto loaded = pipeline.Build(request);
        const auto loadedDocument = Cnb::CnbDocument::Parse(loaded.output.bytes, "stage with processed profile");
        auto budget = loadedDocument.OpenChunk(loadedDocument.RequireSingle(DataChunk));
        check(budget.ReadU32() == 240, "BuildAndLoadAsset imports and scales the profile before the stage computes its health budget");
        budget.RequireExhausted();
        check(profileWrites == 0 && loaded.output.additionalOutputs.empty() && loaded.runtimeReferences.empty() &&
              loadedDocument.ExternalReferences().empty(), "consuming a processed child produces no child compilation or runtime reference");
        bool primary = false, child = false;
        for (const auto& dependency : loaded.dependencies)
        {
            primary |= dependency.kind == Pipeline::ContentDependencyKind::PrimarySource && dependency.identity.ends_with("/nested_builds/arena.stage");
            child |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile && dependency.identity.ends_with("/nested_builds/guard.profile");
        }
        check(primary && child, "the parent records its primary source and the nested imported profile as a source-file dependency");
        request.parameters.Set("CompileChild", true);
        const auto compiled = pipeline.Build(request);
        check(profileWrites == 1 && compiled.output.additionalOutputs.size() == 1,
              "two identical BuildAsset requests compile the child once and return one additional output");
        if (compiled.output.additionalOutputs.size() != 1) return 1;
        const auto& output = compiled.output.additionalOutputs.front();
        check(output.logicalName == ChildName && output.assetTypeName == Profile::XnaTypeName && output.assetSchemaVersion == 1,
              "the additional output preserves the explicitly requested child name and schema identity");
        const auto childDocument = Cnb::CnbDocument::Parse(output.bytes, "compiled profile");
        childDocument.RequireAsset(Cnb::CnbAssetTypeIdFromName(std::string(Profile::XnaTypeName)), 1);
        auto childData = childDocument.OpenChunk(childDocument.RequireSingle(DataChunk));
        const auto childName = childData.ReadString(); const auto childHealth = childData.ReadU32(); childData.RequireExhausted();
        check(childName == "Guard" && childHealth == 80, "the separate child bytes contain the same parameter-scaled profile used for the budget");
        const auto parentDocument = Cnb::CnbDocument::Parse(compiled.output.bytes, "stage referencing compiled profile");
        auto parentData = parentDocument.OpenChunk(parentDocument.RequireSingle(DataChunk));
        check(parentData.ReadU32() == 240 && parentDocument.ExternalReferences().size() == 1,
              "compiling the child preserves the stage budget and emits one XREF entry");
        parentData.RequireExhausted();
        const auto& reference = parentDocument.ExternalReferenceAt(0, "stage profile");
        check(reference.logicalName == output.logicalName && reference.expectedAssetTypeId == output.assetTypeId &&
              compiled.runtimeReferences.size() == 1 && compiled.runtimeReferences.front().logicalName == output.logicalName,
              "the parent CNB reference and coordinator runtime reference identify the actual additional output");
        bool profileSchema = false, stageSchema = false;
        for (const auto& schema : compiled.writerSchemas)
        {
            profileSchema |= schema.assetTypeName == Profile::XnaTypeName;
            stageSchema |= schema.assetTypeName == Stage::XnaTypeName;
        }
        check(profileSchema && stageSchema, "the combined build records both writer schema dependencies");
        auto collision = request; collision.parameters.Set("Collide", true);
        bool refused = false;
        try { (void)pipeline.Build(collision); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            refused = error.Stage() == Pipeline::ContentPipelineStage::Process && error.Component() == StageProcessorName &&
                      std::string(error.what()).find("already built from a different source or processing") != std::string::npos;
            std::cout << "Refused child collision: " << error.what() << '\n';
        }
        check(refused, "different child processing under the same requested output name is refused in the parent process stage");
        const auto repeated = pipeline.Build(request);
        check(repeated.output.bytes == compiled.output.bytes && repeated.output.additionalOutputs.size() == 1 &&
              repeated.output.additionalOutputs.front().bytes == output.bytes,
              "a fresh valid build is deterministic after the failed conflicting request");
        auto capabilityRequest = request;
        capabilityRequest.parameters.Set("CompileChild", false);
        capabilityRequest.parameters.Set("CompileSharedTexture", true);
        bool ungrantedNestedRefused = false;
        try { (void)pipeline.Build(capabilityRequest); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            ungrantedNestedRefused = error.Stage() == Pipeline::ContentPipelineStage::Process &&
                error.Component() == StageProcessorName &&
                std::string(error.what()).find("unknown root alias 'art'") != std::string::npos;
            std::cout << "Ungranted nested capability refused: " << error.what() << '\n';
        }
        check(ungrantedNestedRefused,
              "a nested CNJ cannot use an external source-root alias absent from the parent request");
        const auto textureAssets = CnaDemos::ExecutableDirectory() / "assets/textures";
        capabilityRequest.externalSourceRoots.Add("art", textureAssets);
        const auto capability = pipeline.Build(capabilityRequest);
        bool nestedRecipe = false, nestedArt = false;
        for (const auto& dependency : capability.dependencies)
        {
            nestedRecipe |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                dependency.identity.ends_with("/nested_builds/shared_texture.cnj") &&
                dependency.sourceRoot.empty();
            nestedArt |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                dependency.identity.ends_with("/textures/checker.png") && dependency.sourceRoot == "art";
        }
        bool exactTexture = capability.output.additionalOutputs.size() == 1;
        if (exactTexture)
        {
            const auto& textureOutput = capability.output.additionalOutputs.front();
            const auto textureDocument = Cnb::CnbDocument::Parse(textureOutput.bytes, "nested shared texture");
            const auto decoded = Cnb::DecodeTexture2DFromCnb(textureDocument);
            const auto checker = Pipeline::DecodeImportedImage(textureAssets / "checker.png");
            exactTexture = textureOutput.logicalName == SharedTextureName &&
                textureOutput.assetTypeId == Cnb::CnbAssetTypeId::Texture2D &&
                decoded.width == checker.width && decoded.height == checker.height &&
                decoded.depth == 1 && decoded.faceCount == 1 && decoded.mipCount == 1 &&
                decoded.representations.size() == 1 &&
                decoded.representations.front().format == Cnb::CnbTextureFormat::Rgba8 &&
                decoded.representations.front().levels ==
                    std::vector<std::vector<std::uint8_t>>{checker.rgbaPixels};
        }
        check(nestedRecipe && nestedArt && exactTexture,
              "BuildAsset propagates the art capability and emits the exact nested checker texture");
        const auto capabilityParent = Cnb::CnbDocument::Parse(
            capability.output.bytes, "stage referencing nested shared texture");
        check(capabilityParent.ExternalReferences().size() == 1 &&
                  capabilityParent.ExternalReferenceAt(0, "nested texture").logicalName == SharedTextureName &&
                  capabilityParent.ExternalReferenceAt(0, "nested texture").expectedAssetTypeId ==
                      Cnb::CnbAssetTypeId::Texture2D &&
                  capability.runtimeReferences.size() == 1 &&
                  capability.runtimeReferences.front().logicalName == SharedTextureName,
              "the parent records the propagated nested texture as its runtime reference and XREF");
        // Authoring and runtime shapes differ: the loaded stage carries its resolved profile.
        ContentManager::RegisterCnbLoaderEXT<SharedProfile>(Cnb::CnbAssetTypeIdFromName(std::string(Profile::XnaTypeName)),
            std::string(Profile::XnaTypeName), [](const Cnb::CnbDocument& document, ContentManager&) -> SharedProfile {
                ++profileReads;
                document.RequireAsset(Cnb::CnbAssetTypeIdFromName(std::string(Profile::XnaTypeName)), 1);
                const std::array known{DataChunk}; document.RequireMandatoryChunksUnderstood(known);
                auto data = document.OpenChunk(document.RequireSingle(DataChunk));
                Profile profile{data.ReadString(), static_cast<int>(data.ReadU32())}; data.RequireExhausted();
                return std::make_shared<const Profile>(std::move(profile));
            });
        ContentManager::RegisterCnbLoaderEXT<LoadedStage>(Cnb::CnbAssetTypeIdFromName(std::string(Stage::XnaTypeName)),
            std::string(Stage::XnaTypeName), [](const Cnb::CnbDocument& document, ContentManager& owner) {
                ++stageReads;
                document.RequireAsset(Cnb::CnbAssetTypeIdFromName(std::string(Stage::XnaTypeName)), 1);
                const std::array known{DataChunk}; document.RequireMandatoryChunksUnderstood(known);
                auto data = document.OpenChunk(document.RequireSingle(DataChunk));
                const auto health = static_cast<int>(data.ReadU32()); data.RequireExhausted();
                const auto& profile = document.ExternalReferenceAt(0, "stage profile");
                if (profile.expectedAssetTypeId != Cnb::CnbAssetTypeIdFromName(std::string(Profile::XnaTypeName)))
                    throw ContentLoadException("The stage needs an enemy profile.");
                return LoadedStage{health, owner.Load<SharedProfile>(profile.logicalName)};
            });
        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-nested-output-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not reserve a private content directory.");
        {
            TemporaryOutput cleanup{directory};
            std::cout << "Private compiled content: " << directory << '\n';
            Publish(directory, compiled.logicalName, compiled.output.bytes);
            ContentManager content(nullptr, directory.string());
            bool childMissing = false;
            try { (void)content.Load<LoadedStage>(compiled.logicalName); }
            catch (const ContentLoadException& error) { childMissing = true; std::cout << "Missing child refused: " << error.what() << '\n'; }
            check(childMissing && stageReads == 1 && profileReads == 0,
                  "publishing only the parent cannot load its not-yet-published compiled child");
            for (const auto& childOutput : compiled.output.additionalOutputs)
                Publish(directory, childOutput.logicalName, childOutput.bytes);
            const auto live = content.Load<LoadedStage>(compiled.logicalName);
            check(live.healthBudget == 240 && live.profile && live.profile->name == "Guard" && live.profile->health == 80 &&
                  stageReads == 2 && profileReads == 1,
                  "publishing the actual additional output permits parent retry without Unload and restores the processed profile");
            const auto direct = content.Load<SharedProfile>(ChildName);
            const auto cached = content.Load<LoadedStage>(compiled.logicalName);
            check(direct == live.profile && cached.profile == live.profile && stageReads == 2 && profileReads == 1,
                  "the parent's external load and direct runtime loads share the same manager's compiled profile cache");
            // A matching project item owns the child build; an empty assetName lets BuildAsset recognize it.
            const std::string scheduledName = "profiles/scheduled_guard";
            Pipeline::ContentBuildSibling sibling;
            sibling.source = std::filesystem::canonical(request.sourceRoot / "guard.profile");
            sibling.logicalName = scheduledName; sibling.importer = "CnaDemos.NestedProfileImporter";
            sibling.processor = ProfileProcessorName; sibling.parameters.Set("Scale", std::int64_t{2});
            auto scheduledRequest = request;
            scheduledRequest.logicalName = "stages/scheduled_arena";
            scheduledRequest.parameters.Set("UseSiblingName", true);
            scheduledRequest.siblings = std::make_shared<const std::vector<Pipeline::ContentBuildSibling>>(
                std::vector<Pipeline::ContentBuildSibling>{sibling});
            const int writesBefore = profileWrites.load();
            const auto scheduled = pipeline.Build(scheduledRequest);
            const auto scheduledDocument = Cnb::CnbDocument::Parse(scheduled.output.bytes, "stage referencing a scheduled item");
            check(profileWrites == writesBefore && scheduled.output.additionalOutputs.empty() &&
                  scheduled.runtimeReferences.size() == 1 && scheduled.runtimeReferences.front().logicalName == scheduledName &&
                  scheduledDocument.ExternalReferences().size() == 1 &&
                  scheduledDocument.ExternalReferenceAt(0, "scheduled profile").logicalName == scheduledName,
                  "an unnamed BuildAsset recognizes the matching project item and references its name without compiling it");
            bool profileSource = false, contentBuildDependency = false;
            for (const auto& dependency : scheduled.dependencies)
            {
                profileSource |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                    dependency.identity == sibling.source.generic_string();
                contentBuildDependency |= dependency.kind == Pipeline::ContentDependencyKind::ContentBuild;
            }
            check(profileSource && !contentBuildDependency && scheduled.writerSchemas.size() == 1 &&
                  scheduled.writerSchemas.front().assetTypeName == Stage::XnaTypeName,
                  "processed budget input retains its source dependency while the sibling reference adds no build edge or child writer schema");
            Publish(directory, scheduled.logicalName, scheduled.output.bytes);
            const int stagesBefore = stageReads.load(), profilesBefore = profileReads.load();
            bool scheduledMissing = false;
            try { (void)content.Load<LoadedStage>(scheduled.logicalName); }
            catch (const ContentLoadException& error)
            {
                scheduledMissing = true;
                std::cout << "Unpublished scheduled item refused: " << error.what() << '\n';
            }
            check(scheduledMissing && stageReads == stagesBefore + 1 && profileReads == profilesBefore,
                  "publishing the referencing parent alone does not manufacture the separately scheduled item");
            Pipeline::ContentBuildRequest childRequest;
            childRequest.sourceRoot = request.sourceRoot; childRequest.source = sibling.source;
            childRequest.logicalName = sibling.logicalName; childRequest.importer = sibling.importer;
            childRequest.processor = sibling.processor; childRequest.parameters = sibling.parameters;
            const auto builtItem = pipeline.Build(childRequest); // the host explicitly executes that other item
            Publish(directory, builtItem.logicalName, builtItem.output.bytes);
            const auto resolved = content.Load<LoadedStage>(scheduled.logicalName);
            const auto scheduledDirect = content.Load<SharedProfile>(scheduledName);
            check(profileWrites == writesBefore + 1 && resolved.healthBudget == 240 && resolved.profile &&
                  resolved.profile->name == "Guard" && resolved.profile->health == 80 && scheduledDirect == resolved.profile &&
                  stageReads == stagesBefore + 2 && profileReads == profilesBefore + 1,
                  "the host's separate item build and publication enables parent retry and the scheduled profile's runtime cache");
            sibling.parameters.Set("Scale", std::int64_t{3}); // now a different request, despite the same source/type
            auto mismatched = scheduledRequest;
            mismatched.siblings = std::make_shared<const std::vector<Pipeline::ContentBuildSibling>>(
                std::vector<Pipeline::ContentBuildSibling>{sibling});
            const auto generated = pipeline.Build(mismatched);
            bool generatedCorrectly = false;
            if (generated.output.additionalOutputs.size() == 1)
            {
                const auto& generatedChild = generated.output.additionalOutputs.front();
                const auto generatedParent = Cnb::CnbDocument::Parse(generated.output.bytes, "stage with generated child");
                const auto profileDocument = Cnb::CnbDocument::Parse(generatedChild.bytes, "generated profile");
                auto data = profileDocument.OpenChunk(profileDocument.RequireSingle(DataChunk));
                const auto name = data.ReadString(); const auto health = data.ReadU32(); data.RequireExhausted();
                generatedCorrectly = generatedChild.logicalName != scheduledName && name == "Guard" && health == 80 &&
                    generatedParent.ExternalReferenceAt(0, "generated profile").logicalName == generatedChild.logicalName;
            }
            check(generatedCorrectly && profileWrites == writesBefore + 2,
                  "different sibling parameters prevent reuse and compile the requested profile under its own generated name");
        }
        check(!std::filesystem::exists(directory), "the private published parent and child are removed after their manager is destroyed");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Nested content build failed: " << error.what() << '\n';
        return 1;
    }
}
