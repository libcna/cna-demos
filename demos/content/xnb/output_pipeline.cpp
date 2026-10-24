// SPDX-License-Identifier: MS-PL
// Inspect the XNB target axis: stable asset IDs, option-bound writers and texture selection.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
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
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
namespace Xnb = CNA::Internal::Xnb;
using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
using Microsoft::Xna::Framework::TitleLocation;

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

    std::vector<std::uint8_t> ReadBytes(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Could not read the compiled Effect fixture");
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    std::vector<std::uint8_t> MakeBareEffectSignature()
    {
        std::vector<std::uint8_t> bytes(24u, 0x11u);
        bytes[0] = 0x01u;
        bytes[1] = 0x09u;
        bytes[2] = 0xFFu;
        bytes[3] = 0xFEu;
        return bytes;
    }

    std::vector<std::uint8_t> MakeWrappedEffectSignature()
    {
        std::vector<std::uint8_t> bytes(32u, 0x22u);
        bytes[0] = 0xCFu;
        bytes[1] = 0x0Bu;
        bytes[2] = 0xF0u;
        bytes[3] = 0xBCu;
        bytes[4] = 16u;
        bytes[5] = 0u;
        bytes[6] = 0u;
        bytes[7] = 0u;
        bytes[16] = 0x01u;
        bytes[17] = 0x09u;
        bytes[18] = 0xFFu;
        bytes[19] = 0xFEu;
        return bytes;
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
            // The Model writer also declares the Texture2D children a glTF's extracted images
            // become (CNA-DEMOS-ISSUE-022), ordered by asset type id ahead of its Model schema.
            const bool model = item.inputType == Pipeline::ProcessedModelType;
            const std::size_t own = model ? 1u : 0u;
            exactInventory = exactInventory && writer->InputType() == item.inputType &&
                writer->Identity() == Pipeline::ContentComponentIdentity{
                    item.writerName, "1+w5run"} &&
                writer->OutputFormat() == Pipeline::ContentOutputFormat::Xnb &&
                schemas.size() == own + 1u &&
                schemas[own].assetTypeId == static_cast<std::uint32_t>(item.assetId) &&
                schemas[own].assetSchemaVersion == 5u &&
                schemas[own].assetTypeName == item.assetTypeName &&
                schemas[own].codec == Pipeline::ContentComponentIdentity{
                    "CNA.XnbSerializer", "1+w5run"};
            if (model)
            {
                exactInventory = exactInventory &&
                    schemas[0].assetTypeId == static_cast<std::uint32_t>(Asset::Texture2D) &&
                    schemas[0].assetSchemaVersion == 5u &&
                    schemas[0].assetTypeName == "Microsoft.Xna.Framework.Graphics.Texture2D" &&
                    schemas[0].codec == Pipeline::ContentComponentIdentity{
                        "CNA.XnbSerializer", "1+w5run"};
            }
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

        const auto assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        const std::vector<std::uint8_t> effectBytes =
            ReadBytes(assetRoot / "effects/CnaConformanceEffect.fxb");
        std::vector<std::uint8_t> badWrapper = MakeWrappedEffectSignature();
        badWrapper[4] = 17u;
        const bool actualSignature = Pipeline::IsCompiledEffectBinary(effectBytes);
        const bool bareSignature = Pipeline::IsCompiledEffectBinary(MakeBareEffectSignature());
        const bool wrappedSignature =
            Pipeline::IsCompiledEffectBinary(MakeWrappedEffectSignature());
        const bool emptyRejected = !Pipeline::IsCompiledEffectBinary({});
        const bool foreignRejected = !Pipeline::IsCompiledEffectBinary(
            std::vector<std::uint8_t>(24u, 0u));
        const bool unalignedRejected = !Pipeline::IsCompiledEffectBinary(badWrapper);
        ctx.Check(actualSignature && bareSignature && wrappedSignature && emptyRejected &&
                      foreignRejected && unalignedRejected,
                  "compiled-Effect recognition accepts real, bare and wrapped signatures but rejects truncated, foreign and unaligned inputs");

        const Pipeline::ImportedCompiledEffect imported{effectBytes};
        Pipeline::ImportedCompiledEffect changed = imported;
        changed.bytecode.back() ^= 0x01u;
        ctx.Check(imported == Pipeline::ImportedCompiledEffect{effectBytes} &&
                      imported != changed &&
                      Pipeline::ImportedCompiledEffectType ==
                          std::string("CNA.Content.Pipeline.ImportedCompiledEffect") &&
                      Pipeline::ProcessedCompiledEffectType ==
                          std::string("CNA.Content.Pipeline.CompiledEffect"),
                  "the compiled-Effect carrier owns exact bytecode under stable imported and processed type identities");

        const Pipeline::CompiledEffectImporter effectImporter;
        const Pipeline::CompiledEffectProcessor effectProcessor;
        ctx.Check(effectImporter.Identity() == Pipeline::ContentComponentIdentity{
                          "CNA.CompiledEffectImporter", "1"} &&
                      effectImporter.SourceExtensions() == std::vector<std::string>{".fxb"} &&
                      effectImporter.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedCompiledEffectType} &&
                      !effectImporter.SelectedByNameOnly() &&
                      effectProcessor.Identity() == Pipeline::ContentComponentIdentity{
                          "CNA.CompiledEffectProcessor", "1"} &&
                      effectProcessor.InputType() == Pipeline::ImportedCompiledEffectType &&
                      effectProcessor.OutputType() == Pipeline::ProcessedCompiledEffectType,
                  "the compiled-Effect components expose one unambiguous pass-through route");

        Pipeline::ContentProcessorParameters effectParameters;
        effectParameters.Set("optimize", true);
        bool parameterRefused = false;
        try { effectProcessor.ValidateParameters(effectParameters); }
        catch (const std::invalid_argument& error)
        {
            parameterRefused = std::string(error.what()).find("passed through exactly") !=
                std::string::npos;
        }
        ctx.Check(parameterRefused,
                  "compiled bytecode refuses processor policy because transforming it requires a shader compiler");

        auto effectRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterCompiledEffectContentPipeline(*effectRegistry);
        Pipeline::RegisterXnbOutputContentPipeline(*effectRegistry, defaults);
        const Pipeline::ContentPipeline effectPipeline(effectRegistry);
        Pipeline::ContentBuildRequest effectRequest;
        effectRequest.sourceRoot = assetRoot;
        effectRequest.source = "effects/CnaConformanceEffect.fxb";
        effectRequest.logicalName = "Effects/Conformance";
        effectRequest.outputFormat = Pipeline::ContentOutputFormat::Xnb;
        const Pipeline::ContentBuildResult effect = effectPipeline.Build(effectRequest);
        const Pipeline::ContentBuildResult repeatedEffect = effectPipeline.Build(effectRequest);
        ctx.Check(effect.importer == effectImporter.Identity() &&
                      effect.processor == effectProcessor.Identity() &&
                      effect.writer.name == "CNA.XnbEffectWriter" &&
                      effect.output.assetTypeId == static_cast<std::uint32_t>(Asset::Effect) &&
                      effect.output.assetTypeName ==
                          "Microsoft.Xna.Framework.Graphics.Effect" &&
                      effect.output.rootReaderName ==
                          "Microsoft.Xna.Framework.Content.EffectReader" &&
                      effect.output.bytes == repeatedEffect.output.bytes &&
                      effect.output.bytes.size() > effectBytes.size() &&
                      std::equal(effectBytes.begin(), effectBytes.end(),
                                 effect.output.bytes.end() -
                                     static_cast<std::ptrdiff_t>(effectBytes.size())),
                  "a real fx_2_0 binary passes through unchanged into deterministic Effect XNB output");

        const std::string noCnbReason = Pipeline::CompiledEffectHasNoCnbSchemaReason();
        bool cnbRefused = effectRegistry->AbsentWriterReason(
                              Pipeline::ContentOutputFormat::Cnb,
                              Pipeline::ProcessedCompiledEffectType) == noCnbReason;
        bool cnbBuildFailed = false;
        effectRequest.outputFormat = Pipeline::ContentOutputFormat::Cnb;
        try { static_cast<void>(effectPipeline.Build(effectRequest)); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            cnbBuildFailed = true;
            cnbRefused = cnbRefused &&
                error.Stage() == Pipeline::ContentPipelineStage::Selection &&
                std::string(error.what()).find(noCnbReason) != std::string::npos;
        }
        ctx.Check(cnbBuildFailed && cnbRefused,
                  "the registered CNB absence gives the shared renderer-portability reason instead of producing unusable shader bytes");

        ctx.Print("Registered 10 built-in XNB writers and passed a real fx_2_0 Effect binary through unchanged.");
    }
}

CNA_DEMOS_CONSOLE(XnbOutputPipeline, {
    .id = "content.xnb.output_pipeline",
    .title = "Select and bind XNB output writers",
    .description = "Checks the stable XNB output asset IDs, option digest, version-aware texture selection and all built-in writer registrations, then passes a real compiled Effect through the only portable build-time route while documenting the deliberate CNB absence.",
    .apis = {"CNA::Content::Pipeline::XnbOutputAssetId",
             "CNA::Content::Pipeline::XnbOutputOptionsDigest",
             "CNA::Content::Pipeline::ConvertCnbTextureToXnb",
             "CNA::Content::Pipeline::RegisterXnbOutputContentPipeline",
             "CNA::Content::Pipeline::ContentPipelineRegistry::ResolveWriter",
             "CNA::Content::Pipeline::ContentTypeWriter::Identity",
             "CNA::Content::Pipeline::ContentTypeWriter::OutputFormat",
             "CNA::Content::Pipeline::ContentTypeWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::ContentTypeWriter::Write",
             "CNA::Content::Pipeline::ImportedCompiledEffectType",
             "CNA::Content::Pipeline::ProcessedCompiledEffectType",
             "CNA::Content::Pipeline::ImportedCompiledEffect",
             "CNA::Content::Pipeline::IsCompiledEffectBinary",
             "CNA::Content::Pipeline::CompiledEffectImporter::Identity",
             "CNA::Content::Pipeline::CompiledEffectImporter::SourceExtensions",
             "CNA::Content::Pipeline::CompiledEffectImporter::OutputTypes",
             "CNA::Content::Pipeline::CompiledEffectImporter::Import",
             "CNA::Content::Pipeline::CompiledEffectProcessor::Identity",
             "CNA::Content::Pipeline::CompiledEffectProcessor::InputType",
             "CNA::Content::Pipeline::CompiledEffectProcessor::OutputType",
             "CNA::Content::Pipeline::CompiledEffectProcessor::ValidateParameters",
             "CNA::Content::Pipeline::CompiledEffectProcessor::Process",
             "CNA::Content::Pipeline::RegisterCompiledEffectContentPipeline",
             "CNA::Content::Pipeline::CompiledEffectHasNoCnbSchemaReason"},
    .tags = {"content", "pipeline", "xnb", "writer", "texture", "compression", "effect", "cnaext"},
    .assets = {"effects/CnaConformanceEffect.fxb"},
    .restrictions = "Headless writer registration and exact container-byte checks. The real fx_2_0 fixture is serialized but not executed; compiled-Effect rendering is unavailable in this build, and the emitted probe is not run by genuine XNA.",
})
