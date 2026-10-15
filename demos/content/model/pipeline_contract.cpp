// SPDX-License-Identifier: MS-PL
// Audit the public Model pipeline contract from glTF staging through generated child CNBs.
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbAnimationClipCodec.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbModelCodec.hpp"
#include "CNA/Content/Cnb/CnbModelV2Codec.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
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

        ctx.Print("Built {} Model part(s), {} Texture2D child and a standalone AnimationClip.",
                  model.parts.size(), built.output.additionalOutputs.size());
    }
}

CNA_DEMOS_CONSOLE(ModelPipelineContract, {
    .id = "content.model.pipeline_contract",
    .title = "Audit the Model pipeline contract",
    .description = "Builds an embedded-texture glTF through CNA's public Model importer, processor and writer; verifies staging lifetime, component/schema identities, generated-child policy, canonical bundle variants, deterministic Model/Texture2D CNBs and typed runtime references.",
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
             "CNA::Content::Cnb::DecodeModelFromCnb",
             "CNA::Content::Cnb::DecodeTexture2DFromCnb"},
    .tags = {"content", "pipeline", "model", "gltf", "generated", "texture", "cnaext"},
    .assets = {"models/loose_textured_crate.gltf", "models/loose_slide_rig.gltf"},
    .restrictions = "Headless CPU conversion and exact canonical/container checks. Existing graphics.model demos own GPU upload, rendering, animation and visual evidence; no screenshot or genuine-XNA execution is claimed.",
})
