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
#include "CNA/Content/Cnb/CnbWriter.hpp"
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

    std::array<float, 16> ModelV2Matrix(const float translation)
    {
        return {{1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 translation, translation + 1.0f, translation + 2.0f, 1.0f}};
    }

    Cnb::CnbModelV2Data CompleteModelV2()
    {
        Cnb::CnbModelV2Data model;
        model.bones = {{"Root", false, -1, ModelV2Matrix(1.0f)},
                       {"Child", false, 0, ModelV2Matrix(4.0f)}};
        model.rootBone = 0u;

        const std::array<Cnb::CnbModelV2VertexFormat, 12> formats{
            Cnb::CnbModelV2VertexFormat::Single,
            Cnb::CnbModelV2VertexFormat::Vector2,
            Cnb::CnbModelV2VertexFormat::Vector3,
            Cnb::CnbModelV2VertexFormat::Vector4,
            Cnb::CnbModelV2VertexFormat::Color,
            Cnb::CnbModelV2VertexFormat::Byte4,
            Cnb::CnbModelV2VertexFormat::Short2,
            Cnb::CnbModelV2VertexFormat::Short4,
            Cnb::CnbModelV2VertexFormat::NormalizedShort2,
            Cnb::CnbModelV2VertexFormat::NormalizedShort4,
            Cnb::CnbModelV2VertexFormat::HalfVector2,
            Cnb::CnbModelV2VertexFormat::HalfVector4};
        const std::array<std::uint32_t, 12> formatSizes{
            4u, 8u, 12u, 16u, 4u, 4u, 4u, 8u, 4u, 8u, 4u, 8u};
        Cnb::CnbModelV2VertexDeclaration completeDeclaration;
        std::uint32_t offset = 0u;
        for (std::size_t index = 0u; index < formats.size(); ++index)
        {
            completeDeclaration.elements.push_back({
                offset, formats[index], static_cast<Cnb::CnbModelV2VertexUsage>(index), 0u});
            offset += formatSizes[index];
        }
        completeDeclaration.vertexStride = offset;
        Cnb::CnbModelV2VertexDeclaration finalUsage{
            4u, {{0u, Cnb::CnbModelV2VertexFormat::Single,
                  Cnb::CnbModelV2VertexUsage::TessellateFactor, 3u}}};
        model.vertexDeclarations = {completeDeclaration, finalUsage};

        std::vector<std::uint8_t> firstVertices(completeDeclaration.vertexStride * 4u);
        for (std::size_t index = 0u; index < firstVertices.size(); ++index)
            firstVertices[index] = static_cast<std::uint8_t>(index * 13u + 7u);
        model.vertexBuffers = {{0u, 4u, firstVertices},
                               {1u, 3u, {1u, 2u, 3u, 4u, 5u, 6u,
                                         7u, 8u, 9u, 10u, 11u, 12u}}};
        model.indexBuffers = {{2u, 6u, {0u, 0u, 1u, 0u, 2u, 0u,
                                        2u, 0u, 1u, 0u, 0u, 0u}},
                              {4u, 3u, {0u, 0u, 0u, 0u,
                                        1u, 0u, 0u, 0u,
                                        2u, 0u, 0u, 0u}}};

        Cnb::CnbModelV2Effect basic;
        basic.kind = Cnb::CnbModelV2EffectKind::BasicEffect;
        basic.primaryTexture = "Textures/basic";
        basic.diffuse = {{0.1f, 0.2f, 0.3f}};
        basic.emissive = {{0.4f, 0.5f, 0.6f}};
        basic.specular = {{0.7f, 0.8f, 0.9f}};
        basic.specularPower = 12.0f;
        basic.alpha = 0.75f;
        basic.vertexColorEnabled = true;

        Cnb::CnbModelV2Effect skinned;
        skinned.kind = Cnb::CnbModelV2EffectKind::SkinnedEffect;
        skinned.primaryTexture = "Textures/skinned";
        skinned.diffuse = {{0.2f, 0.3f, 0.4f}};
        skinned.emissive = {{0.5f, 0.6f, 0.7f}};
        skinned.specular = {{0.8f, 0.9f, 1.0f}};
        skinned.specularPower = 24.0f;
        skinned.alpha = 0.8f;
        skinned.weightsPerVertex = 4u;

        Cnb::CnbModelV2Effect dual;
        dual.kind = Cnb::CnbModelV2EffectKind::DualTextureEffect;
        dual.primaryTexture = "Textures/dual-a";
        dual.secondaryTexture = "Textures/dual-b";
        dual.diffuse = {{0.3f, 0.4f, 0.5f}};
        dual.alpha = 0.85f;
        dual.vertexColorEnabled = true;

        Cnb::CnbModelV2Effect alpha;
        alpha.kind = Cnb::CnbModelV2EffectKind::AlphaTestEffect;
        alpha.primaryTexture = "Textures/alpha";
        alpha.diffuse = {{0.4f, 0.5f, 0.6f}};
        alpha.alpha = 0.9f;
        alpha.alphaFunction = 7u;
        alpha.referenceAlpha = 173u;
        alpha.vertexColorEnabled = true;

        Cnb::CnbModelV2Effect environment;
        environment.kind = Cnb::CnbModelV2EffectKind::EnvironmentMapEffect;
        environment.primaryTexture = "Textures/environment-base";
        environment.cubeTexture = "Textures/environment-cube";
        environment.diffuse = {{0.5f, 0.6f, 0.7f}};
        environment.emissive = {{0.1f, 0.2f, 0.3f}};
        environment.specular = {{0.7f, 0.8f, 0.9f}};
        environment.environmentMapAmount = 0.65f;
        environment.fresnelFactor = 0.25f;
        environment.alpha = 0.95f;
        model.effects = {basic, skinned, dual, alpha, environment};

        Cnb::CnbModelV2Mesh effectMesh;
        effectMesh.name = "AllEffects";
        effectMesh.parentBone = 1;
        effectMesh.boundingSphere = {{1.0f, 2.0f, 3.0f, 4.0f}};
        for (std::uint32_t effect = 0u; effect < model.effects.size(); ++effect)
        {
            effectMesh.parts.push_back({1u, 3u, effect % 2u == 0u ? 0u : 3u,
                                        1u, 0u, 0u, effect});
        }
        Cnb::CnbModelV2Mesh wideIndexMesh{
            "WideIndices", 0, {{-1.0f, -2.0f, -3.0f, 5.0f}},
            {{0u, 3u, 0u, 1u, 1u, 1u, 0u}}};
        model.meshes = {effectMesh, wideIndexMesh};
        return model;
    }

    Cnb::CnbDocument BareModelV2Document(const std::uint32_t assetType,
                                         const std::uint32_t schema,
                                         const int headerCopies)
    {
        Cnb::CnbWriter writer(assetType, schema);
        for (int copy = 0; copy < headerCopies; ++copy)
            writer.AddChunk(Cnb::CnbModelV2Chunk::Header, {}, Cnb::CnbChunkFlags::Mandatory, 4u);
        return Cnb::CnbDocument::Parse(writer.Build(), "bare Model schema-2 CNB");
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

        ctx.Check(Cnb::CnbModelV2Chunk::Header == Cnb::MakeChunkId('M', '2', 'H', 'D') &&
                      Cnb::CnbModelV2Chunk::Strings == Cnb::MakeChunkId('M', '2', 'S', 'T') &&
                      Cnb::CnbModelV2Chunk::Bones == Cnb::MakeChunkId('M', '2', 'B', 'N') &&
                      Cnb::CnbModelV2Chunk::Meshes == Cnb::MakeChunkId('M', '2', 'M', 'S') &&
                      Cnb::CnbModelV2Chunk::Parts == Cnb::MakeChunkId('M', '2', 'P', 'T') &&
                      Cnb::CnbModelV2Chunk::VertexDeclarations ==
                          Cnb::MakeChunkId('M', '2', 'V', 'D') &&
                      Cnb::CnbModelV2Chunk::VertexResources ==
                          Cnb::MakeChunkId('M', '2', 'V', 'R') &&
                      Cnb::CnbModelV2Chunk::VertexData == Cnb::MakeChunkId('M', 'V', 'T', 'X') &&
                      Cnb::CnbModelV2Chunk::IndexResources ==
                          Cnb::MakeChunkId('M', '2', 'I', 'R') &&
                      Cnb::CnbModelV2Chunk::IndexData == Cnb::MakeChunkId('M', 'I', 'D', 'X') &&
                      Cnb::CnbModelV2Chunk::Effects == Cnb::MakeChunkId('M', '2', 'F', 'X') &&
                      Cnb::CnbModelV2SchemaVersion == 2u && Cnb::CnbModelV2HeaderStride == 64u &&
                      Cnb::CnbModelV2BoneStride == 72u && Cnb::CnbModelV2MeshStride == 32u &&
                      Cnb::CnbModelV2PartStride == 32u &&
                      Cnb::CnbModelV2DeclarationStride == 16u &&
                      Cnb::CnbModelV2ElementStride == 20u &&
                      Cnb::CnbModelV2ResourceStride == 16u &&
                      Cnb::CnbModelV2EffectStride == 96u &&
                      Cnb::CnbModelV2NoIndex == 0xFFFFFFFFu,
                  "the Model schema-2 chunk IDs, version, row strides and absent-reference sentinel are exact");

        const std::array<Cnb::CnbModelV2VertexFormat, 12> allFormats{
            Cnb::CnbModelV2VertexFormat::Single,
            Cnb::CnbModelV2VertexFormat::Vector2,
            Cnb::CnbModelV2VertexFormat::Vector3,
            Cnb::CnbModelV2VertexFormat::Vector4,
            Cnb::CnbModelV2VertexFormat::Color,
            Cnb::CnbModelV2VertexFormat::Byte4,
            Cnb::CnbModelV2VertexFormat::Short2,
            Cnb::CnbModelV2VertexFormat::Short4,
            Cnb::CnbModelV2VertexFormat::NormalizedShort2,
            Cnb::CnbModelV2VertexFormat::NormalizedShort4,
            Cnb::CnbModelV2VertexFormat::HalfVector2,
            Cnb::CnbModelV2VertexFormat::HalfVector4};
        bool formatOrdinalsExact = true;
        for (std::size_t index = 0u; index < allFormats.size(); ++index)
            formatOrdinalsExact = formatOrdinalsExact &&
                static_cast<std::uint32_t>(allFormats[index]) == index;
        ctx.Check(formatOrdinalsExact,
                  "all twelve stable schema-2 vertex-format ordinals remain contiguous and exact");

        const std::array<Cnb::CnbModelV2VertexUsage, 13> allUsages{
            Cnb::CnbModelV2VertexUsage::Position, Cnb::CnbModelV2VertexUsage::Color,
            Cnb::CnbModelV2VertexUsage::TextureCoordinate, Cnb::CnbModelV2VertexUsage::Normal,
            Cnb::CnbModelV2VertexUsage::Binormal, Cnb::CnbModelV2VertexUsage::Tangent,
            Cnb::CnbModelV2VertexUsage::BlendIndices, Cnb::CnbModelV2VertexUsage::BlendWeight,
            Cnb::CnbModelV2VertexUsage::Depth, Cnb::CnbModelV2VertexUsage::Fog,
            Cnb::CnbModelV2VertexUsage::PointSize, Cnb::CnbModelV2VertexUsage::Sample,
            Cnb::CnbModelV2VertexUsage::TessellateFactor};
        bool usageOrdinalsExact = true;
        for (std::size_t index = 0u; index < allUsages.size(); ++index)
            usageOrdinalsExact = usageOrdinalsExact &&
                static_cast<std::uint32_t>(allUsages[index]) == index;
        ctx.Check(usageOrdinalsExact,
                  "all thirteen stable schema-2 vertex-usage ordinals remain contiguous and exact");

        ctx.Check(static_cast<std::uint32_t>(Cnb::CnbModelV2EffectKind::BasicEffect) == 0u &&
                      static_cast<std::uint32_t>(Cnb::CnbModelV2EffectKind::SkinnedEffect) == 1u &&
                      static_cast<std::uint32_t>(Cnb::CnbModelV2EffectKind::DualTextureEffect) == 2u &&
                      static_cast<std::uint32_t>(Cnb::CnbModelV2EffectKind::AlphaTestEffect) == 3u &&
                      static_cast<std::uint32_t>(Cnb::CnbModelV2EffectKind::EnvironmentMapEffect) == 4u,
                  "all five stock-effect discriminants remain exact");

        const auto realReencoded = Cnb::EncodeModelV2ToCnb(decodedModel, "Legacy/twin-panels-v2");
        const Cnb::CnbDocument realReencodedDocument =
            Cnb::CnbDocument::Parse(realReencoded, "re-encoded real Model schema-2 CNB");
        ctx.Check(realReencoded ==
                          Cnb::EncodeModelV2ToCnb(decodedModel, "Legacy/twin-panels-v2") &&
                      Cnb::DecodeModelV2FromCnb(realReencodedDocument) == decodedModel &&
                      realReencodedDocument.Metadata().assetTypeName ==
                          "Microsoft.Xna.Framework.Graphics.Model" &&
                      realReencodedDocument.Metadata().contentName == "Legacy/twin-panels-v2",
                  "the genuine-XNB-derived shared-resource model re-encodes byte-identically and round trips exactly");

        const Cnb::CnbModelV2Data completeModel = CompleteModelV2();
        const auto completeBytes =
            Cnb::EncodeModelV2ToCnb(completeModel, "Models/complete-v2");
        const Cnb::CnbDocument completeDocument =
            Cnb::CnbDocument::Parse(completeBytes, "complete Model schema-2 CNB");
        const Cnb::CnbModelV2Data restoredComplete =
            Cnb::DecodeModelV2FromCnb(completeDocument);
        ctx.Check(completeBytes ==
                          Cnb::EncodeModelV2ToCnb(completeModel, "Models/complete-v2") &&
                      restoredComplete == completeModel &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::Header).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::Strings).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::Bones).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::Meshes).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::Parts).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::VertexDeclarations).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::VertexResources).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::VertexData).size() == 2u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::IndexResources).size() == 1u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::IndexData).size() == 2u &&
                      completeDocument.FindAll(Cnb::CnbModelV2Chunk::Effects).size() == 1u,
                  "a complete schema-2 value is byte-deterministic and restores every nested field and chunk");

        ctx.Check(restoredComplete.vertexDeclarations.size() == 2u &&
                      restoredComplete.vertexDeclarations[0].elements.size() == 12u &&
                      restoredComplete.vertexDeclarations[1].elements.size() == 1u &&
                      restoredComplete.vertexBuffers.size() == 2u &&
                      restoredComplete.vertexBuffers[0].bytes ==
                          completeModel.vertexBuffers[0].bytes &&
                      restoredComplete.indexBuffers.size() == 2u &&
                      restoredComplete.indexBuffers[0].indexElementSize == 2u &&
                      restoredComplete.indexBuffers[1].indexElementSize == 4u &&
                      restoredComplete.meshes.size() == 2u &&
                      restoredComplete.meshes[0].parts.size() == 5u &&
                      restoredComplete.meshes[1].parts.size() == 1u,
                  "all formats/usages, both index widths, shared draw resources and exact mesh bounds survive");

        ctx.Check(restoredComplete.effects.size() == 5u &&
                      restoredComplete.effects[0].primaryTexture == "Textures/basic" &&
                      restoredComplete.effects[0].vertexColorEnabled &&
                      restoredComplete.effects[1].weightsPerVertex == 4u &&
                      restoredComplete.effects[2].secondaryTexture == "Textures/dual-b" &&
                      restoredComplete.effects[3].alphaFunction == 7u &&
                      restoredComplete.effects[3].referenceAlpha == 173u &&
                      restoredComplete.effects[4].cubeTexture == "Textures/environment-cube" &&
                      restoredComplete.effects[4].environmentMapAmount == 0.65f &&
                      restoredComplete.effects[4].fresnelFactor == 0.25f,
                  "all five canonical stock-effect payload shapes retain their active fields and typed references");

        Cnb::CnbModelV2Data nullNamed = completeModel;
        nullNamed.bones[1].name.clear();
        nullNamed.bones[1].nameIsNull = true;
        const Cnb::CnbModelV2Data restoredNullNamed = Cnb::DecodeModelV2FromCnb(
            Cnb::CnbDocument::Parse(Cnb::EncodeModelV2ToCnb(nullNamed),
                                    "null-name Model schema-2 CNB"));
        ctx.Check(restoredNullNamed.bones[1].name.empty() &&
                      !restoredNullNamed.bones[1].nameIsNull && restoredNullNamed != nullNamed,
                  "the documented memory-only null-name distinction intentionally becomes an empty name through CNB");

        Cnb::CnbModelV2Data graphOnly;
        graphOnly.bones = {{"Only", false, -1, ModelV2Matrix(0.0f)}};
        const Cnb::CnbModelV2Data restoredGraphOnly = Cnb::DecodeModelV2FromCnb(
            Cnb::CnbDocument::Parse(Cnb::EncodeModelV2ToCnb(graphOnly),
                                    "graph-only Model schema-2 CNB"));
        ctx.Check(restoredGraphOnly == graphOnly && restoredGraphOnly.meshes.empty() &&
                      restoredGraphOnly.vertexDeclarations.empty() &&
                      restoredGraphOnly.vertexBuffers.empty() &&
                      restoredGraphOnly.indexBuffers.empty() && restoredGraphOnly.effects.empty(),
                  "one root bone with no meshes or resources is the schema's valid minimal document");

        Cnb::CnbModelV2Data invalidModel = completeModel;
        invalidModel.rootBone = 99u;
        const bool badRoot = Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        invalidModel = completeModel;
        invalidModel.vertexDeclarations[0].elements[1].offset = 0u;
        const bool badDeclaration =
            Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        invalidModel = completeModel;
        invalidModel.vertexBuffers[0].bytes.pop_back();
        const bool badVertexResource =
            Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        invalidModel = completeModel;
        invalidModel.meshes[0].parts[0].effect = 99u;
        const bool badPartResource =
            Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        invalidModel = completeModel;
        invalidModel.indexBuffers[0].bytes[0] = 3u;
        const bool badDrawIndex =
            Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        invalidModel = completeModel;
        invalidModel.effects[2].emissive[0] = 1.0f;
        const bool badEffect = Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        invalidModel = completeModel;
        invalidModel.meshes[0].boundingSphere[3] = -1.0f;
        const bool badBound = Refuses([&] { (void)Cnb::EncodeModelV2ToCnb(invalidModel); });
        ctx.Check(badRoot && badDeclaration && badVertexResource && badPartResource &&
                      badDrawIndex && badEffect && badBound,
                  "the shared validator refuses invalid graph, declaration, payload, draw, effect and bound data");

        ctx.Check(Refuses([&]
                  {
                      (void)Cnb::DecodeModelV2FromCnb(BareModelV2Document(
                          Cnb::CnbAssetTypeId::Curve, Cnb::CnbModelV2SchemaVersion, 0));
                  }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelV2FromCnb(BareModelV2Document(
                              Cnb::CnbAssetTypeId::Model,
                              Cnb::CnbModelV2SchemaVersion + 1u, 0));
                      }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelV2FromCnb(BareModelV2Document(
                              Cnb::CnbAssetTypeId::Model, Cnb::CnbModelV2SchemaVersion, 0));
                      }) &&
                      Refuses([&]
                      {
                          (void)Cnb::DecodeModelV2FromCnb(BareModelV2Document(
                              Cnb::CnbAssetTypeId::Model, Cnb::CnbModelV2SchemaVersion, 2));
                      }),
                  "the decoder refuses wrong identity/version and missing or duplicate mandatory headers");

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
    .description = "Runs every built-in XNB importer dispatch through CNA's public content pipeline, verifies canonical CNB semantics for nine root types, audits the complete Model schema-2 codec, and demonstrates Video metadata overrides plus external-stream deployment behavior.",
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
             "CNA::Content::Cnb::CnbModelV2VertexFormat",
             "CNA::Content::Cnb::CnbModelV2VertexUsage",
             "CNA::Content::Cnb::CnbModelV2VertexElement",
             "CNA::Content::Cnb::CnbModelV2VertexDeclaration",
             "CNA::Content::Cnb::CnbModelV2VertexBuffer",
             "CNA::Content::Cnb::CnbModelV2IndexBuffer",
             "CNA::Content::Cnb::CnbModelV2EffectKind",
             "CNA::Content::Cnb::CnbModelV2Effect",
             "CNA::Content::Cnb::CnbModelV2Bone",
             "CNA::Content::Cnb::CnbModelV2Part",
             "CNA::Content::Cnb::CnbModelV2Mesh",
             "CNA::Content::Cnb::CnbModelV2Data",
             "CNA::Content::Cnb::CnbModelV2Chunk::Header",
             "CNA::Content::Cnb::CnbModelV2Chunk::Strings",
             "CNA::Content::Cnb::CnbModelV2Chunk::Bones",
             "CNA::Content::Cnb::CnbModelV2Chunk::Meshes",
             "CNA::Content::Cnb::CnbModelV2Chunk::Parts",
             "CNA::Content::Cnb::CnbModelV2Chunk::VertexDeclarations",
             "CNA::Content::Cnb::CnbModelV2Chunk::VertexResources",
             "CNA::Content::Cnb::CnbModelV2Chunk::VertexData",
             "CNA::Content::Cnb::CnbModelV2Chunk::IndexResources",
             "CNA::Content::Cnb::CnbModelV2Chunk::IndexData",
             "CNA::Content::Cnb::CnbModelV2Chunk::Effects",
             "CNA::Content::Cnb::CnbModelV2SchemaVersion",
             "CNA::Content::Cnb::CnbModelV2HeaderStride",
             "CNA::Content::Cnb::CnbModelV2BoneStride",
             "CNA::Content::Cnb::CnbModelV2MeshStride",
             "CNA::Content::Cnb::CnbModelV2PartStride",
             "CNA::Content::Cnb::CnbModelV2DeclarationStride",
             "CNA::Content::Cnb::CnbModelV2ElementStride",
             "CNA::Content::Cnb::CnbModelV2ResourceStride",
             "CNA::Content::Cnb::CnbModelV2EffectStride",
             "CNA::Content::Cnb::CnbModelV2NoIndex",
             "CNA::Content::Cnb::EncodeModelV2ToCnb",
             "CNA::Content::Cnb::DecodeModelV2FromCnb"},
    .tags = {"content", "pipeline", "xnb", "cnb", "transcode", "compatibility", "cnaext"},
    .assets = {"xnb/textures/colour_wheel.xnb", "xnb/fonts/pixel_font.xnb",
               "xnb/sounds/chime.xnb", "xnb/curves/bounce.xnb",
               "xnb/models/twin_panels.xnb"},
    .restrictions = "Headless CPU transcoding and exact CNB decoding; representative Model relationship/schema refusals are covered while arbitrary malformed-container fuzzing remains CNA test-suite work. Generated media sidecars are opaque non-playable bytes. The committed Model source is genuine XNB input, but no genuine-XNA execution, GPU upload, playback, rendering or screenshot is claimed.",
})
