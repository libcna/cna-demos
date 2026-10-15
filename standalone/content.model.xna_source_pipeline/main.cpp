// SPDX-License-Identifier: MS-PL
// Audit the public registration and end-to-end .x/.fbx model source routes.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "CNA/Content/Pipeline/ContentPipeline.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbModelV2Codec.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "CNA/Content/Pipeline/TextureCompressionPipeline.hpp"
#include "CNA/Content/Pipeline/XnaModelBridge.hpp"
#include "CNA/Content/Pipeline/XnaModelSourceContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnaPipelineBridge.hpp"
#include "CNA/Content/Pipeline/XnbOutputContentPipeline.hpp"
#include "CNA/Internal/Xnb/XnbFileOptions.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Graphics/MaterialContent.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/ModelImporters.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/PipelineException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Processors/ModelProcessor.hpp"

namespace
{
    namespace Pipeline = CNA::Content::Pipeline;
    namespace Cnb = CNA::Content::Cnb;
    namespace XnaPipeline = Microsoft::Xna::Framework::Content::Pipeline;
    namespace XnaGraphics = Microsoft::Xna::Framework::Content::Pipeline::Graphics;
    namespace XnaProcessors = Microsoft::Xna::Framework::Content::Pipeline::Processors;

    constexpr std::string_view DirectXTriangle =
        "xof 0303txt 0032\n"
        "Mesh Loose {\n"
        "  3;\n"
        "  0.0; 0.0; 0.0;,\n"
        "  2.0; 0.0; 0.0;,\n"
        "  0.0; 2.0; 0.0;;\n"
        "  1;\n"
        "  3; 0, 1, 2;;\n"
        "}\n";

    constexpr std::string_view FbxTriangle = R"FBX(; FBX 6.1.0 project file
FBXHeaderExtension:  {
    FBXHeaderVersion: 1003
    FBXVersion: 6100
}
Definitions:  {
    Version: 100
    Count: 1
    ObjectType: "Model" { Count: 1 }
    ObjectType: "Material" { Count: 0 }
    ObjectType: "Deformer" { Count: 0 }
}
Objects:  {
    Model: "Model::Loose", "Mesh" {
        Version: 232
        Properties60:  {
            Property: "Lcl Translation", "Lcl Translation", "A+",0,0,0
            Property: "Lcl Rotation", "Lcl Rotation", "A+",0,0,0
            Property: "Lcl Scaling", "Lcl Scaling", "A+",1,1,1
        }
        MultiLayer: 0
        MultiTake: 1
        Shading: Y
        Culling: "CullingOff"
        Vertices: 0,0,0,2,0,0,0,2,0
        PolygonVertexIndex: 2,1,-1
        GeometryVersion: 124
        Layer: 0 { Version: 100 }
    }
}
Relations:  {
    Model: "Model::Loose", "Mesh" { }
}
Connections:  {
    Connect: "OO", "Model::Loose", "Model::Scene"
}
Takes:  { Current: "" }
Version5:  {
    AmbientRenderSettings:  {
        Version: 101
        AmbientLightColor: 0,0,0,0
    }
}
)FBX";

    class FixtureRoot
    {
    public:
        FixtureRoot()
            : path_(std::filesystem::temp_directory_path() /
                    ("cna-demos-xna-source-pipeline-" + std::to_string(
                        std::chrono::steady_clock::now().time_since_epoch().count())))
        {
            std::filesystem::remove_all(path_);
            std::filesystem::create_directories(path_ / "out");
            Write("triangle.x", DirectXTriangle);
            Write("triangle.fbx", FbxTriangle);
        }

        ~FixtureRoot()
        {
            std::error_code error;
            std::filesystem::remove_all(path_, error);
        }

        FixtureRoot(const FixtureRoot&) = delete;
        FixtureRoot& operator=(const FixtureRoot&) = delete;

        [[nodiscard]] const std::filesystem::path& Path() const noexcept { return path_; }

    private:
        void Write(const std::filesystem::path& name, const std::string_view text) const
        {
            std::ofstream output(path_ / name, std::ios::binary);
            output.write(text.data(), static_cast<std::streamsize>(text.size()));
            if (!output) throw std::runtime_error("Could not write a model source fixture.");
        }

        std::filesystem::path path_;
    };

    [[nodiscard]] const Pipeline::ContentProcessor* FindProcessor(
        const std::vector<std::shared_ptr<const Pipeline::ContentProcessor>>& processors,
        const std::string_view name)
    {
        const auto found = std::find_if(processors.begin(), processors.end(),
            [name](const auto& processor) { return processor->Identity().name == name; });
        return found == processors.end() ? nullptr : found->get();
    }

    [[nodiscard]] bool StartsWith(const std::vector<std::uint8_t>& bytes,
                                  const std::string_view prefix)
    {
        return bytes.size() >= prefix.size()
            && std::equal(prefix.begin(), prefix.end(), bytes.begin());
    }

    [[nodiscard]] Pipeline::ContentBuildResult Build(
        const Pipeline::ContentPipeline& pipeline, const FixtureRoot& source,
        const std::string& filename, const Pipeline::ContentOutputFormat format)
    {
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = source.Path();
        request.source = filename;
        request.logicalName = "Models/" + std::filesystem::path(filename).stem().string();
        request.outputFormat = format;
        request.environment.outputDirectory = source.Path() / "out";
        return pipeline.Build(request);
    }

    class RecordingLogger final : public Pipeline::ContentBuildLogger
    {
    public:
        void Log(const Pipeline::ContentLogMessage& message) override { messages.push_back(message); }
        std::vector<Pipeline::ContentLogMessage> messages;
    };

    int Run()
    {
        bool passed = true;
        const auto check = [&](const bool condition, const std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterTexture2DContentPipeline(
            *registry, Pipeline::MakeBlockCompressionTextureEncoder());
        Pipeline::RegisterModelContentPipeline(*registry);
        Pipeline::RegisterXnbOutputContentPipeline(
            *registry, CNA::Internal::Xnb::XnbFileOptions{});

        const std::size_t importersBefore = registry->Importers().size();
        const std::size_t processorsBefore = registry->Processors().size();
        const std::size_t absencesBefore = registry->AbsentWriters().size();
        Pipeline::RegisterXnaModelSourceContentPipeline(*registry);

        const auto importers = registry->Importers();
        const auto processors = registry->Processors();
        check(importers.size() == importersBefore + 2u
                  && processors.size() == processorsBefore + 3u
                  && registry->AbsentWriters().size() == absencesBefore + 2u,
              "one call registers two importers, three processors and both intentional material-writer absences");

        const auto xImporter = registry->ResolveImporter("model.X");
        const auto fbxImporter = registry->ResolveImporter("model.FBX");
        check(std::string_view(Pipeline::ImportedXnaNodeGraphType)
                      == "Microsoft.Xna.Framework.Content.Pipeline.Graphics.NodeContent"
                  && xImporter->Identity().name == "CNA.XImporter"
                  && xImporter->SourceExtensions() == std::vector<std::string>{".x"}
                  && xImporter->OutputTypes()
                      == std::vector<std::string>{Pipeline::ImportedXnaNodeGraphType}
                  && xImporter->DefaultProcessor() == "CNA.XnaModelProcessor"
                  && fbxImporter->Identity().name == "CNA.FbxImporter"
                  && fbxImporter->SourceExtensions() == std::vector<std::string>{".fbx"}
                  && fbxImporter->OutputTypes()
                      == std::vector<std::string>{Pipeline::ImportedXnaNodeGraphType}
                  && fbxImporter->DefaultProcessor() == "CNA.XnaModelProcessor",
              "case-insensitive .x and .fbx resolution exposes the declared node-graph type and shared default processor");

        const Pipeline::ContentProcessor* model = FindProcessor(processors, "CNA.XnaModelProcessor");
        const Pipeline::ContentProcessor* material = FindProcessor(processors, "MaterialProcessor");
        const Pipeline::ContentProcessor* texture = FindProcessor(processors, "TextureProcessor");
        check(model != nullptr && material != nullptr && texture != nullptr
                  && model->InputType() == Pipeline::ImportedXnaNodeGraphType
                  && model->OutputType() == Pipeline::ProcessedModelType
                  && !model->SelectedByNameOnly() && !material->SelectedByNameOnly()
                  && texture->SelectedByNameOnly(),
              "the model processor bridges to the canonical model type while nested material and texture processors remain available by XNA name");

        const std::string materialType =
            "Microsoft.Xna.Framework.Content.Pipeline.Graphics.MaterialContent";
        check(!registry->AbsentWriterReason(Pipeline::ContentOutputFormat::Cnb, materialType).empty()
                  && !registry->AbsentWriterReason(Pipeline::ContentOutputFormat::Xnb, materialType).empty(),
              "both output formats explain why an intermediate material intentionally has no standalone writer");

        const Pipeline::ContentPipeline pipeline(registry);
        check(registry->IsFrozen() && &pipeline.Registry() == registry.get(),
              "constructing the coordinator freezes and retains the configured registry");

        const FixtureRoot source;
        const Pipeline::ContentBuildResult xCnb =
            Build(pipeline, source, "triangle.x", Pipeline::ContentOutputFormat::Cnb);
        const Pipeline::ContentBuildResult fbxCnb =
            Build(pipeline, source, "triangle.fbx", Pipeline::ContentOutputFormat::Cnb);
        check(xCnb.importer.name == "CNA.XImporter"
                  && fbxCnb.importer.name == "CNA.FbxImporter"
                  && xCnb.processor.name == "CNA.XnaModelProcessor"
                  && fbxCnb.processor.name == "CNA.XnaModelProcessor"
                  && xCnb.writer.name == fbxCnb.writer.name
                  && xCnb.output.assetTypeName == "Microsoft.Xna.Framework.Graphics.Model"
                  && fbxCnb.output.assetTypeName == "Microsoft.Xna.Framework.Graphics.Model"
                  && StartsWith(xCnb.output.bytes, "CNB") && StartsWith(fbxCnb.output.bytes, "CNB")
                  && xCnb.output.additionalOutputs.empty()
                  && fbxCnb.output.additionalOutputs.empty(),
              "actual .x and .fbx sources select their own importer and produce standalone canonical Model CNB bytes");

        const Pipeline::ContentBuildResult xXnb =
            Build(pipeline, source, "triangle.x", Pipeline::ContentOutputFormat::Xnb);
        const Pipeline::ContentBuildResult fbxXnb =
            Build(pipeline, source, "triangle.fbx", Pipeline::ContentOutputFormat::Xnb);
        check(xXnb.importer.name == "CNA.XImporter"
                  && fbxXnb.importer.name == "CNA.FbxImporter"
                  && xXnb.processor.name == "CNA.XnaModelProcessor"
                  && fbxXnb.processor.name == "CNA.XnaModelProcessor"
                  && xXnb.writer.name == fbxXnb.writer.name
                  && xXnb.output.rootReaderName.find("ModelReader") != std::string::npos
                  && fbxXnb.output.rootReaderName.find("ModelReader") != std::string::npos
                  && StartsWith(xXnb.output.bytes, "XNB") && StartsWith(fbxXnb.output.bytes, "XNB")
                  && xXnb.output.additionalOutputs.empty()
                  && fbxXnb.output.additionalOutputs.empty(),
              "both source routes also retain their importer identities through the Model XNB writer");

        const auto xModel = Cnb::DecodeModelV2FromCnb(
            Cnb::CnbDocument::Parse(xCnb.output.bytes, "x route"));
        const auto fbxModel = Cnb::DecodeModelV2FromCnb(
            Cnb::CnbDocument::Parse(fbxCnb.output.bytes, "fbx route"));
        check(xCnb.output.assetSchemaVersion == Cnb::CnbModelV2SchemaVersion
                  && fbxCnb.output.assetSchemaVersion == Cnb::CnbModelV2SchemaVersion
                  && xModel.bones.size() == 2u && xModel.bones[0].name.empty()
                  && xModel.bones[0].parent == -1 && xModel.bones[1].name == "Loose"
                  && xModel.bones[1].parent == 0 && fbxModel.bones.size() == 1u
                  && fbxModel.bones[0].name == "Loose" && fbxModel.bones[0].parent == -1
                  && xModel.meshes.size() == 1u && fbxModel.meshes.size() == 1u
                  && xModel.meshes[0].name == "Loose" && fbxModel.meshes[0].name == "Loose"
                  && xModel.meshes[0].parentBone == 1 && fbxModel.meshes[0].parentBone == 0
                  && xModel.meshes[0].boundingSphere == fbxModel.meshes[0].boundingSphere
                  && xModel.meshes[0].parts == fbxModel.meshes[0].parts
                  && xModel.vertexDeclarations == fbxModel.vertexDeclarations
                  && xModel.vertexBuffers == fbxModel.vertexBuffers
                  && xModel.indexBuffers == fbxModel.indexBuffers
                  && xModel.effects == fbxModel.effects,
              "the routes preserve their format-specific root graphs while converging on exact geometry, bounds, draw ranges and material");

        Pipeline::ContentSourceRootCapabilities sourceRoots;
        Pipeline::ContentDependencyCollector directDependencies;
        RecordingLogger directLogger;
        Pipeline::ContentBuildEnvironment directEnvironment;
        directEnvironment.outputDirectory = source.Path() / "out";
        const auto directSource = source.Path() / "triangle.x";
        Pipeline::ContentImporterContext canonicalImporterContext(
            source.Path(), directSource, "Models/triangle", "CNA.XImporter", sourceRoots,
            directDependencies, directLogger, directEnvironment);
        Pipeline::XnaBridgeImporterContext importerContext(canonicalImporterContext);
        XnaPipeline::XImporter directImporter;
        const auto importedGraph = directImporter.Import(directSource.string(), importerContext);
        Pipeline::ContentProcessorParameters directParameters;
        Pipeline::ContentProcessorContext canonicalProcessorContext(
            source.Path(), directSource, "Models/triangle", "CNA.XnaModelProcessor", directParameters,
            sourceRoots, directDependencies, directLogger, Pipeline::ContentOutputFormat::Cnb,
            directEnvironment, &pipeline);
        Pipeline::XnaBridgeProcessorContext processorContext(canonicalProcessorContext);
        XnaProcessors::ModelProcessor directProcessor;
        const auto directModel = directProcessor.Process(importedGraph, processorContext);
        const auto canonicalModel = Pipeline::ToCanonicalModel(*directModel);
        auto directV2 = CNA::Internal::Xnb::ConvertXnbModelToCnbV2(
            canonicalModel, [](const std::string& authored) { return authored; });
        for (auto& bone : directV2.bones) bone.nameIsNull = false;
        check(directV2 == xModel,
              "direct XNA import and processing converts to the exact schema-2 graph emitted by the registered route");

        const auto& originalMesh = directModel->getMeshesProperty().front();
        const auto& originalPart = originalMesh->getMeshPartsProperty().front();
        auto duplicatePart = std::make_shared<XnaProcessors::ModelMeshPartContent>(
            originalPart->getVertexBufferProperty(), originalPart->getIndexBufferProperty(),
            originalPart->getVertexOffsetProperty(), originalPart->getNumVerticesProperty(),
            originalPart->getStartIndexProperty(), originalPart->getPrimitiveCountProperty());
        duplicatePart->setMaterialProperty(originalPart->getMaterialProperty());
        auto duplicateMesh = std::make_shared<XnaProcessors::ModelMeshContent>(
            originalMesh->getNameProperty(), originalMesh->getSourceMeshProperty(),
            originalMesh->getParentBoneProperty(), originalMesh->getBoundingSphereProperty(),
            XnaProcessors::ModelMeshPartContentCollection{originalPart, duplicatePart});
        const XnaProcessors::ModelContent duplicateModel(
            directModel->getRootProperty(), directModel->getBonesProperty(),
            XnaProcessors::ModelMeshContentCollection{duplicateMesh});
        const auto sharedCanonical = Pipeline::ToCanonicalModel(duplicateModel);
        check(sharedCanonical.sharedResources.size() == 3u
                  && sharedCanonical.meshes.size() == 1u
                  && sharedCanonical.meshes.front().parts.size() == 2u
                  && sharedCanonical.meshes.front().parts[0].vertexBufferResource
                      == sharedCanonical.meshes.front().parts[1].vertexBufferResource
                  && sharedCanonical.meshes.front().parts[0].indexBufferResource
                      == sharedCanonical.meshes.front().parts[1].indexBufferResource
                  && sharedCanonical.meshes.front().parts[0].effectResource
                      == sharedCanonical.meshes.front().parts[1].effectResource,
              "two parts sharing their buffers and material produce exactly three shared resources");

        auto missingVertexPart = std::make_shared<XnaProcessors::ModelMeshPartContent>(
            nullptr, originalPart->getIndexBufferProperty(), originalPart->getVertexOffsetProperty(),
            originalPart->getNumVerticesProperty(), originalPart->getStartIndexProperty(),
            originalPart->getPrimitiveCountProperty());
        missingVertexPart->setMaterialProperty(originalPart->getMaterialProperty());
        auto missingVertexMesh = std::make_shared<XnaProcessors::ModelMeshContent>(
            "MissingVertex", originalMesh->getSourceMeshProperty(), originalMesh->getParentBoneProperty(),
            originalMesh->getBoundingSphereProperty(),
            XnaProcessors::ModelMeshPartContentCollection{missingVertexPart});
        const XnaProcessors::ModelContent missingVertexModel(
            directModel->getRootProperty(), directModel->getBonesProperty(),
            XnaProcessors::ModelMeshContentCollection{missingVertexMesh});
        bool missingVertexRefused = false;
        try { (void)Pipeline::ToCanonicalModel(missingVertexModel); }
        catch (const XnaPipeline::PipelineException& error)
        {
            missingVertexRefused = std::string(error.what()).find("no vertex buffer") != std::string::npos;
        }
        check(missingVertexRefused,
              "a model part without a vertex buffer is refused with the documented pipeline exception");

        const auto stockMaterial = originalPart->getMaterialProperty();
        originalPart->setMaterialProperty(std::make_shared<XnaGraphics::MaterialContent>());
        bool unsupportedMaterialRefused = false;
        try { (void)Pipeline::ToCanonicalModel(*directModel); }
        catch (const XnaPipeline::PipelineException& error)
        {
            unsupportedMaterialRefused = std::string(error.what()).find("five stock materials") != std::string::npos;
        }
        originalPart->setMaterialProperty(stockMaterial);
        check(unsupportedMaterialRefused,
              "a material outside the five supported stock types is refused with its model context");

        std::cout << "Built equivalent DirectX and FBX sources through CNA's registered XNA model route.\n";
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
        std::cerr << "XNA model source-pipeline audit failed: " << error.what() << '\n';
        return 1;
    }
}
