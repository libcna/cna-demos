// SPDX-License-Identifier: MS-PL
// Inspect the XNB target axis: stable asset IDs, option-bound writers and texture selection.
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/EffectContentPipeline.hpp"
#include "CNA/Content/Pipeline/ModelContentPipeline.hpp"
#include "CNA/Content/Pipeline/SongContentPipeline.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "CNA/Content/Pipeline/VideoContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnbOutputContentPipeline.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
namespace Xnb = CNA::Internal::Xnb;
using Microsoft::Xna::Framework::Graphics::SurfaceFormat;

namespace
{
    struct ExpectedWriter
    {
        const char* inputType;
        const char* writerName;
        Pipeline::XnbOutputAssetId assetId;
        const char* assetTypeName;
    };

    Cnb::CnbTextureData MakeDualRepresentationTexture()
    {
        Cnb::CnbTextureData texture;
        texture.width = 4u;
        texture.height = 4u;
        texture.depth = 1u;
        texture.faceCount = 1u;
        texture.mipCount = 1u;
        texture.representations = {
            {Cnb::CnbTextureFormat::Rgba8,
             {std::vector<std::uint8_t>(64u, 0x11u)}},
            {Cnb::CnbTextureFormat::Bgra8,
             {std::vector<std::uint8_t>(64u, 0x22u)}}};
        return texture;
    }

    void XnbOutputPipeline(CnaDemos::DemoContext& ctx)
    {
        using Asset = Pipeline::XnbOutputAssetId;
        ctx.Check(static_cast<std::uint32_t>(Asset::Texture2D) == 1u &&
                      static_cast<std::uint32_t>(Asset::Texture3D) == 2u &&
                      static_cast<std::uint32_t>(Asset::TextureCube) == 3u &&
                      static_cast<std::uint32_t>(Asset::SpriteFont) == 4u &&
                      static_cast<std::uint32_t>(Asset::SoundEffect) == 5u &&
                      static_cast<std::uint32_t>(Asset::Song) == 6u &&
                      static_cast<std::uint32_t>(Asset::Video) == 7u &&
                      static_cast<std::uint32_t>(Asset::Model) == 8u &&
                      static_cast<std::uint32_t>(Asset::Curve) == 9u &&
                      static_cast<std::uint32_t>(Asset::Effect) == 10u &&
                      static_cast<std::uint32_t>(Asset::XnaObject) == 11u,
                  "all eleven persisted XNB output asset identifiers retain their append-only values");

        const Xnb::XnbFileOptions defaults;
        Xnb::XnbFileOptions extended;
        extended.platform = Xnb::XnbTargetPlatform::DesktopGL;
        extended.graphicsProfile = Xnb::XnbGraphicsProfile::HiDef;
        extended.compression = Xnb::XnbOutputCompression::Lz4;
        extended.readerNameStyle = Xnb::XnbReaderNameStyle::Portable;
        ctx.Check(Pipeline::XnbOutputOptionsDigest(defaults) == "w5run" &&
                      Pipeline::XnbOutputOptionsDigest(extended) == "d5h4p",
                  "the writer digest distinguishes platform, version, profile, compression and reader spelling");

        const Cnb::CnbTextureData texture = MakeDualRepresentationTexture();
        const Xnb::XnbTextureData xna40 = Pipeline::ConvertCnbTextureToXnb(
            texture, Xnb::XnbTextureKind::Texture2D, defaults, "Textures/dual");
        Xnb::XnbFileOptions legacy = defaults;
        legacy.version = Xnb::XnbContainerVersion::Legacy4;
        const Xnb::XnbTextureData version4 = Pipeline::ConvertCnbTextureToXnb(
            texture, Xnb::XnbTextureKind::Texture2D, legacy, "Textures/dual");
        ctx.Check(xna40.surfaceFormat == SurfaceFormat::Color &&
                      xna40.levels == texture.representations[0].levels &&
                      version4.surfaceFormat == SurfaceFormat::ColorBgraEXT &&
                      version4.levels == texture.representations[1].levels,
                  "texture conversion chooses the first representation expressible by the selected container version");

        Cnb::CnbTextureData unsupported = texture;
        unsupported.representations = {
            {Cnb::CnbTextureFormat::Bc7,
             {std::vector<std::uint8_t>(16u, 0x77u)}}};
        bool unsupportedRefused = false;
        try
        {
            static_cast<void>(Pipeline::ConvertCnbTextureToXnb(
                unsupported, Xnb::XnbTextureKind::Texture2D, defaults, "Textures/bc7"));
        }
        catch (const std::runtime_error& error)
        {
            const std::string message = error.what();
            unsupportedRefused = message.find("Textures/bc7") != std::string::npos &&
                message.find("Bc7") != std::string::npos &&
                message.find("container-version-5") != std::string::npos;
        }
        ctx.Check(unsupportedRefused,
                  "an unexpressible representation is refused with its format, asset and container version");

        Cnb::CnbTextureData nonSquareCube = texture;
        nonSquareCube.faceCount = 6u;
        nonSquareCube.height = 2u;
        bool shapeRefused = false;
        try
        {
            static_cast<void>(Pipeline::ConvertCnbTextureToXnb(
                nonSquareCube, Xnb::XnbTextureKind::TextureCube, defaults,
                "Textures/non_square"));
        }
        catch (const std::runtime_error& error)
        {
            const std::string message = error.what();
            shapeRefused = message.find("Textures/non_square") != std::string::npos &&
                message.find("square faces") != std::string::npos &&
                message.find("4x2") != std::string::npos;
        }
        ctx.Check(shapeRefused,
                  "texture conversion refuses a shape whose dimensions contradict its XNB kind");

        const std::array<ExpectedWriter, 10> expected{{
            {Pipeline::ProcessedTexture2DType, "CNA.XnbTexture2DWriter", Asset::Texture2D,
             "Microsoft.Xna.Framework.Graphics.Texture2D"},
            {Pipeline::ProcessedTexture3DType, "CNA.XnbTexture3DWriter", Asset::Texture3D,
             "Microsoft.Xna.Framework.Graphics.Texture3D"},
            {Pipeline::ProcessedTextureCubeType, "CNA.XnbTextureCubeWriter", Asset::TextureCube,
             "Microsoft.Xna.Framework.Graphics.TextureCube"},
            {Pipeline::ProcessedSpriteFontType, "CNA.XnbSpriteFontWriter", Asset::SpriteFont,
             "Microsoft.Xna.Framework.Graphics.SpriteFont"},
            {Pipeline::ProcessedSoundEffectType, "CNA.XnbSoundEffectWriter", Asset::SoundEffect,
             "Microsoft.Xna.Framework.Audio.SoundEffect"},
            {Pipeline::ProcessedSongType, "CNA.XnbSongWriter", Asset::Song,
             "Microsoft.Xna.Framework.Media.Song"},
            {Pipeline::ProcessedVideoType, "CNA.XnbVideoWriter", Asset::Video,
             "Microsoft.Xna.Framework.Media.Video"},
            {Pipeline::ProcessedModelType, "CNA.XnbModelWriter", Asset::Model,
             "Microsoft.Xna.Framework.Graphics.Model"},
            {Pipeline::ProcessedCurveType, "CNA.XnbCurveWriter", Asset::Curve,
             "Microsoft.Xna.Framework.Curve"},
            {Pipeline::ProcessedCompiledEffectType, "CNA.XnbEffectWriter", Asset::Effect,
             "Microsoft.Xna.Framework.Graphics.Effect"}}};

        Pipeline::ContentPipelineRegistry xnbRegistry;
        Pipeline::RegisterXnbOutputContentPipeline(xnbRegistry, defaults);
        xnbRegistry.Freeze();
        bool exactInventory = xnbRegistry.Writers().size() == expected.size();
        for (const ExpectedWriter& item : expected)
        {
            const auto writer = xnbRegistry.ResolveWriter(
                item.inputType, {}, Pipeline::ContentOutputFormat::Xnb);
            const auto schemas = writer->OutputSchemaIdentities();
            exactInventory = exactInventory && writer->InputType() == item.inputType &&
                writer->Identity() == Pipeline::ContentComponentIdentity{
                    item.writerName, "1+w5run"} &&
                writer->OutputFormat() == Pipeline::ContentOutputFormat::Xnb &&
                schemas.size() == 1u &&
                schemas[0].assetTypeId == static_cast<std::uint32_t>(item.assetId) &&
                schemas[0].assetSchemaVersion == 5u &&
                schemas[0].assetTypeName == item.assetTypeName &&
                schemas[0].codec == Pipeline::ContentComponentIdentity{
                    "CNA.XnbSerializer", "1+w5run"};
        }
        ctx.Check(exactInventory,
                  "registration exposes all ten built-in asset writers with exact option-bound identities and schemas");

        Pipeline::ContentPipelineRegistry dualRegistry;
        Pipeline::RegisterTexture2DContentPipeline(dualRegistry);
        Pipeline::RegisterXnbOutputContentPipeline(dualRegistry, defaults);
        dualRegistry.Freeze();
        ctx.Check(dualRegistry.ResolveWriter(Pipeline::ProcessedTexture2DType)->Identity().name ==
                          "CNA.Texture2DContentWriter" &&
                      dualRegistry.ResolveWriter(Pipeline::ProcessedTexture2DType, {},
                                                 Pipeline::ContentOutputFormat::Xnb)
                              ->Identity().name == "CNA.XnbTexture2DWriter",
                  "CNB and XNB writers coexist on the output-format axis without shadowing each other");

        Pipeline::ContentPipelineRegistry extendedRegistry;
        Pipeline::RegisterXnbOutputContentPipeline(extendedRegistry, extended);
        extendedRegistry.Freeze();
        const auto writer = extendedRegistry.ResolveWriter(
            Pipeline::ProcessedTexture2DType, {}, Pipeline::ContentOutputFormat::Xnb);
        const Pipeline::ContentValue value = Pipeline::ContentValue::Create(
            Pipeline::ProcessedTexture2DType, texture);
        const Pipeline::ContentWriteResult first = writer->Write(value, "Textures/options");
        const Pipeline::ContentWriteResult second = writer->Write(value, "Textures/options");
        ctx.Check(first.bytes == second.bytes && first.bytes.size() >= 10u &&
                      first.bytes[0] == 'X' && first.bytes[1] == 'N' && first.bytes[2] == 'B' &&
                      first.bytes[3] == 'd' && first.bytes[4] == 5u && first.bytes[5] == 0x41u &&
                      first.assetTypeId == static_cast<std::uint32_t>(Asset::Texture2D) &&
                      first.assetSchemaVersion == 5u &&
                      first.rootReaderName == "Microsoft.Xna.Framework.Content.Texture2DReader" &&
                      writer->Identity().version == "1+d5h4p",
                  "an option-bound writer emits deterministic DesktopGL HiDef LZ4 bytes and reports the actual root reader");

        ctx.Print("Registered 10 built-in XNB writers; XnaObject remains the custom ContentCompiler route.");
    }
}

CNA_DEMOS_CONSOLE(XnbOutputPipeline, {
    .id = "content.xnb.output_pipeline",
    .title = "Select and bind XNB output writers",
    .description = "Checks the stable XNB output asset IDs, option digest, version-aware canonical texture representation selection, all built-in writer registrations, the independent CNB/XNB format axis and deterministic option-bound output bytes.",
    .apis = {"CNA::Content::Pipeline::XnbOutputAssetId",
             "CNA::Content::Pipeline::XnbOutputOptionsDigest",
             "CNA::Content::Pipeline::ConvertCnbTextureToXnb",
             "CNA::Content::Pipeline::RegisterXnbOutputContentPipeline",
             "CNA::Content::Pipeline::ContentPipelineRegistry::ResolveWriter",
             "CNA::Content::Pipeline::ContentTypeWriter::Identity",
             "CNA::Content::Pipeline::ContentTypeWriter::OutputFormat",
             "CNA::Content::Pipeline::ContentTypeWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::ContentTypeWriter::Write"},
    .tags = {"content", "pipeline", "xnb", "writer", "texture", "compression", "cnaext"},
    .restrictions = "Headless writer registration and exact container-byte checks. Existing asset demos own XNB runtime loading, rendering and audio behavior; the emitted probe is not run by genuine XNA.",
})
