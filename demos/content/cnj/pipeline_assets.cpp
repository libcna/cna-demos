// SPDX-License-Identifier: MS-PL
// Compile the three extended CNJ asset kinds that are not ordinary XNA content sources.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbAnimationClipCodec.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::Graphics::ClipTargetSpaceEXT;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    class TemporarySources
    {
    public:
        explicit TemporarySources(std::filesystem::path path) : path_(std::move(path)) {}
        ~TemporarySources()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path_, ignored);
        }

        TemporarySources(const TemporarySources&) = delete;
        TemporarySources& operator=(const TemporarySources&) = delete;

    private:
        std::filesystem::path path_;
    };

    void WriteBytes(const std::filesystem::path& path,
                    const std::vector<std::uint8_t>& bytes)
    {
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        if (!output) throw std::runtime_error("Could not write a temporary CNJ sidecar");
    }

    void WriteText(const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream output(path, std::ios::binary);
        output << text;
        if (!output) throw std::runtime_error("Could not write a temporary CNJ document");
    }

    bool HasSourceSidecar(const Pipeline::ContentBuildResult& result,
                          const std::string& filename)
    {
        for (const Pipeline::ContentDependency& dependency : result.dependencies)
        {
            if (dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                std::filesystem::path(dependency.identity).filename() == filename)
            {
                return true;
            }
        }
        return false;
    }

    void CnjPipelineAssets(CnaDemos::DemoContext& ctx)
    {
        const Pipeline::CnjImporter importer;
        const std::vector<std::string> expectedTypes{
            Pipeline::ImportedImageType,
            Pipeline::ImportedModelDocumentType,
            Pipeline::ImportedSoundType,
            Pipeline::ImportedSpriteFontType,
            Pipeline::ImportedTexture3DType,
            Pipeline::ImportedTextureCubeType,
            Pipeline::ImportedCurveType,
            Pipeline::ImportedAnimationClipType};
        ctx.Check(importer.Identity() == Pipeline::ContentComponentIdentity{"CNA.CnjImporter", "1"} &&
                      importer.SourceExtensions() == std::vector<std::string>{".cnj"} &&
                      importer.OutputTypes() == expectedTypes && !importer.SelectedByNameOnly(),
                  "the CNJ importer advertises its stable source route and all eight bounded output types");

        const Pipeline::Texture3DProcessor volumeProcessor;
        const Pipeline::TextureCubeProcessor cubeProcessor;
        const Pipeline::AnimationClipProcessor clipProcessor;
        ctx.Check(volumeProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.Texture3DProcessor", "1"} &&
                      volumeProcessor.InputType() == Pipeline::ImportedTexture3DType &&
                      volumeProcessor.OutputType() == Pipeline::ProcessedTexture3DType &&
                      cubeProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.TextureCubeProcessor", "1"} &&
                      cubeProcessor.InputType() == Pipeline::ImportedTextureCubeType &&
                      cubeProcessor.OutputType() == Pipeline::ProcessedTextureCubeType &&
                      clipProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.AnimationClipProcessor", "1"} &&
                      clipProcessor.InputType() == Pipeline::ImportedAnimationClipType &&
                      clipProcessor.OutputType() == Pipeline::ProcessedAnimationClipType,
                  "the three fixed processors connect each imported CNJ value to its compiled type");

        const Pipeline::Texture3DContentWriter volumeWriter;
        const Pipeline::TextureCubeContentWriter cubeWriter;
        const Pipeline::AnimationClipContentWriter clipWriter;
        const auto volumeSchemas = volumeWriter.OutputSchemaIdentities();
        const auto cubeSchemas = cubeWriter.OutputSchemaIdentities();
        const auto clipSchemas = clipWriter.OutputSchemaIdentities();
        ctx.Check(volumeWriter.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.Texture3DContentWriter", "1"} &&
                      volumeWriter.InputType() == Pipeline::ProcessedTexture3DType &&
                      volumeSchemas.size() == 1u &&
                      volumeSchemas[0].assetTypeId == Cnb::CnbAssetTypeId::Texture3D &&
                      cubeWriter.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.TextureCubeContentWriter", "1"} &&
                      cubeWriter.InputType() == Pipeline::ProcessedTextureCubeType &&
                      cubeSchemas.size() == 1u &&
                      cubeSchemas[0].assetTypeId == Cnb::CnbAssetTypeId::TextureCube &&
                      clipWriter.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.AnimationClipContentWriter", "1"} &&
                      clipWriter.InputType() == Pipeline::ProcessedAnimationClipType &&
                      clipSchemas.size() == 1u &&
                      clipSchemas[0].assetTypeId == Cnb::CnbAssetTypeId::AnimationClip,
                  "the three writers declare their exact native CNB asset schemas");

        Pipeline::ContentProcessorParameters invalidParameters;
        invalidParameters.Set("generateMipmaps", true);
        bool parameterRefused = false;
        try
        {
            volumeProcessor.ValidateParameters(invalidParameters);
        }
        catch (const Pipeline::ContentParameterError& error)
        {
            parameterRefused = error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "generateMipmaps";
        }
        ctx.Check(parameterRefused,
                  "a fixed CNJ processor classifies an unsupported setting as an unknown parameter");

        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterCnjContentPipeline(*registry);
        ctx.Check(registry->ResolveImporter("asset.cnj")->Identity() == importer.Identity() &&
                      registry->ResolveProcessor(Pipeline::ImportedTexture3DType)->Identity() ==
                          volumeProcessor.Identity() &&
                      registry->ResolveWriter(Pipeline::ProcessedTexture3DType)->Identity() ==
                          volumeWriter.Identity() &&
                      registry->ResolveProcessor(Pipeline::ImportedTextureCubeType)->Identity() ==
                          cubeProcessor.Identity() &&
                      registry->ResolveWriter(Pipeline::ProcessedTextureCubeType)->Identity() ==
                          cubeWriter.Identity() &&
                      registry->ResolveProcessor(Pipeline::ImportedAnimationClipType)->Identity() ==
                          clipProcessor.Identity() &&
                      registry->ResolveWriter(Pipeline::ProcessedAnimationClipType)->Identity() ==
                          clipWriter.Identity(),
                  "RegisterCnjContentPipeline wires all three extended source routes");

        const auto root = std::filesystem::temp_directory_path() /
            ("cna-demos-cnj-pipeline-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root))
            throw std::runtime_error("Private CNJ source directory already exists");
        TemporarySources cleanup(root);

        std::vector<std::uint8_t> volumePixels(2u * 2u * 2u * 4u);
        for (std::size_t index = 0u; index < volumePixels.size(); ++index)
            volumePixels[index] = static_cast<std::uint8_t>(index * 7u + 3u);
        WriteBytes(root / "volume.rgba", volumePixels);
        WriteText(root / "volume.cnj",
                  R"({"cnjVersion":1,"type":"Texture3D","width":2,"height":2,"depth":2,"data":"volume.rgba"})");
        std::filesystem::copy_file(
            std::filesystem::path(TitleLocation::Path()) / "assets/textures/cube_faces.dds",
            root / "sky.dds");
        WriteText(root / "sky.cnj",
                  R"({"cnjVersion":1,"type":"TextureCube","sourceFile":"sky.dds"})");
        WriteText(root / "walk.cnj",
                  R"({"cnjVersion":1,"type":"AnimationClip","duration":2,"targetSpace":"SceneNode","tracks":[{"boneIndex":2,"keys":[{"time":0,"translation":[1,2,3],"rotation":[0,0,0,1],"scale":[1,1,1]},{"time":1.5,"translation":[4,5,6]}]}]})");

        const Pipeline::ContentPipeline pipeline(registry);
        const auto build = [&](const std::string& source, const std::string& logicalName)
        {
            Pipeline::ContentBuildRequest request;
            request.sourceRoot = root;
            request.source = source;
            request.logicalName = logicalName;
            return pipeline.Build(request);
        };

        const Pipeline::ContentBuildResult volume = build("volume.cnj", "Volumes/checker");
        ctx.Check(volume.importer == importer.Identity() &&
                      volume.processor == volumeProcessor.Identity() &&
                      volume.writer == volumeWriter.Identity() &&
                      volume.output.assetTypeId == Cnb::CnbAssetTypeId::Texture3D &&
                      HasSourceSidecar(volume, "volume.rgba"),
                  "the Texture3D build selects the advertised components and absorbs its raw sidecar");
        const Cnb::CnbDocument volumeDocument =
            Cnb::CnbDocument::Parse(volume.output.bytes, "volume.cnb");
        const Cnb::CnbTextureData decodedVolume = Cnb::DecodeTexture3DFromCnb(volumeDocument);
        ctx.Check(volumeDocument.Metadata().contentName == "Volumes/checker" &&
                      decodedVolume.width == 2u && decodedVolume.height == 2u &&
                      decodedVolume.depth == 2u && decodedVolume.faceCount == 1u &&
                      decodedVolume.mipCount == 1u &&
                      decodedVolume.representations.size() == 1u &&
                      decodedVolume.representations[0].format == Cnb::CnbTextureFormat::Rgba8 &&
                      decodedVolume.representations[0].levels ==
                          std::vector<std::vector<std::uint8_t>>{volumePixels},
                  "the Texture3D CNB preserves the logical name, dimensions and every authored voxel byte");
        ctx.Check(build("volume.cnj", "Volumes/checker").output.bytes == volume.output.bytes,
                  "repeating the Texture3D route produces byte-identical native content");

        const Pipeline::ContentBuildResult cube = build("sky.cnj", "Skies/six_faces");
        const Cnb::CnbTextureData decodedCube = Cnb::DecodeTextureCubeFromCnb(
            Cnb::CnbDocument::Parse(cube.output.bytes, "sky.cnb"));
        const std::array<std::array<std::uint8_t, 4>, 6> faceColors{{
            {{255u, 0u, 0u, 255u}}, {{0u, 255u, 0u, 255u}},
            {{0u, 0u, 255u, 255u}}, {{255u, 255u, 0u, 255u}},
            {{255u, 0u, 255u, 255u}}, {{0u, 255u, 255u, 255u}}}};
        bool exactFaces = decodedCube.representations.size() == 1u &&
            decodedCube.representations[0].format == Cnb::CnbTextureFormat::Rgba8 &&
            decodedCube.representations[0].levels.size() == faceColors.size();
        if (exactFaces)
        {
            for (std::size_t face = 0u; face < faceColors.size(); ++face)
            {
                const auto& pixels = decodedCube.representations[0].levels[face];
                if (pixels.size() != 4u * 4u * 4u)
                {
                    exactFaces = false;
                    break;
                }
                for (std::size_t offset = 0u; offset + 3u < pixels.size(); offset += 4u)
                {
                    exactFaces = exactFaces &&
                        std::equal(faceColors[face].begin(), faceColors[face].end(),
                                   pixels.begin() + static_cast<std::ptrdiff_t>(offset));
                }
            }
        }
        ctx.Check(cube.importer == importer.Identity() &&
                      cube.processor == cubeProcessor.Identity() &&
                      cube.writer == cubeWriter.Identity() &&
                      cube.output.assetTypeId == Cnb::CnbAssetTypeId::TextureCube &&
                      HasSourceSidecar(cube, "sky.dds"),
                  "the TextureCube build selects the advertised components and absorbs its DDS sidecar");
        ctx.Check(decodedCube.width == 4u && decodedCube.height == 4u &&
                      decodedCube.depth == 1u && decodedCube.faceCount == 6u &&
                      decodedCube.mipCount == 1u && exactFaces,
                  "the TextureCube CNB preserves all six distinct DDS faces in canonical order");

        const Pipeline::ContentBuildResult clip = build("walk.cnj", "Animations/walk");
        const auto decodedClip = Cnb::DecodeAnimationClipFromCnb(
            Cnb::CnbDocument::Parse(clip.output.bytes, "walk.cnb"));
        ctx.Check(clip.importer == importer.Identity() &&
                      clip.processor == clipProcessor.Identity() &&
                      clip.writer == clipWriter.Identity() &&
                      clip.output.assetTypeId == Cnb::CnbAssetTypeId::AnimationClip &&
                      clip.dependencies.size() == 1u,
                  "the self-contained AnimationClip build selects the advertised native route without a sidecar");
        ctx.Check(decodedClip.Duration.getTicksProperty() == 20000000 &&
                      decodedClip.TargetSpace == ClipTargetSpaceEXT::SceneNode &&
                      decodedClip.Tracks.size() == 1u && decodedClip.Tracks[0].BoneIndex == 2 &&
                      decodedClip.Tracks[0].Keys.size() == 2u &&
                      decodedClip.Tracks[0].Keys[0].Translation.X == 1.0f &&
                      decodedClip.Tracks[0].Keys[0].Translation.Y == 2.0f &&
                      decodedClip.Tracks[0].Keys[0].Translation.Z == 3.0f &&
                      decodedClip.Tracks[0].Keys[1].Time.getTicksProperty() == 15000000 &&
                      decodedClip.Tracks[0].Keys[1].Translation.X == 4.0f &&
                      decodedClip.Tracks[0].Keys[1].Translation.Y == 5.0f &&
                      decodedClip.Tracks[0].Keys[1].Translation.Z == 6.0f,
                  "the AnimationClip CNB preserves duration, scene-node target and both authored keyframes");

        Pipeline::ContentBuildRequest xnbRequest;
        xnbRequest.sourceRoot = root;
        xnbRequest.source = "walk.cnj";
        xnbRequest.logicalName = "Animations/walk";
        xnbRequest.outputFormat = Pipeline::ContentOutputFormat::Xnb;
        bool xnbAbsenceExplained = false;
        try
        {
            (void)pipeline.Build(xnbRequest);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            const std::string message = error.what();
            xnbAbsenceExplained = error.Stage() == Pipeline::ContentPipelineStage::Selection &&
                message.find("no AnimationClip content type") != std::string::npos &&
                message.find("CNB-only by design") != std::string::npos;
        }
        ctx.Check(xnbAbsenceExplained,
                  "AnimationClip's intentional XNB absence explains why the CNA extension is CNB-only");

        ctx.Print("Compiled Texture3D {}x{}x{}, TextureCube {} faces, AnimationClip {} keys.",
                  decodedVolume.width, decodedVolume.height, decodedVolume.depth,
                  decodedCube.faceCount, decodedClip.Tracks[0].Keys.size());
    }
}

CNA_DEMOS_CONSOLE(CnjPipelineAssets, {
    .id = "content.cnj.pipeline_assets",
    .title = "Extended assets through the CNJ pipeline",
    .description = "Builds raw-volume Texture3D, DDS TextureCube and inline AnimationClip CNJ sources through CNA's registered importer, processors and writers; decodes their native CNBs and verifies dependencies, exact values, determinism, parameter refusal and the clip's intentional CNB-only route.",
    .apis = {"CNA::Content::Pipeline::ImportedTexture3DType",
             "CNA::Content::Pipeline::ProcessedTexture3DType",
             "CNA::Content::Pipeline::ImportedTextureCubeType",
             "CNA::Content::Pipeline::ProcessedTextureCubeType",
             "CNA::Content::Pipeline::ImportedAnimationClipType",
             "CNA::Content::Pipeline::ProcessedAnimationClipType",
             "CNA::Content::Pipeline::ImportedTexture3D",
             "CNA::Content::Pipeline::ImportedTextureCube",
             "CNA::Content::Pipeline::ImportedAnimationClip",
             "CNA::Content::Pipeline::ProcessedAnimationClip",
             "CNA::Content::Pipeline::CnjImporter::Identity",
             "CNA::Content::Pipeline::CnjImporter::SourceExtensions",
             "CNA::Content::Pipeline::CnjImporter::OutputTypes",
             "CNA::Content::Pipeline::CnjImporter::Import",
             "CNA::Content::Pipeline::Texture3DProcessor::Identity",
             "CNA::Content::Pipeline::Texture3DProcessor::InputType",
             "CNA::Content::Pipeline::Texture3DProcessor::OutputType",
             "CNA::Content::Pipeline::Texture3DProcessor::ValidateParameters",
             "CNA::Content::Pipeline::Texture3DProcessor::Process",
             "CNA::Content::Pipeline::Texture3DContentWriter::Identity",
             "CNA::Content::Pipeline::Texture3DContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::Texture3DContentWriter::InputType",
             "CNA::Content::Pipeline::Texture3DContentWriter::Write",
             "CNA::Content::Pipeline::TextureCubeProcessor::Identity",
             "CNA::Content::Pipeline::TextureCubeProcessor::InputType",
             "CNA::Content::Pipeline::TextureCubeProcessor::OutputType",
             "CNA::Content::Pipeline::TextureCubeProcessor::ValidateParameters",
             "CNA::Content::Pipeline::TextureCubeProcessor::Process",
             "CNA::Content::Pipeline::TextureCubeContentWriter::Identity",
             "CNA::Content::Pipeline::TextureCubeContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::TextureCubeContentWriter::InputType",
             "CNA::Content::Pipeline::TextureCubeContentWriter::Write",
             "CNA::Content::Pipeline::AnimationClipProcessor::Identity",
             "CNA::Content::Pipeline::AnimationClipProcessor::InputType",
             "CNA::Content::Pipeline::AnimationClipProcessor::OutputType",
             "CNA::Content::Pipeline::AnimationClipProcessor::ValidateParameters",
             "CNA::Content::Pipeline::AnimationClipProcessor::Process",
             "CNA::Content::Pipeline::AnimationClipContentWriter::Identity",
             "CNA::Content::Pipeline::AnimationClipContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::AnimationClipContentWriter::InputType",
             "CNA::Content::Pipeline::AnimationClipContentWriter::Write",
             "CNA::Content::Pipeline::RegisterCnjContentPipeline",
             "CNA::Content::Pipeline::ContentPipeline::Build",
             "CNA::Content::Cnb::DecodeTexture3DFromCnb",
             "CNA::Content::Cnb::DecodeTextureCubeFromCnb",
             "CNA::Content::Cnb::DecodeAnimationClipFromCnb"},
    .tags = {"content", "pipeline", "cnj", "texture3d", "texturecube", "animation", "cnaext"},
    .assets = {"textures/cube_faces.dds"},
    .restrictions = "Headless CPU compilation and exact CNB decoding; no GPU upload, rendering, screenshot or visual-quality claim.",
})
