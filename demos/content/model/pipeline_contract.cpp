// SPDX-License-Identifier: MS-PL
// Audit the public Model pipeline contract from glTF staging through generated child CNBs.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbAnimationClipCodec.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbModelCodec.hpp"
#include "CNA/Content/Cnb/CnbModelFromCnj.hpp"
#include "CNA/Content/Cnb/CnbModelV2Codec.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::Graphics::AnimationClipEXT;
using Microsoft::Xna::Framework::Graphics::BoneTrackEXT;
using Microsoft::Xna::Framework::Graphics::ClipTargetSpaceEXT;
using Microsoft::Xna::Framework::Graphics::KeyframeEXT;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    class RecordingLogger final : public Pipeline::ContentBuildLogger
    {
    public:
        void Log(const Pipeline::ContentLogMessage& message) override
        {
            messages.push_back(message);
        }

        std::vector<Pipeline::ContentLogMessage> messages;
    };

    class TemporaryDirectory
    {
    public:
        explicit TemporaryDirectory(std::filesystem::path path) : path_(std::move(path)) {}
        ~TemporaryDirectory()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path_, ignored);
        }

        TemporaryDirectory(const TemporaryDirectory&) = delete;
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    private:
        std::filesystem::path path_;
    };

    void WriteText(const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream output(path, std::ios::binary);
        output << text;
        if (!output) throw std::runtime_error("Could not write a temporary Model CNJ");
    }

    bool SameAdditionalOutputs(
        const std::vector<Pipeline::ContentAdditionalWriteOutput>& left,
        const std::vector<Pipeline::ContentAdditionalWriteOutput>& right)
    {
        if (left.size() != right.size()) return false;
        for (std::size_t index = 0u; index < left.size(); ++index)
        {
            if (left[index].logicalName != right[index].logicalName ||
                left[index].bytes != right[index].bytes ||
                left[index].assetTypeId != right[index].assetTypeId ||
                left[index].assetTypeName != right[index].assetTypeName ||
                left[index].assetSchemaVersion != right[index].assetSchemaVersion ||
                left[index].rootReaderName != right[index].rootReaderName ||
                left[index].fromNestedBuild != right[index].fromNestedBuild)
                return false;
        }
        return true;
    }

    template<class Function>
    bool Refuses(Function&& function)
    {
        try
        {
            function();
            return false;
        }
        catch (const std::exception&)
        {
            return true;
        }
    }

    std::array<float, 16> Matrix(const float translation)
    {
        return {{1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 translation, translation + 1.0f, translation + 2.0f, 1.0f}};
    }

    bool SameKey(const KeyframeEXT& left, const KeyframeEXT& right)
    {
        return left.Time == right.Time && left.Translation == right.Translation &&
            left.Rotation == right.Rotation && left.Scale == right.Scale;
    }

    bool SameClip(const AnimationClipEXT& left, const AnimationClipEXT& right)
    {
        if (left.Duration != right.Duration || left.TargetSpace != right.TargetSpace ||
            left.Tracks.size() != right.Tracks.size()) return false;
        for (std::size_t track = 0u; track < left.Tracks.size(); ++track)
        {
            if (left.Tracks[track].BoneIndex != right.Tracks[track].BoneIndex ||
                left.Tracks[track].Keys.size() != right.Tracks[track].Keys.size()) return false;
            for (std::size_t key = 0u; key < left.Tracks[track].Keys.size(); ++key)
            {
                if (!SameKey(left.Tracks[track].Keys[key], right.Tracks[track].Keys[key]))
                    return false;
            }
        }
        return true;
    }

    Cnb::CnbModelData CompleteModel()
    {
        Cnb::CnbModelData model;
        model.bones = {{"Root", -1, Matrix(3.0f)}, {"Child", 0, Matrix(6.0f)}};
        model.hasBoneHierarchy = true;
        model.appliesGltfLightingPolicy = true;

        Cnb::CnbMaterial material;
        material.baseColorTexture = "Textures/base";
        material.texture2 = "Textures/second";
        material.normalMap = "Textures/normal";
        material.metallicRoughnessMap = "Textures/metallic-roughness";
        material.emissiveMap = "Textures/emissive";
        material.occlusionMap = "Textures/occlusion";
        material.specularMap = "Textures/specular";
        material.specularColorMap = "Textures/specular-color";
        material.baseColorFactor = {{0.1f, 0.2f, 0.3f, 0.4f}};
        material.emissiveFactor = {{0.5f, 0.6f, 0.7f}};
        material.specularColorFactor = {{0.8f, 0.9f, 1.0f}};
        material.metallicFactor = 0.11f;
        material.roughnessFactor = 0.22f;
        material.ior = 1.33f;
        material.specularFactor = 0.44f;
        material.normalScale = 0.55f;
        material.occlusionStrength = 0.66f;
        material.alphaCutoff = 0.77f;
        material.alphaMode = 2u;
        material.doubleSided = true;
        for (std::size_t slot = 0u; slot < Cnb::CnbTextureSlotCount; ++slot)
        {
            material.textureCoordinateSets[slot] = static_cast<std::uint8_t>(slot % 2u);
            material.textureTransforms[slot] = {
                static_cast<float>(slot) + 0.1f, static_cast<float>(slot) + 0.2f,
                static_cast<float>(slot) + 1.3f, static_cast<float>(slot) + 1.4f,
                static_cast<float>(slot) + 0.5f};
            material.samplers[slot] = {
                static_cast<std::uint32_t>(slot + 1u),
                static_cast<std::uint32_t>(slot + 11u),
                static_cast<std::uint32_t>(slot + 21u), slot % 2u == 0u};
        }

        const std::array<Cnb::CnbEffectKind, 6> effects{
            Cnb::CnbEffectKind::BasicEffect, Cnb::CnbEffectKind::SkinnedEffect,
            Cnb::CnbEffectKind::DualTextureEffect, Cnb::CnbEffectKind::PbrEffect,
            Cnb::CnbEffectKind::SkinnedPbrEffect, Cnb::CnbEffectKind::External};
        for (std::size_t index = 0u; index < effects.size(); ++index)
        {
            Cnb::CnbModelPart part;
            part.name = "Part" + std::to_string(index);
            part.vertexStride = 4u;
            part.vertexCount = 3u;
            part.indexCount = 3u;
            part.indexElementSize = 2u;
            part.primitiveTopology = 4u;
            part.primitiveCount = 1u;
            part.effectKind = effects[index];
            if (effects[index] == Cnb::CnbEffectKind::External)
                part.externalEffect = "Effects/custom";
            part.vertexColorEnabled = index % 2u == 0u;
            part.unlit = index % 2u != 0u;
            part.material = material;
            part.vertexBytes = {static_cast<std::uint8_t>(index), 1u, 2u, 3u,
                                4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u};
            part.indexBytes = {0u, 0u, 1u, 0u, 2u, 0u};
            model.parts.push_back(std::move(part));
        }

        Cnb::CnbMorphData cubicMorph;
        cubicMorph.vertexCount = 3u;
        cubicMorph.recomputeFlatNormals = true;
        cubicMorph.targets = {
            {{0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f},
             {},
             {1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f}},
            {{}, {-0.1f, -0.2f, -0.3f, -0.4f, -0.5f, -0.6f, -0.7f, -0.8f, -0.9f}, {}}};
        cubicMorph.weights = {0.25f, 0.75f};
        cubicMorph.weightTrackCubicSpline = true;
        cubicMorph.weightTrackKeys = {
            {0.5, {0.2f, 0.8f}, {-0.1f, -0.2f}, {0.3f, 0.4f}}};
        model.parts[0].morph = cubicMorph;

        Cnb::CnbMorphData stepMorph;
        stepMorph.vertexCount = 3u;
        stepMorph.targets = {{{0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f}, {}, {}}};
        stepMorph.weights = {1.0f};
        stepMorph.weightTrackStepInterpolation = true;
        stepMorph.weightTrackKeys = {{1.25, {0.5f}, {}, {}}};
        model.parts[1].morph = stepMorph;

        model.meshes = {{"AllEffects", 1, {0u, 1u, 2u, 3u, 4u, 5u}}};
        model.skeleton = Cnb::CnbModelSkeleton{
            {-1, 0}, {Matrix(10.0f), Matrix(20.0f)},
            {Matrix(30.0f), Matrix(40.0f)}, {Matrix(50.0f), Matrix(60.0f)}};

        AnimationClipEXT clip;
        clip.Duration = System::TimeSpan::FromSeconds(2.0);
        clip.TargetSpace = ClipTargetSpaceEXT::JointPalette;
        clip.Tracks = {{1, {{System::TimeSpan::FromSeconds(0.75),
                            {1.0f, 2.0f, 3.0f}, {0.1f, 0.2f, 0.3f, 0.9f},
                            {1.5f, 1.6f, 1.7f}}}}};
        model.animations = {{"Wave", clip}};
        model.lights = {{{{0.0f, 1.0f, 0.0f}}, {{0.9f, 0.8f, 0.7f}}},
                        {{{-1.0f, 0.0f, 0.0f}}, {{0.2f, 0.3f, 0.4f}}}};
        return model;
    }

    Cnb::CnbDocument EmptyModelDocument(const std::uint32_t assetType,
                                        const std::uint32_t schema,
                                        const int headerCopies = 1,
                                        const bool includeStrings = true)
    {
        Cnb::CnbByteWriter header;
        for (int field = 0; field < 6; ++field) header.WriteU32(0u);
        Cnb::CnbByteWriter emptyCount;
        emptyCount.WriteU32(0u);
        const std::vector<std::uint8_t> headerBytes = header.Take();
        const std::vector<std::uint8_t> countBytes = emptyCount.Take();
        Cnb::CnbWriter writer(assetType, schema);
        writer.SetMetadata("Microsoft.Xna.Framework.Graphics.Model", "synthetic/empty");
        for (int copy = 0; copy < headerCopies; ++copy)
            writer.AddChunk(Cnb::CnbModelChunk::Header, headerBytes,
                            Cnb::CnbChunkFlags::Mandatory, 4u);
        if (includeStrings)
            writer.AddChunk(Cnb::CnbModelChunk::Strings, countBytes,
                            Cnb::CnbChunkFlags::Mandatory, 4u);
        writer.AddChunk(Cnb::CnbModelChunk::Meshes, countBytes,
                        Cnb::CnbChunkFlags::Mandatory, 4u);
        writer.AddChunk(Cnb::CnbModelChunk::Materials, countBytes,
                        Cnb::CnbChunkFlags::Mandatory, 4u);
        return Cnb::CnbDocument::Parse(writer.Build(), "synthetic empty Model CNB");
    }

    void ModelPipelineContract(CnaDemos::DemoContext& ctx)
    {
        const Pipeline::GltfImporter importer;
        const Pipeline::ModelProcessor processor;
        const Pipeline::ModelContentWriter writer;
        const std::vector<Pipeline::ContentWriterSchemaIdentity> schemas =
            writer.OutputSchemaIdentities();
        ctx.Check(Pipeline::ImportedModelDocumentType ==
                          std::string("CNA.Content.Pipeline.ImportedModelDocument") &&
                      Pipeline::ProcessedModelType ==
                          std::string("CNA.Content.Pipeline.ProcessedModelBundle") &&
                      Pipeline::ModelGenerateChildAssetsParameter ==
                          std::string("generateChildAssets") &&
                      importer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.GltfImporter", "2"} &&
                      importer.SourceExtensions() ==
                          std::vector<std::string>{".gltf", ".glb"} &&
                      importer.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedModelDocumentType} &&
                      processor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.ModelProcessor", "3"} &&
                      processor.InputType() == Pipeline::ImportedModelDocumentType &&
                      processor.OutputType() == Pipeline::ProcessedModelType &&
                      writer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.ModelContentWriter", "3"} &&
                      writer.InputType() == Pipeline::ProcessedModelType,
                  "the Model route exposes exact type, parameter, component, extension and version identities");

        const std::vector<Pipeline::ContentWriterSchemaIdentity> expectedSchemas{
            {Cnb::CnbAssetTypeId::Texture2D, Cnb::CnbTextureSchemaVersion,
             "Microsoft.Xna.Framework.Graphics.Texture2D",
             {"CNA.Cnb.EncodeTexture2DToCnb", "1"}},
            {Cnb::CnbAssetTypeId::Model, Cnb::CnbModelSchemaVersion,
             "Microsoft.Xna.Framework.Graphics.Model", {"CNA.Cnb.EncodeModelToCnb", "1"}},
            {Cnb::CnbAssetTypeId::Model, Cnb::CnbModelV2SchemaVersion,
             "Microsoft.Xna.Framework.Graphics.Model", {"CNA.Cnb.EncodeModelV2ToCnb", "1"}},
            {Cnb::CnbAssetTypeId::AnimationClip, Cnb::CnbAnimationClipSchemaVersion,
             "Microsoft.Xna.Framework.Graphics.AnimationClipEXT",
             {"CNA.Cnb.EncodeAnimationClipToCnb", "1"}},
        };
        ctx.Check(schemas == expectedSchemas,
                  "the writer declares sorted Texture2D, Model schema-1, Model schema-2 and AnimationClip codec identities");

        Pipeline::ContentProcessorParameters parameters;
        parameters.Set(Pipeline::ModelGenerateChildAssetsParameter, true);
        bool validAccepted = true;
        try { processor.ValidateParameters(parameters); }
        catch (...) { validAccepted = false; }
        Pipeline::ContentProcessorParameters unknown;
        unknown.Set("generateChildren", true);
        bool unknownRefused = false;
        try { processor.ValidateParameters(unknown); }
        catch (const Pipeline::ContentParameterError& error)
        {
            unknownRefused = error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "generateChildren";
        }
        Pipeline::ContentProcessorParameters mistyped;
        mistyped.Set(Pipeline::ModelGenerateChildAssetsParameter, std::string("true"));
        bool mistypedRefused = false;
        try { processor.ValidateParameters(mistyped); }
        catch (const std::invalid_argument& error)
        {
            mistypedRefused = std::string(error.what()).find("must be a bool") !=
                std::string::npos;
        }
        ctx.Check(validAccepted && unknownRefused && mistypedRefused,
                  "processor validation accepts the bool child policy and distinguishes an unknown name from a mistyped value");

        const std::filesystem::path assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        const std::filesystem::path source =
            assetRoot / "models/loose_textured_crate.gltf";
        std::filesystem::path releasedIntermediateRoot;
        {
            Pipeline::ContentSourceRootCapabilities externalRoots;
            Pipeline::ContentDependencyCollector dependencies;
            RecordingLogger logger;
            Pipeline::ContentImporterContext importContext(
                assetRoot, source, "Models/pipeline-contract", importer.Identity().name,
                externalRoots, dependencies, logger);
            const Pipeline::ContentValue importedValue = importer.Import(importContext);
            const Pipeline::ImportedModelDocument& imported =
                importedValue.Get<Pipeline::ImportedModelDocument>();
            releasedIntermediateRoot = imported.intermediateRoot;
            const bool textureContained = imported.generatedTextureFiles.size() == 1u &&
                imported.generatedTextureFiles[0].parent_path() == imported.intermediateRoot &&
                std::filesystem::is_regular_file(imported.generatedTextureFiles[0]);
            ctx.Check(std::filesystem::is_regular_file(imported.document) &&
                          imported.document.parent_path() == imported.intermediateRoot &&
                          imported.additionalModelDocuments.empty() &&
                          imported.animationDocuments.empty() && textureContained &&
                          imported.generatedBaseName == "loose_textured_crate" &&
                          !imported.intermediateRoot.empty() &&
                          static_cast<bool>(imported.intermediateLifetime) &&
                          !imported.recordAuthoredSidecars &&
                          !imported.canonicalModel.has_value() && !logger.messages.empty(),
                      "the glTF importer owns one canonical Model document and embedded texture in a live contained staging tree");

            const Cnb::CnbModelFromCnjResult nativeFrontEnd =
                Cnb::BuildCnbModelFromCnj(imported.document, imported.intermediateRoot);
            const Cnb::CnbModelFromCnjResult narrowFrontEnd = Cnb::BuildCnbModelFromCnj(
                imported.document.string(), imported.intermediateRoot.string());
            const bool absorbedFilesExist = std::all_of(
                nativeFrontEnd.absorbedFiles.begin(), nativeFrontEnd.absorbedFiles.end(),
                [&imported](const std::string& name)
                {
                    return std::filesystem::is_regular_file(imported.intermediateRoot / name);
                });
            const std::string stagedTexture = std::filesystem::relative(
                imported.generatedTextureFiles[0], imported.intermediateRoot).generic_string();
            ctx.Check(!nativeFrontEnd.model.parts.empty() &&
                          !nativeFrontEnd.model.meshes.empty() &&
                          nativeFrontEnd.absorbedFiles.size() >= 2u && absorbedFilesExist &&
                          nativeFrontEnd.externalReferences ==
                              std::vector<std::string>{stagedTexture},
                      "the public CNJ front end absorbs generated geometry sidecars and retains the staged texture as one external asset");
            ctx.Check(nativeFrontEnd.absorbedFiles == narrowFrontEnd.absorbedFiles &&
                          nativeFrontEnd.externalReferences == narrowFrontEnd.externalReferences &&
                          Cnb::EncodeModelToCnb(nativeFrontEnd.model, "Models/direct-front-end") ==
                              Cnb::EncodeModelToCnb(narrowFrontEnd.model,
                                                    "Models/direct-front-end"),
                      "native filesystem and compatibility narrow-path overloads produce identical model and file lists");
        }
        ctx.Check(!releasedIntermediateRoot.empty() &&
                      !std::filesystem::exists(releasedIntermediateRoot),
                  "releasing the imported carrier removes its opaque temporary staging tree");

        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterModelContentPipeline(*registry);
        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = assetRoot;
        request.source = "models/loose_textured_crate.gltf";
        request.logicalName = "Models/pipeline-contract";
        request.parameters.Set(Pipeline::ModelGenerateChildAssetsParameter, true);

        Pipeline::ContentDependencyCollector dependencies;
        const Pipeline::ContentProcessResult processed =
            pipeline.ImportAndProcess(request, dependencies);
        const Pipeline::ProcessedModelBundle& bundle =
            processed.processed.Get<Pipeline::ProcessedModelBundle>();
        const bool schema1 = bundle.primary.has_value() &&
            std::holds_alternative<Cnb::CnbModelData>(*bundle.primary);
        const bool textureChild = bundle.children.size() == 1u &&
            std::holds_alternative<Cnb::CnbTextureData>(bundle.children[0].value);
        ctx.Check(processed.importer == importer.Identity() &&
                      processed.processor == processor.Identity() && schema1 &&
                      !bundle.xnaModel.has_value() && textureChild &&
                      bundle.children[0].logicalName.starts_with(request.logicalName + "_") &&
                      bundle.children[0].logicalName.ends_with(".png"),
                  "registered import and processing produce one schema-1 primary plus a deterministically named Texture2D child bundle");

        const Pipeline::ContentBuildResult built = pipeline.Build(request);
        const Pipeline::ContentBuildResult repeated = pipeline.Build(request);
        const Cnb::CnbModelData model = Cnb::DecodeModelFromCnb(
            Cnb::CnbDocument::Parse(built.output.bytes, "model pipeline contract CNB"));
        const bool namedTextureReference = std::any_of(
            model.parts.begin(), model.parts.end(),
            [&bundle](const Cnb::CnbModelPart& part)
            {
                return part.material.baseColorTexture == bundle.children[0].logicalName;
            });
        ctx.Check(built.importer == importer.Identity() &&
                      built.processor == processor.Identity() &&
                      built.writer == writer.Identity() &&
                      built.writerSchemas == expectedSchemas &&
                      built.output.assetTypeId == Cnb::CnbAssetTypeId::Model &&
                      built.output.assetSchemaVersion == Cnb::CnbModelSchemaVersion &&
                      built.output.assetTypeName ==
                          "Microsoft.Xna.Framework.Graphics.Model" &&
                      !model.parts.empty() && !model.meshes.empty() && namedTextureReference,
                  "the full build writes a schema-1 Model whose material names the generated texture child");

        const bool oneTextureOutput = built.output.additionalOutputs.size() == 1u &&
            built.output.additionalOutputs[0].logicalName == bundle.children[0].logicalName &&
            built.output.additionalOutputs[0].assetTypeId == Cnb::CnbAssetTypeId::Texture2D &&
            built.output.additionalOutputs[0].assetSchemaVersion ==
                Cnb::CnbTextureSchemaVersion &&
            built.output.additionalOutputs[0].assetTypeName ==
                "Microsoft.Xna.Framework.Graphics.Texture2D";
        Cnb::CnbTextureData childTexture;
        if (oneTextureOutput)
        {
            childTexture = Cnb::DecodeTexture2DFromCnb(Cnb::CnbDocument::Parse(
                built.output.additionalOutputs[0].bytes, "generated model texture CNB"));
        }
        const bool typedRuntimeReference = oneTextureOutput &&
            std::find(built.runtimeReferences.begin(), built.runtimeReferences.end(),
                      Pipeline::RuntimeContentReference{
                          built.output.additionalOutputs[0].logicalName,
                          Cnb::CnbAssetTypeId::Texture2D}) != built.runtimeReferences.end();
        ctx.Check(oneTextureOutput && childTexture.width > 0u && childTexture.height > 0u &&
                      childTexture.representations.size() == 1u &&
                      childTexture.representations[0].format ==
                          Cnb::CnbTextureFormat::Rgba8 && typedRuntimeReference,
                  "the writer emits the generated image as a decodable Texture2D CNB and records its typed runtime reference separately");

        ctx.Check(built.output.bytes == repeated.output.bytes &&
                      SameAdditionalOutputs(built.output.additionalOutputs,
                                            repeated.output.additionalOutputs) &&
                      built.dependencies == repeated.dependencies &&
                      built.runtimeReferences == repeated.runtimeReferences &&
                      built.dependencies.size() == 1u &&
                      built.dependencies[0].kind ==
                          Pipeline::ContentDependencyKind::PrimarySource,
                  "repeated embedded-source builds preserve primary bytes, child bytes, dependency identity and runtime references");

        request.source = "models/loose_slide_rig.gltf";
        request.logicalName = "Models/animated-contract";
        const Pipeline::ContentBuildResult animated = pipeline.Build(request);
        const Cnb::CnbModelData animatedModel = Cnb::DecodeModelFromCnb(
            Cnb::CnbDocument::Parse(animated.output.bytes, "animated model pipeline CNB"));
        const auto animationOutput = std::find_if(
            animated.output.additionalOutputs.begin(), animated.output.additionalOutputs.end(),
            [](const Pipeline::ContentAdditionalWriteOutput& child)
            {
                return child.assetTypeId == Cnb::CnbAssetTypeId::AnimationClip;
            });
        bool standaloneAnimation = animationOutput != animated.output.additionalOutputs.end();
        if (standaloneAnimation)
        {
            const auto clip = Cnb::DecodeAnimationClipFromCnb(Cnb::CnbDocument::Parse(
                animationOutput->bytes, "generated model animation CNB"));
            standaloneAnimation = animationOutput->logicalName.starts_with(request.logicalName + "_") &&
                animationOutput->assetSchemaVersion == Cnb::CnbAnimationClipSchemaVersion &&
                !clip.Tracks.empty() && clip.Duration.getTicksProperty() > 0;
        }
        ctx.Check(!animatedModel.animations.empty() && standaloneAnimation,
                  "generated-child mode retains the embedded Model animation and emits its decodable standalone AnimationClip companion");

        ctx.Check(Cnb::CnbModelChunk::Header == Cnb::MakeChunkId('M', 'D', 'L', 'H') &&
                      Cnb::CnbModelChunk::Strings == Cnb::MakeChunkId('M', 'S', 'T', 'R') &&
                      Cnb::CnbModelChunk::Bones == Cnb::MakeChunkId('M', 'B', 'O', 'N') &&
                      Cnb::CnbModelChunk::Meshes == Cnb::MakeChunkId('M', 'M', 'S', 'H') &&
                      Cnb::CnbModelChunk::Materials == Cnb::MakeChunkId('M', 'M', 'A', 'T') &&
                      Cnb::CnbModelChunk::VertexData == Cnb::MakeChunkId('M', 'V', 'T', 'X') &&
                      Cnb::CnbModelChunk::IndexData == Cnb::MakeChunkId('M', 'I', 'D', 'X') &&
                      Cnb::CnbModelChunk::MorphData == Cnb::MakeChunkId('M', 'M', 'R', 'P') &&
                      Cnb::CnbModelChunk::Skeleton == Cnb::MakeChunkId('M', 'S', 'K', 'L') &&
                      Cnb::CnbModelChunk::Animations == Cnb::MakeChunkId('M', 'A', 'N', 'M') &&
                      Cnb::CnbModelChunk::Lights == Cnb::MakeChunkId('M', 'L', 'I', 'T') &&
                      Cnb::CnbModelSchemaVersion == 1u && Cnb::CnbModelBoneStride == 72u &&
                      Cnb::CnbModelMeshStride == 16u && Cnb::CnbModelPartStride == 56u &&
                      Cnb::CnbModelMaterialStride == 368u && Cnb::CnbNoIndex == 0xFFFFFFFFu &&
                      Cnb::CnbTextureSlotCount == 7u && Cnb::CnbMaxEffectKind == 5u,
                  "the schema freezes all eleven Model chunk IDs, version, strides, sentinel and public bounds");

        ctx.Check(static_cast<std::uint32_t>(Cnb::CnbEffectKind::BasicEffect) == 0u &&
                      static_cast<std::uint32_t>(Cnb::CnbEffectKind::SkinnedEffect) == 1u &&
                      static_cast<std::uint32_t>(Cnb::CnbEffectKind::DualTextureEffect) == 2u &&
                      static_cast<std::uint32_t>(Cnb::CnbEffectKind::PbrEffect) == 3u &&
                      static_cast<std::uint32_t>(Cnb::CnbEffectKind::SkinnedPbrEffect) == 4u &&
                      static_cast<std::uint32_t>(Cnb::CnbEffectKind::External) == 5u,
                  "all six persisted Model effect ordinals remain exact");

        const Cnb::CnbModelData complete = CompleteModel();
        const std::vector<std::uint8_t> completeBytes =
            Cnb::EncodeModelToCnb(complete, "Models/complete-codec");
        const Cnb::CnbDocument completeDocument =
            Cnb::CnbDocument::Parse(completeBytes, "complete Model CNB");
        const Cnb::CnbModelData restored = Cnb::DecodeModelFromCnb(completeDocument);
        ctx.Check(completeBytes == Cnb::EncodeModelToCnb(complete, "Models/complete-codec") &&
                      completeDocument.Metadata().assetTypeName ==
                          "Microsoft.Xna.Framework.Graphics.Model" &&
                      completeDocument.Metadata().contentName == "Models/complete-codec" &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Header).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Strings).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Bones).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Meshes).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Materials).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::VertexData).size() == 6u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::IndexData).size() == 6u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::MorphData).size() == 2u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Skeleton).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Animations).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelChunk::Lights).size() == 1u,
                  "a complete Model encodes deterministically with exact metadata and schema chunk cardinalities");

        ctx.Check(restored.hasBoneHierarchy && restored.appliesGltfLightingPolicy &&
                      restored.bones.size() == 2u && restored.bones[0].name == "Root" &&
                      restored.bones[0].parent == -1 && restored.bones[0].transform == Matrix(3.0f) &&
                      restored.bones[1].name == "Child" && restored.bones[1].parent == 0 &&
                      restored.bones[1].transform == Matrix(6.0f),
                  "Model flags and the parent-before-child bone names, indices and transforms round trip exactly");

        bool partsExact = restored.parts.size() == complete.parts.size();
        for (std::size_t index = 0u; partsExact && index < restored.parts.size(); ++index)
        {
            const auto& left = restored.parts[index];
            const auto& right = complete.parts[index];
            partsExact = left.name == right.name && left.vertexStride == right.vertexStride &&
                left.vertexCount == right.vertexCount && left.indexCount == right.indexCount &&
                left.indexElementSize == right.indexElementSize &&
                left.primitiveTopology == right.primitiveTopology &&
                left.primitiveCount == right.primitiveCount && left.effectKind == right.effectKind &&
                left.externalEffect == right.externalEffect &&
                left.vertexColorEnabled == right.vertexColorEnabled && left.unlit == right.unlit &&
                left.vertexBytes == right.vertexBytes && left.indexBytes == right.indexBytes;
        }
        ctx.Check(partsExact && restored.meshes.size() == 1u &&
                      restored.meshes[0].name == "AllEffects" &&
                      restored.meshes[0].parentBone == 1 &&
                      restored.meshes[0].partIndices ==
                          std::vector<std::uint32_t>({0u, 1u, 2u, 3u, 4u, 5u}),
                  "all six effect kinds preserve their part geometry, flags, external effect and mesh ordering");

        const Cnb::CnbMaterial& restoredMaterial = restored.parts[0].material;
        const Cnb::CnbMaterial& sourceMaterial = complete.parts[0].material;
        ctx.Check(restoredMaterial.baseColorTexture == sourceMaterial.baseColorTexture &&
                      restoredMaterial.texture2 == sourceMaterial.texture2 &&
                      restoredMaterial.normalMap == sourceMaterial.normalMap &&
                      restoredMaterial.metallicRoughnessMap == sourceMaterial.metallicRoughnessMap &&
                      restoredMaterial.emissiveMap == sourceMaterial.emissiveMap &&
                      restoredMaterial.occlusionMap == sourceMaterial.occlusionMap &&
                      restoredMaterial.specularMap == sourceMaterial.specularMap &&
                      restoredMaterial.specularColorMap == sourceMaterial.specularColorMap &&
                      restoredMaterial.baseColorFactor == sourceMaterial.baseColorFactor &&
                      restoredMaterial.emissiveFactor == sourceMaterial.emissiveFactor &&
                      restoredMaterial.specularColorFactor == sourceMaterial.specularColorFactor &&
                      restoredMaterial.metallicFactor == sourceMaterial.metallicFactor &&
                      restoredMaterial.roughnessFactor == sourceMaterial.roughnessFactor &&
                      restoredMaterial.ior == sourceMaterial.ior &&
                      restoredMaterial.specularFactor == sourceMaterial.specularFactor &&
                      restoredMaterial.normalScale == sourceMaterial.normalScale &&
                      restoredMaterial.occlusionStrength == sourceMaterial.occlusionStrength &&
                      restoredMaterial.alphaCutoff == sourceMaterial.alphaCutoff &&
                      restoredMaterial.alphaMode == sourceMaterial.alphaMode &&
                      restoredMaterial.doubleSided == sourceMaterial.doubleSided,
                  "all material texture references, factors, alpha mode and sidedness round trip exactly");

        bool textureStateExact =
            restoredMaterial.textureCoordinateSets == sourceMaterial.textureCoordinateSets;
        for (std::size_t slot = 0u; textureStateExact && slot < Cnb::CnbTextureSlotCount; ++slot)
        {
            const auto& leftTransform = restoredMaterial.textureTransforms[slot];
            const auto& rightTransform = sourceMaterial.textureTransforms[slot];
            const auto& leftSampler = restoredMaterial.samplers[slot];
            const auto& rightSampler = sourceMaterial.samplers[slot];
            textureStateExact = leftTransform.offsetX == rightTransform.offsetX &&
                leftTransform.offsetY == rightTransform.offsetY &&
                leftTransform.scaleX == rightTransform.scaleX &&
                leftTransform.scaleY == rightTransform.scaleY &&
                leftTransform.rotation == rightTransform.rotation &&
                leftSampler.filter == rightSampler.filter &&
                leftSampler.addressU == rightSampler.addressU &&
                leftSampler.addressV == rightSampler.addressV &&
                leftSampler.declared == rightSampler.declared;
        }
        ctx.Check(textureStateExact,
                  "all seven coordinate-set, UV-transform and sampler records remain distinct");

        const auto& cubic = *restored.parts[0].morph;
        const auto& step = *restored.parts[1].morph;
        ctx.Check(cubic.vertexCount == 3u && cubic.recomputeFlatNormals &&
                      cubic.targets.size() == 2u &&
                      cubic.targets[0].positionDeltas ==
                          complete.parts[0].morph->targets[0].positionDeltas &&
                      cubic.targets[0].normalDeltas.empty() &&
                      cubic.targets[0].tangentDeltas ==
                          complete.parts[0].morph->targets[0].tangentDeltas &&
                      cubic.targets[1].positionDeltas.empty() &&
                      cubic.targets[1].normalDeltas ==
                          complete.parts[0].morph->targets[1].normalDeltas &&
                      cubic.weights == std::vector<float>({0.25f, 0.75f}) &&
                      cubic.weightTrackCubicSpline && !cubic.weightTrackStepInterpolation &&
                      cubic.weightTrackKeys.size() == 1u &&
                      cubic.weightTrackKeys[0].timeSeconds == 0.5 &&
                      cubic.weightTrackKeys[0].weights == std::vector<float>({0.2f, 0.8f}) &&
                      cubic.weightTrackKeys[0].inTangent == std::vector<float>({-0.1f, -0.2f}) &&
                      cubic.weightTrackKeys[0].outTangent == std::vector<float>({0.3f, 0.4f}) &&
                      step.weightTrackStepInterpolation && !step.weightTrackCubicSpline &&
                      step.weightTrackKeys[0].weights == std::vector<float>({0.5f}),
                  "sparse morph streams, default weights and cubic/step weight-track payloads round trip");

        ctx.Check(restored.skeleton.has_value() &&
                      restored.skeleton->hierarchy == std::vector<std::int32_t>({-1, 0}) &&
                      restored.skeleton->bindPose == complete.skeleton->bindPose &&
                      restored.skeleton->inverseBindPose == complete.skeleton->inverseBindPose &&
                      restored.skeleton->rootPrefix == complete.skeleton->rootPrefix,
                  "the skeleton preserves hierarchy, bind pose, inverse bind pose and optional root prefixes");
        ctx.Check(restored.animations.size() == 1u && restored.animations[0].name == "Wave" &&
                      SameClip(restored.animations[0].clip, complete.animations[0].clip),
                  "the embedded named animation preserves duration, target space, track and key fields");
        ctx.Check(restored.lights.size() == 2u &&
                      restored.lights[0].direction == complete.lights[0].direction &&
                      restored.lights[0].diffuseColor == complete.lights[0].diffuseColor &&
                      restored.lights[1].direction == complete.lights[1].direction &&
                      restored.lights[1].diffuseColor == complete.lights[1].diffuseColor,
                  "multiple directional lights preserve exact direction and diffuse colour");

        const Cnb::CnbModelData emptyModel;
        const Cnb::CnbModelData restoredEmpty = Cnb::DecodeModelFromCnb(
            Cnb::CnbDocument::Parse(Cnb::EncodeModelToCnb(emptyModel), "empty Model CNB"));
        ctx.Check(restoredEmpty.bones.empty() && restoredEmpty.parts.empty() &&
                      restoredEmpty.meshes.empty() && !restoredEmpty.skeleton.has_value() &&
                      restoredEmpty.animations.empty() && restoredEmpty.lights.empty() &&
                      !restoredEmpty.appliesGltfLightingPolicy && !restoredEmpty.hasBoneHierarchy,
                  "the wholly empty Model remains a valid value distinct from every optional payload");

        Cnb::CnbModelData invalid = complete;
        invalid.bones[1].parent = 1;
        const bool badBone = Refuses([&] { (void)Cnb::EncodeModelToCnb(invalid); });
        invalid = complete;
        invalid.parts[0].vertexBytes.pop_back();
        const bool badGeometry = Refuses([&] { (void)Cnb::EncodeModelToCnb(invalid); });
        invalid = complete;
        invalid.parts[0].primitiveCount = 2u;
        const bool badPrimitiveCount = Refuses([&] { (void)Cnb::EncodeModelToCnb(invalid); });
        invalid = complete;
        invalid.parts[0].morph->targets[0].positionDeltas.pop_back();
        const bool badMorph = Refuses([&] { (void)Cnb::EncodeModelToCnb(invalid); });
        invalid = complete;
        invalid.meshes[0].partIndices.push_back(99u);
        const bool badMesh = Refuses([&] { (void)Cnb::EncodeModelToCnb(invalid); });
        invalid = complete;
        invalid.skeleton->bindPose.pop_back();
        const bool badSkeleton = Refuses([&] { (void)Cnb::EncodeModelToCnb(invalid); });
        ctx.Check(badBone && badGeometry && badPrimitiveCount && badMorph && badMesh && badSkeleton,
                  "the encoder refuses invalid hierarchy, geometry, draw, morph, mesh and skeleton relationships");

        ctx.Check(Refuses([&]
                  {
                      (void)Cnb::DecodeModelFromCnb(EmptyModelDocument(
                          Cnb::CnbAssetTypeId::Curve, Cnb::CnbModelSchemaVersion));
                  }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelFromCnb(EmptyModelDocument(
                              Cnb::CnbAssetTypeId::Model, Cnb::CnbModelSchemaVersion + 1u));
                      }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelFromCnb(EmptyModelDocument(
                              Cnb::CnbAssetTypeId::Model, Cnb::CnbModelSchemaVersion, 0));
                      }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelFromCnb(EmptyModelDocument(
                              Cnb::CnbAssetTypeId::Model, Cnb::CnbModelSchemaVersion, 2));
                      }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelFromCnb(EmptyModelDocument(
                              Cnb::CnbAssetTypeId::Model, Cnb::CnbModelSchemaVersion, 1, false));
                      }),
                  "the decoder refuses wrong identity/version and missing or duplicate mandatory tables");

        const auto encoderRefusesInvalid = [&](const auto& corrupt)
        {
            Cnb::CnbModelData candidate = complete;
            corrupt(candidate);
            try
            {
                (void)Cnb::EncodeModelToCnb(candidate, "Models/invalid");
                return false;
            }
            catch (const Microsoft::Xna::Framework::Content::ContentLoadException&)
            {
                return true;
            }
        };
        ctx.Check(
            encoderRefusesInvalid([](Cnb::CnbModelData& value)
            {
                value.bones[0].parent = -2;
            }) &&
            encoderRefusesInvalid([](Cnb::CnbModelData& value)
            {
                value.meshes[0].parentBone = -2;
            }) &&
            encoderRefusesInvalid([](Cnb::CnbModelData& value)
            {
                value.parts[0].indexBytes[0] = 3u;
            }) &&
            encoderRefusesInvalid([](Cnb::CnbModelData& value)
            {
                value.parts[0].material.alphaMode = 3u;
            }) &&
            encoderRefusesInvalid([](Cnb::CnbModelData& value)
            {
                value.parts[0].material.textureCoordinateSets[0] = 2u;
            }),
            "the encoder refuses all five invalid Model values before producing a container");

        const std::filesystem::path cnjRoot = std::filesystem::temp_directory_path() /
            ("cna-demos-model-front-end-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(cnjRoot))
            throw std::runtime_error("Private Model CNJ directory already exists");
        TemporaryDirectory removeCnjRoot(cnjRoot);
        WriteText(cnjRoot / "minimal-v1.cnj",
                  R"({"cnjVersion":1,"type":"Model","meshes":[]})");
        WriteText(cnjRoot / "minimal-v2.cnj",
                  R"({"cnjVersion":2,"type":"Model","bones":[],"meshes":[]})");
        const Cnb::CnbModelFromCnjResult minimalV1 =
            Cnb::BuildCnbModelFromCnj(cnjRoot / "minimal-v1.cnj", cnjRoot);
        const Cnb::CnbModelFromCnjResult minimalV2 =
            Cnb::BuildCnbModelFromCnj(cnjRoot / "minimal-v2.cnj", cnjRoot);
        ctx.Check(minimalV1.model.bones.size() == 1u &&
                      minimalV1.model.bones[0].name == "Root" &&
                      minimalV1.model.bones[0].parent == -1 &&
                      minimalV1.model.bones[0].transform == Cnb::CnbModelBone{}.transform &&
                      !minimalV1.model.hasBoneHierarchy &&
                      !minimalV1.model.appliesGltfLightingPolicy &&
                      minimalV1.model.parts.empty() && minimalV1.model.meshes.empty() &&
                      minimalV1.absorbedFiles.empty() && minimalV1.externalReferences.empty() &&
                      minimalV2.model.bones.size() == 1u &&
                      minimalV2.model.bones[0].name == minimalV1.model.bones[0].name &&
                      minimalV2.model.bones[0].parent == minimalV1.model.bones[0].parent &&
                      minimalV2.model.bones[0].transform == minimalV1.model.bones[0].transform &&
                      minimalV2.model.appliesGltfLightingPolicy,
                  "minimal version-1/2 documents synthesize the same identity root while preserving the glTF lighting-policy fork");

        WriteText(cnjRoot / "wrong-type.cnj",
                  R"({"cnjVersion":1,"type":"Curve","meshes":[]})");
        WriteText(cnjRoot / "source-file.cnj",
                  R"({"cnjVersion":1,"type":"Model","sourceFile":"model.gltf","meshes":[]})");
        WriteText(cnjRoot / "variants.cnj",
                  R"({"cnjVersion":1,"type":"Model","materialVariantNames":["Gold"],"meshes":[]})");
        WriteText(cnjRoot / "missing-meshes.cnj",
                  R"({"cnjVersion":1,"type":"Model"})");
        WriteText(cnjRoot / "outside-root.cnj",
                  R"({"cnjVersion":1,"type":"Model","meshes":[{"vertices":"../outside.bin","indices":"inside.bin"}]})");
        const auto refusesCnj = [&](const char* filename)
        {
            return Refuses([&]
            {
                (void)Cnb::BuildCnbModelFromCnj(cnjRoot / filename, cnjRoot);
            });
        };
        ctx.Check(refusesCnj("wrong-type.cnj") && refusesCnj("source-file.cnj") &&
                      refusesCnj("variants.cnj") && refusesCnj("missing-meshes.cnj") &&
                      refusesCnj("outside-root.cnj"),
                  "the front end refuses wrong type, source indirection, variants, missing meshes and root escape");

        ctx.Print("Built {} real Model part(s) and round-tripped {} synthetic effect variants.",
                  model.parts.size(), restored.parts.size());
    }
}

CNA_DEMOS_CONSOLE(ModelPipelineContract, {
    .id = "content.model.pipeline_contract",
    .title = "Audit the Model pipeline contract",
    .description = "Builds embedded-texture and animated glTF sources through CNA's public Model pipeline, directly audits the CNJ front end and schema-1 codec, and covers hierarchy, every effect kind, material state, geometry, morphing, skinning, animation and lights.",
    .apis = {"CNA::Content::Pipeline::CanonicalModelValue",
             "CNA::Content::Pipeline::ImportedModelDocumentType",
             "CNA::Content::Pipeline::ProcessedModelType",
             "CNA::Content::Pipeline::ModelGenerateChildAssetsParameter",
             "CNA::Content::Pipeline::ImportedModelDocument",
             "CNA::Content::Pipeline::ProcessedModelChildValue",
             "CNA::Content::Pipeline::ProcessedModelChild",
             "CNA::Content::Pipeline::ProcessedModelBundle",
             "CNA::Content::Pipeline::GltfImporter::Identity",
             "CNA::Content::Pipeline::GltfImporter::SourceExtensions",
             "CNA::Content::Pipeline::GltfImporter::OutputTypes",
             "CNA::Content::Pipeline::GltfImporter::Import",
             "CNA::Content::Pipeline::ModelProcessor::Identity",
             "CNA::Content::Pipeline::ModelProcessor::InputType",
             "CNA::Content::Pipeline::ModelProcessor::OutputType",
             "CNA::Content::Pipeline::ModelProcessor::ValidateParameters",
             "CNA::Content::Pipeline::ModelProcessor::Process",
             "CNA::Content::Pipeline::ModelContentWriter::Identity",
             "CNA::Content::Pipeline::ModelContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::ModelContentWriter::InputType",
             "CNA::Content::Pipeline::ModelContentWriter::Write",
             "CNA::Content::Pipeline::RegisterModelContentPipeline",
             "CNA::Content::Cnb::CnbModelBone",
             "CNA::Content::Cnb::CnbEffectKind",
             "CNA::Content::Cnb::CnbMaxEffectKind",
             "CNA::Content::Cnb::CnbTextureTransform",
             "CNA::Content::Cnb::CnbSamplerState",
             "CNA::Content::Cnb::CnbTextureSlotCount",
             "CNA::Content::Cnb::CnbMaterial",
             "CNA::Content::Cnb::CnbMorphTarget",
             "CNA::Content::Cnb::CnbMorphWeightKey",
             "CNA::Content::Cnb::CnbMorphData",
             "CNA::Content::Cnb::CnbModelPart",
             "CNA::Content::Cnb::CnbModelMesh",
             "CNA::Content::Cnb::CnbModelSkeleton",
             "CNA::Content::Cnb::CnbModelAnimation",
             "CNA::Content::Cnb::CnbModelLight",
             "CNA::Content::Cnb::CnbModelData",
             "CNA::Content::Cnb::CnbModelChunk::Header",
             "CNA::Content::Cnb::CnbModelChunk::Strings",
             "CNA::Content::Cnb::CnbModelChunk::Bones",
             "CNA::Content::Cnb::CnbModelChunk::Meshes",
             "CNA::Content::Cnb::CnbModelChunk::Materials",
             "CNA::Content::Cnb::CnbModelChunk::VertexData",
             "CNA::Content::Cnb::CnbModelChunk::IndexData",
             "CNA::Content::Cnb::CnbModelChunk::MorphData",
             "CNA::Content::Cnb::CnbModelChunk::Skeleton",
             "CNA::Content::Cnb::CnbModelChunk::Animations",
             "CNA::Content::Cnb::CnbModelChunk::Lights",
             "CNA::Content::Cnb::CnbModelSchemaVersion",
             "CNA::Content::Cnb::CnbModelBoneStride",
             "CNA::Content::Cnb::CnbModelMeshStride",
             "CNA::Content::Cnb::CnbModelPartStride",
             "CNA::Content::Cnb::CnbModelMaterialStride",
             "CNA::Content::Cnb::CnbNoIndex",
             "CNA::Content::Cnb::CnbModelFromCnjResult",
             "CNA::Content::Cnb::BuildCnbModelFromCnj",
             "CNA::Content::Cnb::EncodeModelToCnb",
             "CNA::Content::Cnb::DecodeModelFromCnb",
             "CNA::Content::Cnb::DecodeTexture2DFromCnb"},
    .tags = {"content", "pipeline", "model", "gltf", "generated", "texture", "cnaext"},
    .assets = {"models/loose_textured_crate.gltf", "models/loose_slide_rig.gltf"},
    .restrictions = "Headless CPU conversion and exact canonical/container checks. Representative relationship and schema refusals are covered; arbitrary malformed-container fuzzing remains CNA test-suite work. Existing graphics.model demos own GPU upload, rendering, animation and visual evidence; no screenshot or genuine-XNA execution is claimed.",
})
