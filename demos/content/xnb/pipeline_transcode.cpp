// SPDX-License-Identifier: MS-PL
// Transcode every supported built-in XNB root into CNA's native CNB schemas.
#include <algorithm>
#include <array>
#include <bit>
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
#include "CNA/Content/Cnb/CnbCurveCodec.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbMediaCodec.hpp"
#include "CNA/Content/Cnb/CnbModelV2Codec.hpp"
#include "CNA/Content/Cnb/CnbSoundEffectCodec.hpp"
#include "CNA/Content/Cnb/CnbSpriteFontCodec.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "CNA/Content/Pipeline/SongContentPipeline.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "CNA/Content/Pipeline/VideoContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnbContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
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

    class ByteWriter
    {
    public:
        void U8(std::uint8_t value) { bytes.push_back(value); }

        void U32(std::uint32_t value)
        {
            for (unsigned int shift = 0u; shift < 32u; shift += 8u)
                U8(static_cast<std::uint8_t>(value >> shift));
        }

        void I32(std::int32_t value) { U32(static_cast<std::uint32_t>(value)); }
        void F32(float value) { U32(std::bit_cast<std::uint32_t>(value)); }

        void SevenBit(std::uint32_t value)
        {
            while (value >= 0x80u)
            {
                U8(static_cast<std::uint8_t>(value | 0x80u));
                value >>= 7u;
            }
            U8(static_cast<std::uint8_t>(value));
        }

        void String(const std::string& value)
        {
            SevenBit(static_cast<std::uint32_t>(value.size()));
            bytes.insert(bytes.end(), value.begin(), value.end());
        }

        void Bytes(const std::vector<std::uint8_t>& value)
        {
            bytes.insert(bytes.end(), value.begin(), value.end());
        }

        std::vector<std::uint8_t> bytes;
    };

    std::vector<std::uint8_t> MakeXnb(const std::vector<std::string>& readers,
                                      const std::vector<std::uint8_t>& payload)
    {
        ByteWriter body;
        body.SevenBit(static_cast<std::uint32_t>(readers.size()));
        for (const std::string& reader : readers)
        {
            body.String(reader);
            body.I32(0);
        }
        body.SevenBit(0u); // no shared resources
        body.SevenBit(1u); // root uses the first table entry
        body.Bytes(payload);

        std::vector<std::uint8_t> result{
            'X', 'N', 'B', 'w', 5u, 0u, 0u, 0u, 0u, 0u};
        result.insert(result.end(), body.bytes.begin(), body.bytes.end());
        const std::uint32_t size = static_cast<std::uint32_t>(result.size());
        for (unsigned int byte = 0u; byte < 4u; ++byte)
            result[6u + byte] = static_cast<std::uint8_t>(size >> (byte * 8u));
        return result;
    }

    std::vector<std::uint8_t> MakeTexture3DXnb()
    {
        ByteWriter payload;
        payload.I32(0); // SurfaceFormat.Color
        payload.I32(4);
        payload.I32(2);
        payload.I32(2);
        payload.I32(3);
        for (const std::size_t size : {64u, 8u, 4u})
        {
            payload.I32(static_cast<std::int32_t>(size));
            for (std::size_t index = 0u; index < size; ++index)
                payload.U8(static_cast<std::uint8_t>(index * 17u + size));
        }
        return MakeXnb({"Microsoft.Xna.Framework.Content.Texture3DReader"}, payload.bytes);
    }

    std::vector<std::uint8_t> MakeTextureCubeXnb()
    {
        const std::array<std::array<std::uint8_t, 4>, 6> colors{{
            {{255u, 0u, 0u, 255u}}, {{0u, 255u, 0u, 255u}},
            {{0u, 0u, 255u, 255u}}, {{255u, 255u, 0u, 255u}},
            {{255u, 0u, 255u, 255u}}, {{0u, 255u, 255u, 255u}}}};
        ByteWriter payload;
        payload.I32(0); // SurfaceFormat.Color
        payload.I32(4); // square face size
        payload.I32(1); // mip levels
        for (const auto& color : colors)
        {
            payload.I32(4 * 4 * 4);
            for (int texel = 0; texel < 16; ++texel)
                payload.bytes.insert(payload.bytes.end(), color.begin(), color.end());
        }
        return MakeXnb({"Microsoft.Xna.Framework.Content.TextureCubeReader"}, payload.bytes);
    }

    std::vector<std::uint8_t> MakeSongXnb(const std::string& mediaPath)
    {
        ByteWriter payload;
        payload.String(mediaPath);
        payload.SevenBit(2u); // Int32Reader object dispatch
        payload.I32(4321);
        return MakeXnb({"Microsoft.Xna.Framework.Content.SongReader",
                        "Microsoft.Xna.Framework.Content.Int32Reader"}, payload.bytes);
    }

    std::vector<std::uint8_t> MakeVideoXnb(const std::string& mediaPath)
    {
        ByteWriter payload;
        payload.SevenBit(2u); // StringReader
        payload.String(mediaPath);
        payload.SevenBit(3u); payload.I32(2345); // Int32Reader: duration
        payload.SevenBit(3u); payload.I32(320);  // width
        payload.SevenBit(3u); payload.I32(180);  // height
        payload.SevenBit(4u); payload.F32(24.0f); // SingleReader
        payload.SevenBit(3u); payload.I32(2);    // soundtrack
        return MakeXnb({"Microsoft.Xna.Framework.Content.VideoReader",
                        "Microsoft.Xna.Framework.Content.StringReader",
                        "Microsoft.Xna.Framework.Content.Int32Reader",
                        "Microsoft.Xna.Framework.Content.SingleReader"}, payload.bytes);
    }

    void WriteBytes(const std::filesystem::path& path,
                    const std::vector<std::uint8_t>& bytes)
    {
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        if (!output) throw std::runtime_error("Could not write a temporary XNB source");
    }

    Cnb::CnbDocument Parse(const Pipeline::ContentBuildResult& result,
                           const std::string& name)
    {
        return Cnb::CnbDocument::Parse(result.output.bytes, name);
    }

    void XnbPipelineTranscode(CnaDemos::DemoContext& ctx)
    {
        const Pipeline::XnbImporter importer;
        const std::vector<std::string> expectedTypes{
            Pipeline::ImportedImageType,
            Pipeline::ImportedSpriteFontType,
            Pipeline::ImportedSoundType,
            Pipeline::ImportedTexture3DType,
            Pipeline::ImportedTextureCubeType,
            Pipeline::ImportedCurveType,
            Pipeline::ImportedSongSourceType,
            Pipeline::ImportedXnbVideoType,
            Pipeline::ImportedModelDocumentType};
        ctx.Check(importer.Identity() == Pipeline::ContentComponentIdentity{"CNA.XnbImporter", "3"} &&
                      importer.SourceExtensions() == std::vector<std::string>{".xnb"} &&
                      importer.OutputTypes() == expectedTypes && !importer.SelectedByNameOnly(),
                  "the XNB importer advertises its stable source route and all nine canonical outputs");

        const Pipeline::XnbVideoProcessor videoProcessor;
        ctx.Check(videoProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.XnbVideoProcessor", "2"} &&
                      videoProcessor.InputType() == Pipeline::ImportedXnbVideoType &&
                      videoProcessor.OutputType() == Pipeline::ProcessedVideoType,
                  "the XNB video adapter connects imported compatibility metadata to native video data");

        Pipeline::ContentProcessorParameters invalid;
        invalid.Set("quality", std::string("High"));
        bool unknownRefused = false;
        try { videoProcessor.ValidateParameters(invalid); }
        catch (const Pipeline::ContentParameterError& error)
        {
            unknownRefused = error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "quality";
        }
        invalid = {};
        invalid.Set(Pipeline::VideoWidthParameter, std::uint64_t{0u});
        bool rangeRefused = false;
        try { videoProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument& error)
        {
            rangeRefused = std::string(error.what()).find("dimension override") != std::string::npos;
        }
        ctx.Check(unknownRefused && rangeRefused,
                  "video overrides distinguish unknown settings from invalid typed ranges");

        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterTexture2DContentPipeline(*registry);
        Pipeline::RegisterSoundEffectContentPipeline(*registry);
        Pipeline::RegisterSongContentPipeline(*registry);
        Pipeline::RegisterVideoContentPipeline(*registry);
        Pipeline::RegisterModelContentPipeline(*registry);
        Pipeline::RegisterCnjContentPipeline(*registry);
        Pipeline::RegisterXnbContentPipeline(*registry);
        ctx.Check(registry->ResolveImporter("legacy.xnb")->Identity() == importer.Identity() &&
                      registry->ResolveProcessor(Pipeline::ImportedXnbVideoType)->Identity() ==
                          videoProcessor.Identity(),
                  "RegisterXnbContentPipeline wires the importer and XNB-specific video adapter");

        const auto root = std::filesystem::temp_directory_path() /
            ("cna-demos-xnb-transcode-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root))
            throw std::runtime_error("Private XNB source directory already exists");
        TemporarySources cleanup(root);
        const std::filesystem::path assets =
            std::filesystem::path(TitleLocation::Path()) / "assets/xnb";
        const auto copy = [&](const std::filesystem::path& from, const std::string& to)
        {
            std::filesystem::copy_file(assets / from, root / to);
        };
        copy("textures/colour_wheel.xnb", "texture2d.xnb");
        copy("fonts/pixel_font.xnb", "font.xnb");
        copy("sounds/chime.xnb", "sound.xnb");
        copy("curves/bounce.xnb", "curve.xnb");
        copy("models/twin_panels.xnb", "model.xnb");
        WriteBytes(root / "volume.xnb", MakeTexture3DXnb());
        WriteBytes(root / "cube.xnb", MakeTextureCubeXnb());
        WriteBytes(root / "song.xnb", MakeSongXnb("music.wma"));
        WriteBytes(root / "music.ogg", {1u, 3u, 5u, 7u, 9u});
        WriteBytes(root / "video.xnb", MakeVideoXnb("clip.wmv"));
        WriteBytes(root / "clip.ogv", {2u, 4u, 6u, 8u});

        const Pipeline::ContentPipeline pipeline(registry);
        const auto build = [&](const std::string& source, const std::string& logicalName,
                               const Pipeline::ContentProcessorParameters& parameters = {})
        {
            Pipeline::ContentBuildRequest request;
            request.sourceRoot = root;
            request.source = source;
            request.logicalName = logicalName;
            request.parameters = parameters;
            return pipeline.Build(request);
        };

        const auto texture2d = build("texture2d.xnb", "Legacy/colour_wheel");
        const auto decodedTexture2d = Cnb::DecodeTexture2DFromCnb(Parse(texture2d, "texture2d.cnb"));
        ctx.Check(texture2d.importer == importer.Identity() &&
                      texture2d.processor.name == "CNA.TextureProcessor" &&
                      texture2d.output.assetTypeId == Cnb::CnbAssetTypeId::Texture2D &&
                      decodedTexture2d.width == 128u && decodedTexture2d.height == 128u &&
                      decodedTexture2d.faceCount == 1u &&
                      decodedTexture2d.representations.size() == 1u &&
                      !decodedTexture2d.representations[0].levels.empty() &&
                      !decodedTexture2d.representations[0].levels[0].empty(),
                  "a project-built Texture2D XNB becomes populated canonical native pixels");
        ctx.Check(build("texture2d.xnb", "Legacy/colour_wheel").output.bytes ==
                          texture2d.output.bytes,
                  "repeating an XNB source build produces byte-identical CNB content");

        const auto font = build("font.xnb", "Legacy/pixel_font");
        const auto decodedFont = Cnb::DecodeSpriteFontFromCnb(Parse(font, "font.cnb"));
        ctx.Check(font.output.assetTypeId == Cnb::CnbAssetTypeId::SpriteFont &&
                      font.processor.name == "CNA.SpriteFontProcessor" &&
                      decodedFont.characters.size() >= 95u &&
                      decodedFont.glyphBounds.size() == decodedFont.characters.size() &&
                      decodedFont.cropping.size() == decodedFont.characters.size() &&
                      decodedFont.kerning.size() == decodedFont.characters.size(),
                  "a project-built SpriteFont XNB preserves a complete parallel glyph table");

        const auto sound = build("sound.xnb", "Legacy/chime");
        const auto decodedSound = Cnb::DecodeSoundEffectFromCnb(Parse(sound, "sound.cnb"));
        ctx.Check(sound.output.assetTypeId == Cnb::CnbAssetTypeId::SoundEffect &&
                      sound.processor.name == "CNA.SoundEffectProcessor" &&
                      decodedSound.sampleRate == 22050u && decodedSound.channels == 1u &&
                      decodedSound.frameCount == 6615u &&
                      decodedSound.samples.size() == 13230u,
                  "a project-built SoundEffect XNB becomes populated native PCM and metadata");

        const auto curve = build("curve.xnb", "Legacy/bounce");
        const auto decodedCurve = Cnb::DecodeCurveFromCnb(Parse(curve, "curve.cnb"));
        ctx.Check(curve.output.assetTypeId == Cnb::CnbAssetTypeId::Curve &&
                      curve.processor.name == "CNA.CurveProcessor" &&
                      decodedCurve.getKeysProperty().getCountProperty() == 3 &&
                      decodedCurve.Evaluate(0.5f) == 1.0f,
                  "a project-built Curve XNB preserves its three-key bounce semantics");

        const auto model = build("model.xnb", "Legacy/twin_panels");
        const Cnb::CnbDocument modelDocument = Parse(model, "model.cnb");
        const auto decodedModel = Cnb::DecodeModelV2FromCnb(modelDocument);
        bool exactModel = decodedModel.bones.size() == 5u &&
            decodedModel.rootBone == 0u && decodedModel.meshes.size() == 2u &&
            decodedModel.vertexDeclarations.size() == 1u &&
            decodedModel.vertexBuffers.size() == 1u &&
            decodedModel.indexBuffers.size() == 1u && decodedModel.effects.size() == 2u &&
            decodedModel.meshes[0].parts.size() == 1u &&
            decodedModel.meshes[1].parts.size() == 1u;
        if (exactModel)
        {
            const auto& left = decodedModel.meshes[0].parts[0];
            const auto& right = decodedModel.meshes[1].parts[0];
            exactModel = left.vertexBuffer == right.vertexBuffer &&
                left.indexBuffer == right.indexBuffer && left.effect != right.effect &&
                left.effect < decodedModel.effects.size() &&
                right.effect < decodedModel.effects.size() &&
                decodedModel.effects[left.effect].diffuse ==
                    std::array<float, 3>{{1.0f, 0.0f, 0.0f}} &&
                decodedModel.effects[right.effect].diffuse ==
                    std::array<float, 3>{{0.0f, 1.0f, 0.0f}};
        }
        ctx.Check(model.output.assetTypeId == Cnb::CnbAssetTypeId::Model &&
                      model.output.assetSchemaVersion == Cnb::CnbModelV2SchemaVersion &&
                      modelDocument.AssetSchemaVersion() == Cnb::CnbModelV2SchemaVersion &&
                      model.processor.name == "CNA.ModelProcessor" &&
                      exactModel,
                  "a Model XNB selects schema 2 and preserves its hierarchy, shared buffers and separate effects");

        const auto volume = build("volume.xnb", "Legacy/volume");
        const auto decodedVolume = Cnb::DecodeTexture3DFromCnb(Parse(volume, "volume.cnb"));
        bool exactVolume = decodedVolume.representations.size() == 1u &&
            decodedVolume.representations[0].levels.size() == 3u;
        if (exactVolume)
        {
            const std::array<std::size_t, 3> sizes{{64u, 8u, 4u}};
            for (std::size_t level = 0u; level < sizes.size(); ++level)
            {
                const auto& bytes = decodedVolume.representations[0].levels[level];
                exactVolume = bytes.size() == sizes[level];
                for (std::size_t index = 0u; exactVolume && index < bytes.size(); ++index)
                {
                    exactVolume = bytes[index] ==
                        static_cast<std::uint8_t>(index * 17u + sizes[level]);
                }
            }
        }
        ctx.Check(volume.output.assetTypeId == Cnb::CnbAssetTypeId::Texture3D &&
                      volume.processor.name == "CNA.Texture3DProcessor" &&
                      decodedVolume.width == 4u && decodedVolume.height == 2u &&
                      decodedVolume.depth == 2u && decodedVolume.mipCount == 3u &&
                      exactVolume,
                  "a Texture3D XNB preserves every byte of all three volume mip levels");

        const auto cube = build("cube.xnb", "Legacy/cube");
        const auto decodedCube = Cnb::DecodeTextureCubeFromCnb(Parse(cube, "cube.cnb"));
        const std::array<std::array<std::uint8_t, 4>, 6> faceColors{{
            {{255u, 0u, 0u, 255u}}, {{0u, 255u, 0u, 255u}},
            {{0u, 0u, 255u, 255u}}, {{255u, 255u, 0u, 255u}},
            {{255u, 0u, 255u, 255u}}, {{0u, 255u, 255u, 255u}}}};
        bool exactCube = decodedCube.representations.size() == 1u &&
            decodedCube.representations[0].levels.size() == faceColors.size();
        if (exactCube)
        {
            for (std::size_t face = 0u; face < faceColors.size(); ++face)
            {
                const auto& pixels = decodedCube.representations[0].levels[face];
                exactCube = pixels.size() == 64u;
                for (std::size_t index = 0u; exactCube && index < pixels.size(); ++index)
                    exactCube = pixels[index] == faceColors[face][index % 4u];
                if (!exactCube) break;
            }
        }
        ctx.Check(cube.output.assetTypeId == Cnb::CnbAssetTypeId::TextureCube &&
                      cube.processor.name == "CNA.TextureCubeProcessor" &&
                      decodedCube.width == 4u && decodedCube.height == 4u &&
                      decodedCube.faceCount == 6u && decodedCube.mipCount == 1u && exactCube,
                  "a TextureCube XNB preserves six distinct faces in canonical order");

        const auto song = build("song.xnb", "Legacy/theme");
        const Cnb::CnbSongData decodedSong = Cnb::DecodeSongFromCnb(Parse(song, "song.cnb"));
        ctx.Check(song.output.assetTypeId == Cnb::CnbAssetTypeId::Song &&
                      song.processor.name == "CNA.SongProcessor" &&
                      decodedSong.streamReference == "music.ogg" &&
                      decodedSong.name == "Legacy/theme" && decodedSong.durationMs == 4321u &&
                      song.dependencies.size() == 2u && song.runtimeReferences.size() == 1u &&
                      song.deploymentFiles.size() == 1u &&
                      song.deploymentFiles[0].outputPath == "music.ogg",
                  "a Song XNB preserves metadata while recording its selected stream as dependency, XREF and deployment");

        const auto video = build("video.xnb", "Legacy/clip");
        const Cnb::CnbVideoData decodedVideo = Cnb::DecodeVideoFromCnb(Parse(video, "video.cnb"));
        ctx.Check(video.importer == importer.Identity() &&
                      video.processor == videoProcessor.Identity() &&
                      video.writer.name == "CNA.VideoContentWriter" &&
                      decodedVideo.streamReference == "clip.ogv" &&
                      decodedVideo.durationMs == 2345u && decodedVideo.width == 320u &&
                      decodedVideo.height == 180u && decodedVideo.framesPerSecond == 24.0f &&
                      decodedVideo.soundtrackType == 2u && video.dependencies.size() == 2u &&
                      video.runtimeReferences.size() == 1u && video.deploymentFiles.size() == 1u,
                  "a Video XNB preserves its native metadata and selected external stream");

        Pipeline::ContentProcessorParameters overrides;
        overrides.Set(Pipeline::VideoStreamReferenceParameter,
                      std::string("published/legacy-clip.ogv"));
        overrides.Set(Pipeline::VideoDurationMsParameter, std::uint64_t{3456u});
        overrides.Set(Pipeline::VideoWidthParameter, std::uint64_t{640u});
        overrides.Set(Pipeline::VideoHeightParameter, std::uint64_t{360u});
        overrides.Set(Pipeline::VideoFramesPerSecondParameter, 30.0);
        overrides.Set(Pipeline::VideoSoundtrackTypeParameter, std::uint64_t{1u});
        const auto overridden = build("video.xnb", "Legacy/clip_override", overrides);
        const Cnb::CnbVideoData decodedOverride =
            Cnb::DecodeVideoFromCnb(Parse(overridden, "video-override.cnb"));
        ctx.Check(decodedOverride.streamReference == "published/legacy-clip.ogv" &&
                      decodedOverride.durationMs == 3456u && decodedOverride.width == 640u &&
                      decodedOverride.height == 360u &&
                      decodedOverride.framesPerSecond == 30.0f &&
                      decodedOverride.soundtrackType == 1u &&
                      overridden.runtimeReferences.size() == 1u &&
                      overridden.runtimeReferences[0].logicalName ==
                          "published/legacy-clip.ogv" &&
                      overridden.deploymentFiles.size() == 1u &&
                      overridden.deploymentFiles[0].source.filename() == "clip.ogv" &&
                      overridden.deploymentFiles[0].outputPath == "published/legacy-clip.ogv",
                  "typed video overrides replace every published metadata field and stream destination");

        ctx.Print("Transcoded 9 XNB roots: Texture2D, SpriteFont, SoundEffect, Curve, Model, Texture3D, TextureCube, Song and Video.");
    }
}

CNA_DEMOS_CONSOLE(XnbPipelineTranscode, {
    .id = "content.xnb.pipeline_transcode",
    .title = "Transcode legacy XNB sources to native CNB",
    .description = "Runs every built-in XNB importer dispatch through CNA's public content pipeline, verifies canonical CNB semantics for nine root types, and demonstrates Video metadata overrides plus external-stream dependency, XREF and deployment behavior.",
    .apis = {"CNA::Content::Pipeline::ImportedXnbVideoType",
             "CNA::Content::Pipeline::ImportedXnbVideo",
             "CNA::Content::Pipeline::XnbImporter::Identity",
             "CNA::Content::Pipeline::XnbImporter::SourceExtensions",
             "CNA::Content::Pipeline::XnbImporter::OutputTypes",
             "CNA::Content::Pipeline::XnbImporter::Import",
             "CNA::Content::Pipeline::XnbVideoProcessor::Identity",
             "CNA::Content::Pipeline::XnbVideoProcessor::InputType",
             "CNA::Content::Pipeline::XnbVideoProcessor::OutputType",
             "CNA::Content::Pipeline::XnbVideoProcessor::ValidateParameters",
             "CNA::Content::Pipeline::XnbVideoProcessor::Process",
             "CNA::Content::Pipeline::RegisterXnbContentPipeline",
             "CNA::Content::Pipeline::ContentPipeline::Build",
             "CNA::Content::Cnb::DecodeTexture2DFromCnb",
             "CNA::Content::Cnb::DecodeTexture3DFromCnb",
             "CNA::Content::Cnb::DecodeTextureCubeFromCnb",
             "CNA::Content::Cnb::DecodeSpriteFontFromCnb",
             "CNA::Content::Cnb::DecodeSoundEffectFromCnb",
             "CNA::Content::Cnb::DecodeCurveFromCnb",
             "CNA::Content::Cnb::DecodeSongFromCnb",
             "CNA::Content::Cnb::DecodeVideoFromCnb",
             "CNA::Content::Cnb::DecodeModelV2FromCnb"},
    .tags = {"content", "pipeline", "xnb", "cnb", "transcode", "compatibility", "cnaext"},
    .assets = {"xnb/textures/colour_wheel.xnb", "xnb/fonts/pixel_font.xnb",
               "xnb/sounds/chime.xnb", "xnb/curves/bounce.xnb",
               "xnb/models/twin_panels.xnb"},
    .restrictions = "Headless CPU transcoding and exact CNB decoding; generated media sidecars are opaque non-playable bytes. No GPU upload, playback, rendering, screenshot or genuine-XNA execution claim.",
})
