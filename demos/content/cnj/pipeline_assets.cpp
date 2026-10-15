// SPDX-License-Identifier: MS-PL
// Compile the three extended CNJ asset kinds that are not ordinary XNA content sources.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbAnimationClipCodec.hpp"
#include "CNA/Content/Cnb/CnbCurveCodec.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbModelCodec.hpp"
#include "CNA/Content/Cnb/CnbSoundEffectCodec.hpp"
#include "CNA/Content/Cnb/CnbSpriteFontCodec.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Cnb/CnjToCnb.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::Graphics::ClipTargetSpaceEXT;
using Microsoft::Xna::Framework::Graphics::AnimationClipEXT;
using Microsoft::Xna::Framework::Graphics::BoneTrackEXT;
using Microsoft::Xna::Framework::Graphics::KeyframeEXT;
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

    bool SameKey(const KeyframeEXT& left, const KeyframeEXT& right)
    {
        return left.Time == right.Time && left.Translation == right.Translation &&
            left.Rotation == right.Rotation && left.Scale == right.Scale;
    }

    bool SameClip(const AnimationClipEXT& left, const AnimationClipEXT& right)
    {
        if (left.Duration != right.Duration || left.TargetSpace != right.TargetSpace ||
            left.Tracks.size() != right.Tracks.size())
        {
            return false;
        }
        for (std::size_t track = 0u; track < left.Tracks.size(); ++track)
        {
            if (left.Tracks[track].BoneIndex != right.Tracks[track].BoneIndex ||
                left.Tracks[track].Keys.size() != right.Tracks[track].Keys.size())
            {
                return false;
            }
            for (std::size_t key = 0u; key < left.Tracks[track].Keys.size(); ++key)
            {
                if (!SameKey(left.Tracks[track].Keys[key], right.Tracks[track].Keys[key]))
                    return false;
            }
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

    std::vector<std::uint8_t> ClipHeader(const double duration,
                                         const std::uint32_t targetSpace,
                                         const std::uint32_t trackCount,
                                         const std::uint32_t keyCount)
    {
        Cnb::CnbByteWriter bytes;
        bytes.WriteF64(duration);
        bytes.WriteU32(targetSpace);
        bytes.WriteU32(trackCount);
        bytes.WriteU32(keyCount);
        return bytes.Take();
    }

    std::vector<std::uint8_t> ClipTrack(const std::int32_t boneIndex,
                                        const std::uint32_t firstKey,
                                        const std::uint32_t keyCount)
    {
        Cnb::CnbByteWriter bytes;
        bytes.WriteI32(boneIndex);
        bytes.WriteU32(firstKey);
        bytes.WriteU32(keyCount);
        return bytes.Take();
    }

    std::vector<std::uint8_t> ClipKey(const KeyframeEXT& key)
    {
        Cnb::CnbByteWriter bytes;
        Cnb::WriteCnbKeyframe(bytes, key);
        return bytes.Take();
    }

    Cnb::CnbDocument ClipDocument(const std::uint32_t assetType,
                                  const std::uint32_t schema,
                                  const std::vector<std::uint8_t>& header,
                                  const std::vector<std::uint8_t>& tracks,
                                  const std::vector<std::uint8_t>& keys,
                                  const int headerCopies = 1,
                                  const int trackCopies = 1,
                                  const int keyCopies = 1)
    {
        Cnb::CnbWriter writer(assetType, schema);
        writer.SetMetadata("Microsoft.Xna.Framework.Graphics.AnimationClipEXT",
                           "synthetic/clip");
        for (int copy = 0; copy < headerCopies; ++copy)
            writer.AddChunk(Cnb::CnbAnimationClipChunk::Header, header,
                            Cnb::CnbChunkFlags::Mandatory, 8u);
        for (int copy = 0; copy < trackCopies; ++copy)
            writer.AddChunk(Cnb::CnbAnimationClipChunk::Tracks, tracks,
                            Cnb::CnbChunkFlags::Mandatory, 4u);
        for (int copy = 0; copy < keyCopies; ++copy)
            writer.AddChunk(Cnb::CnbAnimationClipChunk::Keys, keys,
                            Cnb::CnbChunkFlags::Mandatory, 8u);
        return Cnb::CnbDocument::Parse(writer.Build(), "synthetic AnimationClip CNB");
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

        WriteText(root / "bounce.cnj",
                  R"({"cnjVersion":1,"type":"Curve","preLoop":"Constant","postLoop":"Cycle","keys":[{"position":0,"value":0,"tangentIn":0,"tangentOut":2},{"position":0.5,"value":1,"tangentIn":0,"tangentOut":0},{"position":1,"value":0,"tangentIn":-2,"tangentOut":0}]})");
        WriteBytes(root / "triangle.vertices.bin",
                   {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u});
        WriteBytes(root / "triangle.indices.bin", {0u, 0u, 1u, 0u, 2u, 0u});
        WriteText(root / "triangle.cnj",
                  R"({"cnjVersion":1,"type":"Model","meshes":[{"name":"Triangle","vertices":"triangle.vertices.bin","indices":"triangle.indices.bin","vertexStride":4,"texture":"textures/shared.png"}]})");
        std::filesystem::copy_file(
            std::filesystem::path(TitleLocation::Path()) / "assets/textures/checker.png",
            root / "checker.png");
        WriteText(root / "checker.cnj",
                  R"({"cnjVersion":1,"type":"Texture2D","sourceFile":"checker.png"})");
        std::filesystem::copy_file(root / "checker.png", root / "tiny-atlas.png");
        WriteText(root / "tiny-font.cnj",
                  R"({"cnjVersion":1,"type":"SpriteFont","texture":"tiny-atlas.png","lineSpacing":12,"spacing":1.5,"defaultCharacter":"A","glyphs":[{"char":65,"source":[0,0,8,8],"crop":[1,2,8,8],"kerning":[1,8,2]},{"char":66,"source":[8,0,8,8],"crop":[0,1,8,8],"kerning":[0,8,1]}]})");
        std::filesystem::copy_file(
            std::filesystem::path(TitleLocation::Path()) / "assets/sounds/tone_440.wav",
            root / "tone.wav");
        WriteText(root / "tone.cnj",
                  R"({"cnjVersion":1,"type":"SoundEffect","sourceFile":"tone.wav"})");

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

        const auto compile = [&](const char* source, const char* logicalName)
        {
            return Cnb::CompileCnjToCnb((root / source).string(), root.string(), logicalName);
        };
        const Cnb::CnjToCnbResult compiledCurve = compile("bounce.cnj", "Curves/bounce");
        const Cnb::CnjToCnbResult compiledClip = compile("walk.cnj", "Animations/walk");
        const Cnb::CnjToCnbResult compiledModel = compile("triangle.cnj", "Models/triangle");
        const Cnb::CnjToCnbResult compiledImage = compile("checker.cnj", "Textures/checker");
        const Cnb::CnjToCnbResult compiledVolume = compile("volume.cnj", "Volumes/checker");
        const Cnb::CnjToCnbResult compiledCube = compile("sky.cnj", "Skies/six_faces");
        const Cnb::CnjToCnbResult compiledFont = compile("tiny-font.cnj", "Fonts/tiny");
        const Cnb::CnjToCnbResult compiledSound = compile("tone.cnj", "Sounds/tone");

        struct CompiledExpectation
        {
            const Cnb::CnjToCnbResult* result;
            std::uint32_t assetTypeId;
            const char* assetTypeName;
            const char* contentName;
        };
        const std::array<CompiledExpectation, 8> compiled{{
            {&compiledCurve, Cnb::CnbAssetTypeId::Curve,
             "Microsoft.Xna.Framework.Curve", "Curves/bounce"},
            {&compiledClip, Cnb::CnbAssetTypeId::AnimationClip,
             "Microsoft.Xna.Framework.Graphics.AnimationClipEXT", "Animations/walk"},
            {&compiledModel, Cnb::CnbAssetTypeId::Model,
             "Microsoft.Xna.Framework.Graphics.Model", "Models/triangle"},
            {&compiledImage, Cnb::CnbAssetTypeId::Texture2D,
             "Microsoft.Xna.Framework.Graphics.Texture2D", "Textures/checker"},
            {&compiledVolume, Cnb::CnbAssetTypeId::Texture3D,
             "Microsoft.Xna.Framework.Graphics.Texture3D", "Volumes/checker"},
            {&compiledCube, Cnb::CnbAssetTypeId::TextureCube,
             "Microsoft.Xna.Framework.Graphics.TextureCube", "Skies/six_faces"},
            {&compiledFont, Cnb::CnbAssetTypeId::SpriteFont,
             "Microsoft.Xna.Framework.Graphics.SpriteFont", "Fonts/tiny"},
            {&compiledSound, Cnb::CnbAssetTypeId::SoundEffect,
             "Microsoft.Xna.Framework.Audio.SoundEffect", "Sounds/tone"},
        }};
        bool exactCompilerIdentities = true;
        for (const CompiledExpectation& expected : compiled)
        {
            const Cnb::CnbDocument document =
                Cnb::CnbDocument::Parse(expected.result->bytes, expected.contentName);
            exactCompilerIdentities = exactCompilerIdentities &&
                !expected.result->bytes.empty() &&
                expected.result->assetTypeId == expected.assetTypeId &&
                expected.result->assetTypeName == expected.assetTypeName &&
                document.AssetTypeId() == expected.assetTypeId && document.Metadata().present &&
                document.Metadata().assetTypeName == expected.assetTypeName &&
                document.Metadata().contentName == expected.contentName;
        }
        ctx.Check(exactCompilerIdentities,
                  "the direct compiler reports and writes exact identities and logical names for all eight CNJ types");

        const auto directCurve = Cnb::DecodeCurveFromCnb(
            Cnb::CnbDocument::Parse(compiledCurve.bytes, "direct Curve CNB"));
        const auto directModel = Cnb::DecodeModelFromCnb(
            Cnb::CnbDocument::Parse(compiledModel.bytes, "direct Model CNB"));
        const auto directImage = Cnb::DecodeTexture2DFromCnb(
            Cnb::CnbDocument::Parse(compiledImage.bytes, "direct Texture2D CNB"));
        const auto directFont = Cnb::DecodeSpriteFontFromCnb(
            Cnb::CnbDocument::Parse(compiledFont.bytes, "direct SpriteFont CNB"));
        const auto directSound = Cnb::DecodeSoundEffectFromCnb(
            Cnb::CnbDocument::Parse(compiledSound.bytes, "direct SoundEffect CNB"));
        ctx.Check(directCurve.getKeysProperty().getCountProperty() == 3 &&
                      directCurve.getPreLoopProperty() == Microsoft::Xna::Framework::CurveLoopType::Constant &&
                      directCurve.getPostLoopProperty() == Microsoft::Xna::Framework::CurveLoopType::Cycle &&
                      directModel.parts.size() == 1u && directModel.meshes.size() == 1u &&
                      directModel.parts[0].vertexCount == 3u && directModel.parts[0].indexCount == 3u &&
                      directImage.width == 64u && directImage.height == 64u &&
                      directImage.faceCount == 1u && directImage.representations.size() == 1u &&
                      directFont.characters == std::vector<SharpRuntime::charcs>{u'A', u'B'} &&
                      directFont.lineSpacing == 12 && directFont.spacing == 1.5f &&
                      directFont.defaultCharacter == u'A' && directFont.atlas.width == 64u &&
                      directSound.format == Cnb::CnbAudioFormat::Pcm16 &&
                      directSound.sampleRate == 22050u && directSound.channels == 1u &&
                      directSound.frameCount == 11025u &&
                      directSound.samples.size() == directSound.frameCount * 2u,
                  "the five additional compiler routes decode to their exact authored CPU values");

        ctx.Check(compiledClip.bytes == clip.output.bytes &&
                      compiledVolume.bytes == volume.output.bytes &&
                      compiledCube.bytes == cube.output.bytes,
                  "the direct compiler is byte-identical to the registered pipeline for all three extended routes");

        ctx.Check(compiledCurve.absorbedFiles == std::vector<std::string>{"bounce.cnj"} &&
                      compiledClip.absorbedFiles == std::vector<std::string>{"walk.cnj"} &&
                      compiledModel.absorbedFiles ==
                          std::vector<std::string>{"triangle.cnj", "triangle.vertices.bin",
                                                   "triangle.indices.bin"} &&
                      compiledModel.externalReferences ==
                          std::vector<std::string>{"textures/shared.png"} &&
                      compiledImage.absorbedFiles ==
                          std::vector<std::string>{"checker.cnj", "checker.png"} &&
                      compiledVolume.absorbedFiles ==
                          std::vector<std::string>{"volume.cnj", "volume.rgba"} &&
                      compiledCube.absorbedFiles ==
                          std::vector<std::string>{"sky.cnj", "sky.dds"} &&
                      compiledFont.absorbedFiles ==
                          std::vector<std::string>{"tiny-font.cnj", "tiny-atlas.png"} &&
                      compiledSound.absorbedFiles ==
                          std::vector<std::string>{"tone.cnj", "tone.wav"} &&
                      compiledCurve.externalReferences.empty() &&
                      compiledClip.externalReferences.empty() &&
                      compiledImage.externalReferences.empty() &&
                      compiledVolume.externalReferences.empty() &&
                      compiledCube.externalReferences.empty() &&
                      compiledFont.externalReferences.empty() &&
                      compiledSound.externalReferences.empty(),
                  "the result separates every absorbed source/sidecar from the Model's surviving external reference");

        const Cnb::CnjToCnbResult defaulted =
            Cnb::CompileCnjToCnb((root / "bounce.cnj").string());
        const Cnb::CnjToCnbResult defaultedAgain =
            Cnb::CompileCnjToCnb((root / "bounce.cnj").string());
        ctx.Check(defaulted.bytes == defaultedAgain.bytes &&
                      Cnb::CnbDocument::Parse(defaulted.bytes, "defaulted Curve CNB")
                              .Metadata().contentName == "bounce" &&
                      defaulted.absorbedFiles == std::vector<std::string>{"bounce.cnj"},
                  "default root/name resolution is deterministic and uses the source parent and stem");

        WriteText(root / "unsupported.cnj", R"({"cnjVersion":1,"type":"Effect"})");
        WriteText(root / "future-image.cnj",
                  R"({"cnjVersion":2,"type":"Texture2D","sourceFile":"checker.png"})");
        WriteText(root / "missing-image.cnj",
                  R"({"cnjVersion":1,"type":"Texture2D","sourceFile":"missing.png"})");
        WriteText(root / "escaping-image.cnj",
                  R"({"cnjVersion":1,"type":"Texture2D","sourceFile":"../checker.png"})");
        ctx.Check(Refuses([&] { (void)compile("unsupported.cnj", "Bad/type"); }) &&
                      Refuses([&] { (void)compile("future-image.cnj", "Bad/version"); }) &&
                      Refuses([&] { (void)compile("missing-image.cnj", "Bad/missing"); }) &&
                      Refuses([&] { (void)compile("escaping-image.cnj", "Bad/escape"); }),
                  "unsupported type/version, missing sidecar and content-root escape are refused independently");

        ctx.Check(Cnb::CnbAnimationClipChunk::Header ==
                          Cnb::MakeChunkId('A', 'C', 'L', 'H') &&
                      Cnb::CnbAnimationClipChunk::Tracks ==
                          Cnb::MakeChunkId('A', 'C', 'L', 'T') &&
                      Cnb::CnbAnimationClipChunk::Keys ==
                          Cnb::MakeChunkId('A', 'C', 'L', 'K') &&
                      Cnb::CnbAnimationClipSchemaVersion == 1u &&
                      Cnb::CnbAnimationTrackStride == 12u &&
                      Cnb::CnbAnimationKeyStride == 48u,
                  "the AnimationClip schema freezes all chunk IDs, version and row strides");

        const auto encodedClip = Cnb::EncodeAnimationClipToCnb(decodedClip, "Animations/roundtrip");
        const auto repeatedClip = Cnb::EncodeAnimationClipToCnb(decodedClip, "Animations/roundtrip");
        const Cnb::CnbDocument encodedDocument =
            Cnb::CnbDocument::Parse(encodedClip, "round-trip AnimationClip CNB");
        const AnimationClipEXT restoredClip = Cnb::DecodeAnimationClipFromCnb(encodedDocument);
        ctx.Check(encodedClip == repeatedClip && SameClip(restoredClip, decodedClip) &&
                      encodedDocument.Metadata().assetTypeName ==
                          "Microsoft.Xna.Framework.Graphics.AnimationClipEXT" &&
                      encodedDocument.Metadata().contentName == "Animations/roundtrip" &&
                      encodedDocument.FindAll(Cnb::CnbAnimationClipChunk::Header).size() == 1u &&
                      encodedDocument.FindAll(Cnb::CnbAnimationClipChunk::Tracks).size() == 1u &&
                      encodedDocument.FindAll(Cnb::CnbAnimationClipChunk::Keys).size() == 1u,
                  "the full codec is byte-deterministic and restores the exact real clip and metadata");

        const KeyframeEXT directKey{
            System::TimeSpan::FromSeconds(-1.25),
            {-2.0f, 3.0f, -4.0f},
            {0.1f, -0.2f, 0.3f, -0.4f},
            {1.5f, 2.5f, 3.5f}};
        const std::vector<std::uint8_t> directKeyBytes = ClipKey(directKey);
        Cnb::CnbByteReader directKeyReader(directKeyBytes, "direct keyframe");
        const KeyframeEXT restoredKey = Cnb::ReadCnbKeyframe(directKeyReader);
        ctx.Check(directKeyBytes.size() == Cnb::CnbAnimationKeyStride &&
                      directKeyReader.Remaining() == 0u && SameKey(restoredKey, directKey),
                  "the shared keyframe helpers preserve time, translation, rotation and scale in 48 bytes");

        Cnb::CnbByteWriter secondsWriter;
        secondsWriter.WriteF64(1.25);
        const std::vector<std::uint8_t> secondsBytes = secondsWriter.Take();
        Cnb::CnbByteReader secondsReader(secondsBytes, "valid seconds");
        const double seconds = Cnb::ReadCnbSeconds(secondsReader, "the sample time");
        const auto refusesSeconds = [](const std::vector<std::uint8_t>& bytes)
        {
            return Refuses([&]
            {
                Cnb::CnbByteReader reader(bytes, "invalid seconds");
                (void)Cnb::ReadCnbSeconds(reader, "the sample time");
            });
        };
        Cnb::CnbByteWriter nanWriter;
        nanWriter.WriteF64(std::numeric_limits<double>::quiet_NaN());
        Cnb::CnbByteWriter infiniteWriter;
        infiniteWriter.WriteF64(std::numeric_limits<double>::infinity());
        ctx.Check(seconds == 1.25 && secondsReader.Remaining() == 0u &&
                      refusesSeconds(nanWriter.Take()) && refusesSeconds(infiniteWriter.Take()) &&
                      refusesSeconds(std::vector<std::uint8_t>{0u, 1u, 2u}),
                  "bounded second decoding accepts an exact finite value and refuses NaN, infinity and truncation");

        AnimationClipEXT paletteClip;
        paletteClip.Duration = System::TimeSpan::FromSeconds(3.0);
        paletteClip.TargetSpace = ClipTargetSpaceEXT::JointPalette;
        BoneTrackEXT populated;
        populated.BoneIndex = 4;
        populated.Keys = {directKey,
                          {System::TimeSpan::FromSeconds(2.75), {8.0f, 9.0f, 10.0f}}};
        BoneTrackEXT emptyTrack;
        emptyTrack.BoneIndex = -1;
        paletteClip.Tracks = {populated, emptyTrack};
        const AnimationClipEXT restoredPalette = Cnb::DecodeAnimationClipFromCnb(
            Cnb::CnbDocument::Parse(Cnb::EncodeAnimationClipToCnb(paletteClip),
                                    "palette AnimationClip CNB"));
        const AnimationClipEXT emptyClip;
        const AnimationClipEXT restoredEmpty = Cnb::DecodeAnimationClipFromCnb(
            Cnb::CnbDocument::Parse(Cnb::EncodeAnimationClipToCnb(emptyClip),
                                    "empty AnimationClip CNB"));
        ctx.Check(SameClip(restoredPalette, paletteClip) && SameClip(restoredEmpty, emptyClip) &&
                      restoredPalette.TargetSpace == ClipTargetSpaceEXT::JointPalette &&
                      restoredPalette.Tracks[1].Keys.empty() && restoredEmpty.Tracks.empty(),
                  "joint-palette, negative/empty-track and wholly empty clip states remain distinct");

        const auto emptyHeader = ClipHeader(1.0, 0u, 0u, 0u);
        const std::vector<std::uint8_t> emptyTable;
        ctx.Check(
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::Curve, 1u, emptyHeader, emptyTable, emptyTable));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 2u, emptyHeader,
                    emptyTable, emptyTable));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u, emptyHeader,
                    emptyTable, emptyTable, 0, 1, 1));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u, emptyHeader,
                    emptyTable, emptyTable, 2, 1, 1));
            }),
            "wrong identity/version and missing or duplicate mandatory clip tables are refused");

        Cnb::CnbByteWriter nanKey;
        nanKey.WriteF64(std::numeric_limits<double>::quiet_NaN());
        nanKey.WriteZeros(Cnb::CnbAnimationKeyStride - sizeof(double));
        ctx.Check(
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u,
                    ClipHeader(1.0, 2u, 0u, 0u), emptyTable, emptyTable));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u,
                    ClipHeader(1.0, 0u, 1u, 0u), emptyTable, emptyTable));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u,
                    ClipHeader(1.0, 0u, 1u, 1u), ClipTrack(0, 1u, 1u),
                    ClipKey(directKey)));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u,
                    ClipHeader(std::numeric_limits<double>::quiet_NaN(), 0u, 0u, 0u),
                    emptyTable, emptyTable));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeAnimationClipFromCnb(ClipDocument(
                    Cnb::CnbAssetTypeId::AnimationClip, 1u,
                    ClipHeader(1.0, 0u, 1u, 1u), ClipTrack(0, 0u, 1u), nanKey.Take()));
            }),
            "invalid target/count/range and non-finite duration or key time are refused independently");

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

        ctx.Print("Compiled all eight CNJ types: Texture3D {}x{}x{}, TextureCube {} faces, AnimationClip {} keys.",
                  decodedVolume.width, decodedVolume.height, decodedVolume.depth,
                  decodedCube.faceCount, decodedClip.Tracks[0].Keys.size());
    }
}

CNA_DEMOS_CONSOLE(CnjPipelineAssets, {
    .id = "content.cnj.pipeline_assets",
    .title = "Extended assets through the CNJ pipeline",
    .description = "Builds all eight supported CNJ asset types through the direct compiler, compares the three extended routes with CNA's registered pipeline, decodes exact CPU values and verifies identity, absorbed/external files, defaults, determinism and refusals.",
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
             "CNA::Content::Cnb::CnbAnimationClipChunk::Header",
             "CNA::Content::Cnb::CnbAnimationClipChunk::Tracks",
             "CNA::Content::Cnb::CnbAnimationClipChunk::Keys",
             "CNA::Content::Cnb::CnbAnimationClipSchemaVersion",
             "CNA::Content::Cnb::CnbAnimationTrackStride",
             "CNA::Content::Cnb::CnbAnimationKeyStride",
             "CNA::Content::Cnb::ReadCnbSeconds",
             "CNA::Content::Cnb::WriteCnbKeyframe",
             "CNA::Content::Cnb::ReadCnbKeyframe",
             "CNA::Content::Cnb::EncodeAnimationClipToCnb",
             "CNA::Content::Cnb::DecodeAnimationClipFromCnb",
             "CNA::Content::Cnb::CnjToCnbResult",
             "CNA::Content::Cnb::CompileCnjToCnb"},
    .tags = {"content", "pipeline", "cnj", "texture3d", "texturecube", "animation", "cnaext"},
    .assets = {"textures/cube_faces.dds", "textures/checker.png", "sounds/tone_440.wav"},
    .restrictions = "Headless CPU compilation and exact CNB decoding; no GPU upload, rendering, screenshot or visual-quality claim.",
})
