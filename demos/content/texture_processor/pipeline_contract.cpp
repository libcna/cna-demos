// SPDX-License-Identifier: MS-PL
// Audit the public Texture2D build-time contract from decoded source pixels to container output.
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    void WriteBigEndian32(std::vector<std::uint8_t>& bytes, const std::size_t offset,
                          const std::uint32_t value)
    {
        bytes[offset] = static_cast<std::uint8_t>(value >> 24);
        bytes[offset + 1u] = static_cast<std::uint8_t>(value >> 16);
        bytes[offset + 2u] = static_cast<std::uint8_t>(value >> 8);
        bytes[offset + 3u] = static_cast<std::uint8_t>(value);
    }

    std::vector<std::uint8_t> MakeGammaPngHeader()
    {
        // ApplyPngFileGammaEXT only consumes the PNG signature, IHDR colour type and chunks before
        // IDAT. CRCs and image data belong to the decoder and are deliberately outside this probe.
        std::vector<std::uint8_t> bytes(49u, 0u);
        const std::array<std::uint8_t, 8> signature{0x89u, 'P', 'N', 'G', '\r', '\n', 0x1Au,
                                                     '\n'};
        std::copy(signature.begin(), signature.end(), bytes.begin());
        WriteBigEndian32(bytes, 8u, 13u);
        std::copy_n("IHDR", 4u, bytes.begin() + 12);
        bytes[25] = 6u; // Truecolour with alpha: the GDI+ gamma path applies.
        WriteBigEndian32(bytes, 33u, 4u);
        std::copy_n("gAMA", 4u, bytes.begin() + 37);
        WriteBigEndian32(bytes, 41u, 45000u);
        return bytes;
    }

    void TexturePipelineContract(CnaDemos::DemoContext& ctx)
    {
        using Format = Pipeline::TextureBuildFormat;
        const std::array<std::pair<const char*, Format>, 6> formats{{
            {"NoChange", Format::NoChange},
            {"Color", Format::Color},
            {"DxtCompressed", Format::DxtCompressed},
            {"Dxt1", Format::Dxt1},
            {"Dxt3", Format::Dxt3},
            {"Dxt5", Format::Dxt5},
        }};
        bool formatRoundTrips = true;
        for (const auto& [name, format] : formats)
        {
            formatRoundTrips = formatRoundTrips &&
                Pipeline::TryParseTextureBuildFormat(name) == format &&
                Pipeline::TextureBuildFormatName(format) == name;
        }
        ctx.Check(formatRoundTrips &&
                      Pipeline::TryParseTextureBuildFormat("dXtCoMpReSsEd") ==
                          Format::DxtCompressed &&
                      !Pipeline::TryParseTextureBuildFormat("Bc7").has_value(),
                  "all six texture formats round-trip canonically, parsing is case-insensitive and an unknown format stays absent");

        ctx.Check(Pipeline::ImportedImageType ==
                          std::string("CNA.Content.Pipeline.ImportedImage") &&
                      Pipeline::ProcessedTexture2DType ==
                          std::string("CNA.Content.Cnb.Texture2DData") &&
                      Pipeline::TextureColorKeyParameter == std::string("colorKey") &&
                      Pipeline::TextureFormatParameter == std::string("textureFormat") &&
                      Pipeline::TextureGenerateMipmapsParameter ==
                          std::string("generateMipmaps") &&
                      Pipeline::TexturePremultiplyAlphaParameter ==
                          std::string("premultiplyAlpha") &&
                      Pipeline::TextureResizeToPowerOfTwoParameter ==
                          std::string("resizeToPowerOfTwo") &&
                      Pipeline::TexturePassThroughParameter == std::string("passThrough"),
                  "the imported/processed identities and six processor parameter keys are stable");

        ctx.Check(Pipeline::NextPowerOfTwoDimension(0u) == 1u &&
                      Pipeline::NextPowerOfTwoDimension(1u) == 1u &&
                      Pipeline::NextPowerOfTwoDimension(3u) == 4u &&
                      Pipeline::NextPowerOfTwoDimension(100u) == 128u &&
                      Pipeline::NextPowerOfTwoDimension(1024u) == 1024u,
                  "power-of-two sizing rounds upward while retaining exact powers");

        std::vector<std::uint8_t> line(3u * 4u);
        for (std::size_t texel = 0u; texel < 3u; ++texel)
            std::fill_n(line.begin() + static_cast<std::ptrdiff_t>(texel * 4u), 4u,
                        static_cast<std::uint8_t>(texel * 60u));
        const std::vector<std::uint8_t> shrunk =
            Pipeline::ResampleRgbaImage(line, 3u, 1u, 2u, 1u);
        const std::vector<std::uint8_t> grown = Pipeline::ResampleRgbaImage(
            std::vector<std::uint8_t>{0u, 0u, 0u, 0u, 200u, 200u, 200u, 200u},
            2u, 1u, 4u, 1u);
        ctx.Check(shrunk.size() == 8u && shrunk[0] == 20u && shrunk[4] == 100u &&
                      grown.size() == 16u && grown.front() == 0u && grown[12] == 200u &&
                      grown[4] > grown[0] && grown[8] < grown[12],
                  "resampling area-averages an awkward shrink and interpolates a growth while retaining its endpoints");

        std::vector<std::uint8_t> alphaPixels{
            30u, 60u, 90u, 200u, 1u, 2u, 3u, 255u, 8u, 9u, 10u, 0u, 99u};
        Pipeline::PremultiplyRgbaAlpha(alphaPixels);
        ctx.Check(alphaPixels == std::vector<std::uint8_t>{
                                      23u, 47u, 70u, 200u, 1u, 2u, 3u, 255u,
                                      0u, 0u, 0u, 0u, 99u},
                  "alpha multiplication truncates partial channels, preserves opaque pixels, clears transparent colour and ignores a partial texel");

        const std::vector<std::vector<std::uint8_t>> mips = Pipeline::GenerateRgbaMipChain(
            std::vector<std::uint8_t>(4u * 2u * 4u, 64u), 4u, 2u);
        ctx.Check(mips.size() == 2u && mips[0].size() == 2u * 1u * 4u &&
                      mips[1] == std::vector<std::uint8_t>(4u, 64u) &&
                      Pipeline::GenerateRgbaMipChain(
                          std::vector<std::uint8_t>(4u, 7u), 1u, 1u)
                          .empty(),
                  "mipmap generation halves both axes to 1x1 and a 1x1 source needs no child level");

        std::vector<std::uint8_t> gammaPixels{128u, 64u, 255u, 77u};
        Pipeline::ApplyPngFileGammaEXT(MakeGammaPngHeader(), gammaPixels);
        std::vector<std::uint8_t> foreignPixels{128u, 64u, 255u, 77u};
        Pipeline::ApplyPngFileGammaEXT(std::vector<std::uint8_t>{'N', 'O', 'T'},
                                       foreignPixels);
        ctx.Check(gammaPixels == std::vector<std::uint8_t>{127u, 63u, 255u, 77u} &&
                      foreignPixels == std::vector<std::uint8_t>{128u, 64u, 255u, 77u},
                  "the PNG gAMA compatibility helper corrects RGB but not alpha and leaves non-PNG input alone");

        const std::filesystem::path assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        const Pipeline::ImportedImage checker =
            Pipeline::DecodeImportedImage(assetRoot / "textures/checker.png");
        ctx.Check(checker.width == 64u && checker.height == 64u &&
                      checker.rgbaPixels.size() == 64u * 64u * 4u &&
                      checker.additionalRgbaMipLevels.empty() &&
                      checker.sourceCompressedLevels.empty() &&
                      !checker.sourceCompressedFormat.has_value() &&
                      !checker.authoredColorKey.has_value() &&
                      !checker.authoredPremultiplyAlpha.has_value(),
                  "a real PNG decodes to one source-oriented Rgba8 level with no authored policy or compressed side channel");

        Pipeline::ImportedImage synthetic;
        synthetic.width = 2u;
        synthetic.height = 2u;
        synthetic.rgbaPixels = std::vector<std::uint8_t>(16u, 9u);
        synthetic.additionalRgbaMipLevels = {std::vector<std::uint8_t>(4u, 3u)};
        const Cnb::CnbTextureData canonical =
            Pipeline::BuildCnbTexture2DData(std::move(synthetic));
        ctx.Check(canonical.width == 2u && canonical.height == 2u && canonical.depth == 1u &&
                      canonical.faceCount == 1u && canonical.mipCount == 2u &&
                      canonical.representations.size() == 1u &&
                      canonical.representations[0].format == Cnb::CnbTextureFormat::Rgba8 &&
                      canonical.representations[0].levels ==
                          std::vector<std::vector<std::uint8_t>>{
                              std::vector<std::uint8_t>(16u, 9u),
                              std::vector<std::uint8_t>(4u, 3u)},
                  "the parameter-free builder preserves level order in one canonical Rgba8 Texture2D representation");

        const Pipeline::ImageImporter importer;
        const Pipeline::TextureProcessor defaultProcessor;
        const Pipeline::Texture2DContentWriter writer;
        const std::vector<std::string> expectedExtensions{
            ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".psd",
            ".hdr", ".pic", ".pnm", ".dds", ".dib", ".pfm", ".ppm"};
        const std::vector<Pipeline::ContentWriterSchemaIdentity> schemas =
            writer.OutputSchemaIdentities();
        ctx.Check(importer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.ImageImporter", "1"} &&
                      importer.SourceExtensions() == expectedExtensions &&
                      importer.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedImageType,
                                                   Pipeline::ImportedTextureCubeType,
                                                   Pipeline::ImportedTexture3DType} &&
                      defaultProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.TextureProcessor", "3"} &&
                      defaultProcessor.InputType() == Pipeline::ImportedImageType &&
                      defaultProcessor.OutputType() == Pipeline::ProcessedTexture2DType &&
                      writer.Identity() == Pipeline::ContentComponentIdentity{
                                               "CNA.Texture2DContentWriter", "1"} &&
                      writer.InputType() == Pipeline::ProcessedTexture2DType &&
                      schemas.size() == 1u &&
                      schemas[0] == Pipeline::ContentWriterSchemaIdentity{
                                            Cnb::CnbAssetTypeId::Texture2D,
                                            Cnb::CnbTextureSchemaVersion,
                                            "Microsoft.Xna.Framework.Graphics.Texture2D",
                                            {"CNA.Cnb.EncodeTexture2DToCnb", "1"}},
                  "the importer, processor and writer expose exact routing, version and persistent schema identities");

        Pipeline::ContentProcessorParameters badParameters;
        badParameters.Set("colourKey", std::string("1,2,3"));
        bool unknownRefused = false;
        try { defaultProcessor.ValidateParameters(badParameters); }
        catch (const Pipeline::ContentParameterError& error)
        {
            unknownRefused = error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "colourKey";
        }
        badParameters = {};
        badParameters.Set(Pipeline::TextureColorKeyParameter, std::string("1,2"));
        bool malformedRefused = false;
        try { defaultProcessor.ValidateParameters(badParameters); }
        catch (const Pipeline::ContentParameterError& error)
        {
            malformedRefused =
                error.Fault() == Pipeline::ContentParameterFault::UnconvertibleValue &&
                error.Parameter() == Pipeline::TextureColorKeyParameter;
        }
        ctx.Check(unknownRefused && malformedRefused,
                  "parameter validation distinguishes an unknown key from an unconvertible value without parsing diagnostic text");

        struct EncoderObservation
        {
            std::size_t calls = 0u;
            Cnb::CnbTextureFormat format = Cnb::CnbTextureFormat::Rgba8;
            std::uint32_t width = 0u;
            std::uint32_t height = 0u;
            std::size_t rgbaBytes = 0u;
        } observation;
        Pipeline::TextureBlockEncoder encoder =
            [&observation](const Cnb::CnbTextureFormat format,
                           const std::span<const std::uint8_t> rgba,
                           const std::uint32_t width, const std::uint32_t height)
            {
                ++observation.calls;
                observation.format = format;
                observation.width = width;
                observation.height = height;
                observation.rgbaBytes = rgba.size();
                const std::size_t blocks =
                    static_cast<std::size_t>((width + 3u) / 4u) * ((height + 3u) / 4u);
                return std::vector<std::uint8_t>(blocks * 8u, 0x5Au);
            };
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterTexture2DContentPipeline(*registry, encoder);
        const Pipeline::ContentPipeline pipeline(registry);

        Pipeline::ContentBuildRequest request;
        request.sourceRoot = assetRoot;
        request.source = "textures/checker.png";
        request.logicalName = "Textures/checker-contract";
        request.outputFormat = Pipeline::ContentOutputFormat::Cnb;
        const Pipeline::ContentBuildResult built = pipeline.Build(request);
        const Pipeline::ContentBuildResult repeated = pipeline.Build(request);
        const Cnb::CnbTextureData decoded = Cnb::DecodeTexture2DFromCnb(
            Cnb::CnbDocument::Parse(built.output.bytes, "texture pipeline contract CNB"));
        ctx.Check(built.importer == importer.Identity() &&
                      built.processor == defaultProcessor.Identity() &&
                      built.writer == writer.Identity() && built.output.bytes == repeated.output.bytes &&
                      decoded.width == checker.width && decoded.height == checker.height &&
                      decoded.mipCount == 1u && decoded.representations.size() == 1u &&
                      decoded.representations[0].format == Cnb::CnbTextureFormat::Rgba8 &&
                      decoded.representations[0].levels[0] == checker.rgbaPixels,
                  "registered components build a real opaque PNG into deterministic CNB bytes whose decoded pixels are exact");

        request.outputFormat = Pipeline::ContentOutputFormat::Xnb;
        request.parameters.Set(Pipeline::TextureFormatParameter, std::string("Dxt1"));
        Pipeline::ContentDependencyCollector dependencies;
        const Pipeline::ContentProcessResult compressed =
            pipeline.ImportAndProcess(request, dependencies);
        const Cnb::CnbTextureData& compressedTexture =
            compressed.processed.Get<Cnb::CnbTextureData>();
        ctx.Check(observation.calls == 1u && observation.format == Cnb::CnbTextureFormat::Bc1 &&
                      observation.width == 64u && observation.height == 64u &&
                      observation.rgbaBytes == checker.rgbaPixels.size() &&
                      compressedTexture.representations.size() == 1u &&
                      compressedTexture.representations[0].format ==
                          Cnb::CnbTextureFormat::Bc1 &&
                      compressedTexture.representations[0].levels.size() == 1u &&
                      compressedTexture.representations[0].levels[0] ==
                          std::vector<std::uint8_t>(2048u, 0x5Au),
                  "the injected block encoder receives the exact requested format, dimensions and RGBA bytes and supplies the processed level");

        const std::size_t callsBeforePassThrough = observation.calls;
        const Pipeline::ImportedImage dds =
            Pipeline::DecodeImportedImage(assetRoot / "textures/striped_mips.dds");
        request.source = "textures/striped_mips.dds";
        request.logicalName = "Textures/striped-pass-through";
        request.parameters = {};
        request.parameters.Set(Pipeline::TexturePassThroughParameter, true);
        const Pipeline::ContentProcessResult passed =
            pipeline.ImportAndProcess(request, dependencies);
        const Cnb::CnbTextureData& passedTexture = passed.processed.Get<Cnb::CnbTextureData>();
        ctx.Check(dds.width == 16u && dds.height == 8u &&
                      dds.sourceCompressedFormat == Cnb::CnbTextureFormat::Bc1 &&
                      !dds.sourceCompressedLevels.empty() &&
                      observation.calls == callsBeforePassThrough &&
                      passedTexture.mipCount == dds.sourceCompressedLevels.size() &&
                      passedTexture.representations.size() == 1u &&
                      passedTexture.representations[0].format == Cnb::CnbTextureFormat::Bc1 &&
                      passedTexture.representations[0].levels == dds.sourceCompressedLevels,
                  "pass-through preserves every authored DDS BC1 block and bypasses the encoder and pixel policies");

        ctx.Print("Audited 14 image routes, 6 build formats and exact PNG/DDS processor paths.");
    }
}

CNA_DEMOS_CONSOLE(TexturePipelineContract, {
    .id = "content.texture_processor.pipeline_contract",
    .title = "Audit the Texture2D pipeline contract",
    .description = "Exercises CNA's public Texture2D build-time contract: component identities, format parsing, decoded-image ownership, deterministic resampling/premultiplication/mip helpers, PNG gamma compatibility, exact CNB output, injected block encoding and DDS pass-through.",
    .apis = {"CNA::Content::Pipeline::ImportedImageType",
             "CNA::Content::Pipeline::ProcessedTexture2DType",
             "CNA::Content::Pipeline::TextureColorKeyParameter",
             "CNA::Content::Pipeline::TextureFormatParameter",
             "CNA::Content::Pipeline::TextureGenerateMipmapsParameter",
             "CNA::Content::Pipeline::TexturePremultiplyAlphaParameter",
             "CNA::Content::Pipeline::TextureResizeToPowerOfTwoParameter",
             "CNA::Content::Pipeline::TexturePassThroughParameter",
             "CNA::Content::Pipeline::TextureBuildFormat",
             "CNA::Content::Pipeline::TryParseTextureBuildFormat",
             "CNA::Content::Pipeline::TextureBuildFormatName",
             "CNA::Content::Pipeline::TextureBlockEncoder",
             "CNA::Content::Pipeline::NextPowerOfTwoDimension",
             "CNA::Content::Pipeline::ResampleRgbaImage",
             "CNA::Content::Pipeline::PremultiplyRgbaAlpha",
             "CNA::Content::Pipeline::GenerateRgbaMipChain",
             "CNA::Content::Pipeline::ImportedImage",
             "CNA::Content::Pipeline::DecodeImportedImage",
             "CNA::Content::Pipeline::ApplyPngFileGammaEXT",
             "CNA::Content::Pipeline::BuildCnbTexture2DData",
             "CNA::Content::Pipeline::ImageImporter::Identity",
             "CNA::Content::Pipeline::ImageImporter::SourceExtensions",
             "CNA::Content::Pipeline::ImageImporter::OutputTypes",
             "CNA::Content::Pipeline::ImageImporter::Import",
             "CNA::Content::Pipeline::TextureProcessor::TextureProcessor",
             "CNA::Content::Pipeline::TextureProcessor::Identity",
             "CNA::Content::Pipeline::TextureProcessor::InputType",
             "CNA::Content::Pipeline::TextureProcessor::OutputType",
             "CNA::Content::Pipeline::TextureProcessor::ValidateParameters",
             "CNA::Content::Pipeline::TextureProcessor::Process",
             "CNA::Content::Pipeline::Texture2DContentWriter::Identity",
             "CNA::Content::Pipeline::Texture2DContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::Texture2DContentWriter::InputType",
             "CNA::Content::Pipeline::Texture2DContentWriter::Write",
             "CNA::Content::Pipeline::RegisterTexture2DContentPipeline",
             "CNA::Content::Cnb::DecodeTexture2DFromCnb"},
    .tags = {"content", "pipeline", "texture", "image", "mipmap", "compression", "cnaext"},
    .assets = {"textures/checker.png", "textures/striped_mips.dds"},
    .restrictions = "Headless CPU build and exact byte/data checks. The injected encoder uses recognizable probe blocks to expose its contract; those blocks are not loaded or rendered. Existing texture demos own real compression and visual evidence.",
})
