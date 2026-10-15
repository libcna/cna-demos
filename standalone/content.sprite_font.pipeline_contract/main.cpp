// SPDX-License-Identifier: MS-PL
// Audit the public SpriteFont source pipeline through a real OFL TrueType face and a font sheet.
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbSpriteFontCodec.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/SpriteFontContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnbOutputContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
namespace Graphics = Microsoft::Xna::Framework::Graphics;
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

    class FontDirectoryGuard
    {
    public:
        FontDirectoryGuard()
            : original_(Pipeline::FontSearchDirectoriesEXT())
        {
        }

        ~FontDirectoryGuard()
        {
            Pipeline::SetFontSearchDirectoriesEXT(std::move(original_));
        }

        FontDirectoryGuard(const FontDirectoryGuard&) = delete;
        FontDirectoryGuard& operator=(const FontDirectoryGuard&) = delete;

    private:
        std::vector<std::filesystem::path> original_;
    };

    std::string ReadText(const std::filesystem::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) throw std::runtime_error("Could not read the SpriteFont fixture");
        return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    }

    bool SameTexture(const Cnb::CnbTextureData& left, const Cnb::CnbTextureData& right)
    {
        if (left.width != right.width || left.height != right.height ||
            left.depth != right.depth || left.faceCount != right.faceCount ||
            left.mipCount != right.mipCount ||
            left.representations.size() != right.representations.size())
        {
            return false;
        }
        for (std::size_t index = 0u; index < left.representations.size(); ++index)
        {
            if (left.representations[index].format != right.representations[index].format ||
                left.representations[index].levels != right.representations[index].levels)
            {
                return false;
            }
        }
        return true;
    }

    bool SameFont(const Cnb::CnbSpriteFontData& left, const Cnb::CnbSpriteFontData& right)
    {
        return SameTexture(left.atlas, right.atlas) &&
            left.glyphBounds == right.glyphBounds && left.cropping == right.cropping &&
            left.kerning == right.kerning && left.characters == right.characters &&
            left.lineSpacing == right.lineSpacing && left.spacing == right.spacing &&
            left.defaultCharacter == right.defaultCharacter;
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

    std::vector<std::uint8_t> MakeSheet(const std::uint32_t count,
                                        const std::uint32_t side,
                                        std::uint32_t& width,
                                        std::uint32_t& height,
                                        const std::array<std::uint8_t, 4> ink =
                                            {255u, 255u, 255u, 255u})
    {
        width = count * (side + 1u) + 1u;
        height = side + 2u;
        std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width) * height * 4u);
        for (std::size_t at = 0u; at < rgba.size(); at += 4u)
        {
            rgba[at] = 255u;
            rgba[at + 1u] = 0u;
            rgba[at + 2u] = 255u;
            rgba[at + 3u] = 255u;
        }
        for (std::uint32_t cell = 0u; cell < count; ++cell)
        {
            for (std::uint32_t y = 0u; y < side; ++y)
            {
                for (std::uint32_t x = 0u; x < side; ++x)
                {
                    const std::size_t at =
                        ((static_cast<std::size_t>(y + 1u) * width) +
                         cell * (side + 1u) + 1u + x) * 4u;
                    std::copy(ink.begin(), ink.end(), rgba.begin() +
                        static_cast<std::ptrdiff_t>(at));
                }
            }
        }
        return rgba;
    }

    std::uint32_t RoundUpPowerOfTwo(std::uint32_t value)
    {
        std::uint32_t rounded = 1u;
        while (rounded < value) rounded <<= 1u;
        return rounded;
    }

    int Run()
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };
        if (!Pipeline::IsFontRasterizationAvailable())
        {
            std::cout << "SKIP: CNA was built without the optional FreeType SpriteFont rasterizer\n";
            return 77;
        }

        FontDirectoryGuard restoreDirectories;
        Pipeline::SetFontSearchDirectoriesEXT({});

        const Pipeline::FontDescription defaults;
        const Pipeline::FontCharacterRegion emptyRegion;
        check(Pipeline::ImportedFontDescriptionType ==
                          std::string("CNA.Content.Pipeline.ImportedFontDescription") &&
                      defaults.fontName.empty() && defaults.size == 12.0f &&
                      defaults.spacing == 0.0f && defaults.useKerning &&
                      defaults.style == Pipeline::FontDescriptionStyle::Regular &&
                      !defaults.defaultCharacter.has_value() &&
                      defaults.characterRegions.empty() && defaults.resolvedFontFile.empty() &&
                      !defaults.resolvedFromSystemFonts && emptyRegion.start == u'\0' &&
                      emptyRegion.end == u'\0' &&
                      emptyRegion == Pipeline::FontCharacterRegion{} &&
                      static_cast<int>(Pipeline::FontDescriptionStyle::Regular) == 0 &&
                      static_cast<int>(Pipeline::FontDescriptionStyle::Bold) == 1 &&
                      static_cast<int>(Pipeline::FontDescriptionStyle::Italic) == 2 &&
                      static_cast<int>(Pipeline::FontDescriptionStyle::BoldItalic) == 3,
                  "font descriptions, character regions and all four style values have stable safe defaults");

        const std::filesystem::path assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        const std::filesystem::path fontDirectory = assetRoot / "fonts/liberation_mono";
        const std::filesystem::path source = fontDirectory / "pipeline.spritefont";
        const std::filesystem::path fontFile = fontDirectory / "LiberationMono-Regular.ttf";
        const std::string xml = ReadText(source);

        const Pipeline::FontDescription parsed =
            Pipeline::ParseFontDescription(xml, source.generic_string());
        Pipeline::FontDescription expected;
        expected.fontName = "LiberationMono-Regular";
        expected.size = 14.0f;
        expected.spacing = 1.5f;
        expected.useKerning = true;
        expected.style = Pipeline::FontDescriptionStyle::Regular;
        expected.defaultCharacter = u'?';
        expected.characterRegions = {{u'a', u'c'}, {u'A', u'D'}};
        check(parsed == expected && parsed.characterRegions[0] != parsed.characterRegions[1],
                  "the XNA XML parser restores every authored field and preserves region order");

        Pipeline::FontDescription overlapping = parsed;
        overlapping.characterRegions.push_back({u'B', u'b'});
        const std::vector<SharpRuntime::charcs> expanded =
            Pipeline::ExpandCharacterRegions(overlapping);
        check(expanded == std::vector<SharpRuntime::charcs>{
                                  u'?', u'A', u'B', u'C', u'D', u'E', u'F', u'G', u'H', u'I',
                                  u'J', u'K', u'L', u'M', u'N', u'O', u'P', u'Q', u'R', u'S',
                                  u'T', u'U', u'V', u'W', u'X', u'Y', u'Z', u'[', u'\\', u']',
                                  u'^', u'_', u'`', u'a', u'b', u'c'},
                  "region expansion sorts, deduplicates overlaps and inserts an out-of-range fallback");

        Pipeline::FontDescription inverted = parsed;
        inverted.defaultCharacter.reset();
        inverted.characterRegions = {{u'Z', u'A'}};
        Pipeline::FontDescription oversized = parsed;
        oversized.defaultCharacter.reset();
        oversized.characterRegions = {{u'\0', static_cast<SharpRuntime::charcs>(0xFFFFu)}};
        check(Refuses([&] { (void)Pipeline::ParseFontDescription("<broken>", "memory"); }),
              "malformed SpriteFont XML is refused before import");
        const std::vector<SharpRuntime::charcs> invertedExpansion =
            Pipeline::ExpandCharacterRegions(inverted);
        check(invertedExpansion.empty(),
              "an inverted programmatic range currently disappears silently (CNA-DEMOS-ISSUE-108)");
        check(Refuses([&] { (void)Pipeline::ExpandCharacterRegions(oversized); }),
              "a programmatic character set above the 8192-glyph limit is refused");

        const Pipeline::FontDescriptionImporter unconfiguredImporter;
        check(unconfiguredImporter.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.FontDescriptionImporter", "1"} &&
                      unconfiguredImporter.SourceExtensions() ==
                          std::vector<std::string>{".spritefont"} &&
                      unconfiguredImporter.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedFontDescriptionType} &&
                      !unconfiguredImporter.SelectedByNameOnly(),
                  "the ordinary .spritefont importer publishes its exact route and unconfigured identity");

        Pipeline::SetFontSearchDirectoriesEXT({fontDirectory});
        const Pipeline::FontDescriptionImporter importer;
        const std::filesystem::path beside = Pipeline::FindFontFamilyBeside(
            fontDirectory, "Liberation Mono", Pipeline::FontDescriptionStyle::Regular);
        const std::filesystem::path searched = Pipeline::FindSystemFontFile(
            "Liberation Mono", Pipeline::FontDescriptionStyle::Regular);
        std::error_code equivalentError;
        const bool besideIsFixture = !beside.empty() &&
            std::filesystem::equivalent(beside, fontFile, equivalentError) && !equivalentError;
        equivalentError.clear();
        const bool searchedIsFixture = !searched.empty() &&
            std::filesystem::equivalent(searched, fontFile, equivalentError) && !equivalentError;
        check(Pipeline::FontSearchDirectoriesEXT() ==
                          std::vector<std::filesystem::path>{fontDirectory} &&
                      importer.Identity() == Pipeline::ContentComponentIdentity{
                          "CNA.FontDescriptionImporter",
                          "1+fonts:" + fontDirectory.generic_string()} && besideIsFixture &&
                      searchedIsFixture &&
                      Pipeline::FindSystemFontFile("No Such CNA Font Family").empty(),
                  "configured and beside-family searches resolve the exact OFL face and participate in importer identity");

        const Pipeline::FontDescriptionProcessor processor;
        check(processor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.FontDescriptionProcessor", "1"} &&
                      processor.InputType() == Pipeline::ImportedFontDescriptionType &&
                      processor.OutputType() == Pipeline::ProcessedSpriteFontType &&
                      !processor.SelectedByNameOnly(),
                  "the description processor joins the parsed description to canonical SpriteFont data");
        Pipeline::ContentProcessorParameters unknownDescriptionParameter;
        unknownDescriptionParameter.Set("antialias", true);
        bool descriptionParametersExact = true;
        try { processor.ValidateParameters({}); }
        catch (...) { descriptionParametersExact = false; }
        try
        {
            processor.ValidateParameters(unknownDescriptionParameter);
            descriptionParametersExact = false;
        }
        catch (const Pipeline::ContentParameterError& error)
        {
            descriptionParametersExact = descriptionParametersExact &&
                error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "antialias";
        }
        check(descriptionParametersExact,
                  "the description processor accepts no parameter because the XML carries its complete policy");

        Pipeline::FontDescription resolved = parsed;
        resolved.resolvedFontFile = fontFile;
        std::vector<std::string> warnings;
        const Cnb::CnbSpriteFontData rasterized =
            Pipeline::RasterizeFontDescription(resolved, warnings,
                                               Pipeline::ContentStrictness::Strict,
                                               Graphics::GraphicsProfile::HiDef);
        const std::vector<SharpRuntime::charcs> expectedCharacters{
            u'?', u'A', u'B', u'C', u'D', u'a', u'b', u'c'};
        check(warnings.empty() && rasterized.characters == expectedCharacters &&
                      rasterized.glyphBounds.size() == expectedCharacters.size() &&
                      rasterized.cropping.size() == expectedCharacters.size() &&
                      rasterized.kerning.size() == expectedCharacters.size() &&
                      rasterized.lineSpacing > 0 && rasterized.spacing == 1.5f &&
                      rasterized.defaultCharacter == u'?' && rasterized.atlas.width > 0u &&
                      rasterized.atlas.height > 0u &&
                      rasterized.atlas.representations.size() == 1u &&
                      rasterized.atlas.representations[0].format == Cnb::CnbTextureFormat::Rgba8,
                  "the real TrueType face rasterizes every sorted glyph into one lossless atlas with authored metrics");

        bool glyphGeometry = true;
        for (std::size_t index = 0u; index < rasterized.characters.size(); ++index)
        {
            const auto& glyph = rasterized.glyphBounds[index];
            const auto& crop = rasterized.cropping[index];
            const auto& kerning = rasterized.kerning[index];
            glyphGeometry = glyphGeometry && glyph.Width > 0 && glyph.Height > 0 &&
                glyph.X >= 0 && glyph.Y >= 0 && crop.Width == glyph.Width && crop.Y >= 0 &&
                kerning.Y == static_cast<float>(glyph.Width) &&
                kerning.X + kerning.Y + kerning.Z >= 0.0f &&
                static_cast<std::uint32_t>(glyph.X + glyph.Width) <= rasterized.atlas.width &&
                static_cast<std::uint32_t>(glyph.Y + glyph.Height) <= rasterized.atlas.height;
        }
        check(glyphGeometry,
                  "every rasterized glyph exposes bounded atlas geometry, matching crop width and a nonnegative ABC advance");

        Pipeline::FontDescription missingGlyph = resolved;
        missingGlyph.defaultCharacter.reset();
        missingGlyph.characterRegions = {{static_cast<SharpRuntime::charcs>(0x4E00u),
                                          static_cast<SharpRuntime::charcs>(0x4E00u)}};
        const bool strictMissing = Refuses([&]
        {
            std::vector<std::string> ignored;
            (void)Pipeline::RasterizeFontDescription(
                missingGlyph, ignored, Pipeline::ContentStrictness::Strict);
        });
        std::vector<std::string> compatibleWarnings;
        const Cnb::CnbSpriteFontData compatibleMissing = Pipeline::RasterizeFontDescription(
            missingGlyph, compatibleWarnings, Pipeline::ContentStrictness::XnaCompatible);
        check(strictMissing && compatibleMissing.characters.size() == 1u &&
                      !compatibleWarnings.empty(),
                  "strict rasterization refuses a missing glyph while XNA-compatible mode keeps its notdef glyph and warning");

        Pipeline::FontDescription synthesized = resolved;
        synthesized.style = Pipeline::FontDescriptionStyle::BoldItalic;
        std::vector<std::string> styleWarnings;
        const Cnb::CnbSpriteFontData styled =
            Pipeline::RasterizeFontDescription(synthesized, styleWarnings);
        check(styled.characters == expectedCharacters && !styleWarnings.empty() &&
                      std::any_of(styleWarnings.begin(), styleWarnings.end(),
                                  [](const std::string& warning)
                                  { return warning.find("synthesized") != std::string::npos; }),
                  "a missing bold-italic face is synthesized explicitly and reported instead of masquerading as authored");

        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSpriteFontSourceContentPipeline(*registry);
        Pipeline::RegisterCnjContentPipeline(*registry);
        Pipeline::RegisterXnbOutputContentPipeline(*registry, {});
        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = assetRoot;
        request.source = "fonts/liberation_mono/pipeline.spritefont";
        request.logicalName = "Fonts/pipeline-contract";
        request.environment.targetProfile = Graphics::GraphicsProfile::HiDef;

        Pipeline::ContentDependencyCollector dependencies;
        const Pipeline::ContentProcessResult processed =
            pipeline.ImportAndProcess(request, dependencies);
        const Cnb::CnbSpriteFontData& processedFont =
            processed.processed.Get<Cnb::CnbSpriteFontData>();
        const std::vector<Pipeline::ContentDependency> processDependencies =
            dependencies.Dependencies();
        check(processed.importer == importer.Identity() &&
                      processed.processor == processor.Identity() &&
                      SameFont(processedFont, rasterized) && processDependencies.size() == 2u &&
                      std::any_of(processDependencies.begin(), processDependencies.end(),
                                  [](const Pipeline::ContentDependency& dependency)
                                  {
                                      return dependency.kind ==
                                                 Pipeline::ContentDependencyKind::SourceFile &&
                                          dependency.identity.ends_with(
                                              "LiberationMono-Regular.ttf");
                                  }),
                  "registered import and processing resolve the project-local font, record it as a dependency and reproduce direct rasterization");

        const Pipeline::ContentBuildResult built = pipeline.Build(request);
        const Pipeline::ContentBuildResult repeated = pipeline.Build(request);
        const Cnb::CnbSpriteFontData decoded = Cnb::DecodeSpriteFontFromCnb(
            Cnb::CnbDocument::Parse(built.output.bytes, "pipeline SpriteFont CNB"));
        check(built.writer ==
                          Pipeline::ContentComponentIdentity{"CNA.SpriteFontContentWriter", "1"} &&
                      built.output.assetTypeId == Cnb::CnbAssetTypeId::SpriteFont &&
                      built.output.assetSchemaVersion == Cnb::CnbSpriteFontSchemaVersion &&
                      built.output.assetTypeName ==
                          "Microsoft.Xna.Framework.Graphics.SpriteFont" &&
                      SameFont(decoded, processedFont) && built.output.bytes == repeated.output.bytes &&
                      built.dependencies == repeated.dependencies,
                  "the complete CNB build is byte-deterministic and restores the exact processed atlas and tables");

        request.outputFormat = Pipeline::ContentOutputFormat::Xnb;
        Pipeline::ContentDependencyCollector xnbDependencies;
        const Pipeline::ContentProcessResult xnbProcessed =
            pipeline.ImportAndProcess(request, xnbDependencies);
        const Cnb::CnbSpriteFontData& xnbFont =
            xnbProcessed.processed.Get<Cnb::CnbSpriteFontData>();
        const Pipeline::ContentBuildResult xnb = pipeline.Build(request);
        check(xnbFont.characters == expectedCharacters &&
                      xnbFont.atlas.representations.size() == 1u &&
                      xnbFont.atlas.representations[0].format == Cnb::CnbTextureFormat::Bc2 &&
                      xnb.writer.name == "CNA.XnbSpriteFontWriter" &&
                      xnb.output.rootReaderName ==
                          "Microsoft.Xna.Framework.Content.SpriteFontReader" &&
                      xnb.output.bytes.size() >= 3u && xnb.output.bytes[0] == 'X' &&
                      xnb.output.bytes[1] == 'N' && xnb.output.bytes[2] == 'B',
                  "the XNB target converts the same glyphs to the measured DXT3 atlas and SpriteFontReader root");

        const Pipeline::FontTextureProcessor sheetProcessor;
        check(Pipeline::FontTextureFirstCharacterParameter ==
                          std::string("firstCharacter") &&
                      sheetProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.FontTextureProcessor", "1"} &&
                      sheetProcessor.InputType() == Pipeline::ImportedImageType &&
                      sheetProcessor.OutputType() == Pipeline::ProcessedSpriteFontType &&
                      sheetProcessor.SelectedByNameOnly(),
                  "the font-sheet processor publishes its exact named-only component and value identities");

        Pipeline::ContentProcessorParameters sheetParameters;
        sheetParameters.Set(Pipeline::FontTextureFirstCharacterParameter, std::string("A"));
        sheetParameters.Set(Pipeline::TexturePremultiplyAlphaParameter, false);
        sheetParameters.Set(Pipeline::TextureFormatParameter, std::string("Color"));
        bool sheetValidation = true;
        try { sheetProcessor.ValidateParameters(sheetParameters); }
        catch (...) { sheetValidation = false; }
        Pipeline::ContentProcessorParameters unknownSheetParameter;
        unknownSheetParameter.Set("glyphCount", std::int64_t{2});
        try
        {
            sheetProcessor.ValidateParameters(unknownSheetParameter);
            sheetValidation = false;
        }
        catch (const Pipeline::ContentParameterError& error)
        {
            sheetValidation = sheetValidation &&
                error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "glyphCount";
        }
        check(sheetValidation,
                  "font-sheet validation accepts exactly first character, premultiplication and texture format");

        std::uint32_t sheetWidth = 0u;
        std::uint32_t sheetHeight = 0u;
        const std::vector<std::uint8_t> manySheet =
            MakeSheet(40u, 7u, sheetWidth, sheetHeight);
        const Cnb::CnbSpriteFontData hiDefSheet = Pipeline::BuildFontFromTextureSheet(
            sheetWidth, sheetHeight, manySheet, u' ', "forty-glyph sheet",
            Graphics::GraphicsProfile::HiDef);
        const Cnb::CnbSpriteFontData reachSheet = Pipeline::BuildFontFromTextureSheet(
            sheetWidth, sheetHeight, manySheet, u' ', "forty-glyph sheet",
            Graphics::GraphicsProfile::Reach);
        std::uint32_t usedHeight = 0u;
        for (const auto& bounds : hiDefSheet.glyphBounds)
        {
            usedHeight = std::max(usedHeight,
                                  static_cast<std::uint32_t>(bounds.Y + bounds.Height));
        }
        ++usedHeight;
        check(hiDefSheet.characters.size() == 40u &&
                      hiDefSheet.characters.front() == u' ' &&
                      hiDefSheet.characters.back() == u'G' &&
                      hiDefSheet.lineSpacing == 7 && hiDefSheet.spacing == 0.0f &&
                      !hiDefSheet.defaultCharacter.has_value() &&
                      hiDefSheet.atlas.width == reachSheet.atlas.width &&
                      hiDefSheet.atlas.height == ((usedHeight + 3u) & ~3u) &&
                      reachSheet.atlas.height == RoundUpPowerOfTwo(usedHeight) &&
                      hiDefSheet.atlas.height != reachSheet.atlas.height,
                  "font-sheet packing derives sequential characters and applies distinct HiDef and Reach height rules");

        std::uint32_t smallWidth = 0u;
        std::uint32_t smallHeight = 0u;
        Pipeline::ImportedImage smallSheet;
        smallSheet.rgbaPixels = MakeSheet(2u, 2u, smallWidth, smallHeight,
                                          {200u, 100u, 50u, 128u});
        smallSheet.width = smallWidth;
        smallSheet.height = smallHeight;
        const Pipeline::ContentValue smallValue =
            Pipeline::ContentValue::Create(Pipeline::ImportedImageType, smallSheet);
        Pipeline::ContentSourceRootCapabilities externalRoots;
        Pipeline::ContentDependencyCollector sheetDependencies;
        RecordingLogger rawLogger;
        Pipeline::ContentBuildEnvironment sheetEnvironment;
        sheetEnvironment.targetProfile = Graphics::GraphicsProfile::HiDef;
        Pipeline::ContentProcessorContext rawContext(
            assetRoot, source, "Fonts/raw-sheet", sheetProcessor.Identity().name,
            sheetParameters, externalRoots, sheetDependencies, rawLogger,
            Pipeline::ContentOutputFormat::Cnb, sheetEnvironment);
        const Pipeline::ContentValue rawValue = sheetProcessor.Process(smallValue, rawContext);
        const Cnb::CnbSpriteFontData& rawSheet =
            rawValue.Get<Cnb::CnbSpriteFontData>();
        const auto rawBounds = rawSheet.glyphBounds.front();
        const std::size_t rawAt =
            (static_cast<std::size_t>(rawBounds.Y) * rawSheet.atlas.width + rawBounds.X) * 4u;

        Pipeline::ContentProcessorParameters premultipliedParameters = sheetParameters;
        premultipliedParameters.Set(Pipeline::TexturePremultiplyAlphaParameter, true);
        RecordingLogger premultipliedLogger;
        Pipeline::ContentProcessorContext premultipliedContext(
            assetRoot, source, "Fonts/premultiplied-sheet", sheetProcessor.Identity().name,
            premultipliedParameters, externalRoots, sheetDependencies, premultipliedLogger,
            Pipeline::ContentOutputFormat::Cnb, sheetEnvironment);
        const Pipeline::ContentValue premultipliedValue =
            sheetProcessor.Process(smallValue, premultipliedContext);
        const Cnb::CnbSpriteFontData& premultipliedSheet =
            premultipliedValue.Get<Cnb::CnbSpriteFontData>();
        const auto& rawPixels = rawSheet.atlas.representations[0].levels[0];
        const auto& premultipliedPixels =
            premultipliedSheet.atlas.representations[0].levels[0];
        check(rawSheet.characters == std::vector<SharpRuntime::charcs>{u'A', u'B'} &&
                      rawPixels[rawAt] == 200u && rawPixels[rawAt + 1u] == 100u &&
                      rawPixels[rawAt + 2u] == 50u && rawPixels[rawAt + 3u] == 128u &&
                      premultipliedPixels[rawAt] == 100u &&
                      premultipliedPixels[rawAt + 1u] == 50u &&
                      premultipliedPixels[rawAt + 2u] == 25u &&
                      premultipliedPixels[rawAt + 3u] == 128u &&
                      !rawLogger.messages.empty() && !premultipliedLogger.messages.empty(),
                  "the processor consumes its explicit first character and applies optional alpha premultiplication exactly");

        Pipeline::ContentProcessorParameters compressedParameters = sheetParameters;
        compressedParameters.Set(Pipeline::FontTextureFirstCharacterParameter,
                                 std::int64_t{65});
        compressedParameters.Set(Pipeline::TextureFormatParameter,
                                 std::string("DxtCompressed"));
        RecordingLogger compressedLogger;
        Pipeline::ContentProcessorContext compressedContext(
            assetRoot, source, "Fonts/compressed-sheet", sheetProcessor.Identity().name,
            compressedParameters, externalRoots, sheetDependencies, compressedLogger,
            Pipeline::ContentOutputFormat::Xnb, sheetEnvironment);
        const Pipeline::ContentValue compressedValue =
            sheetProcessor.Process(smallValue, compressedContext);
        const Cnb::CnbSpriteFontData& compressedSheet =
            compressedValue.Get<Cnb::CnbSpriteFontData>();
        const std::size_t compressedByteCount =
            static_cast<std::size_t>((compressedSheet.atlas.width + 3u) / 4u) *
            ((compressedSheet.atlas.height + 3u) / 4u) * 16u;
        check(compressedSheet.characters.front() == u'A' &&
                      compressedSheet.atlas.representations.size() == 1u &&
                      compressedSheet.atlas.representations[0].format ==
                          Cnb::CnbTextureFormat::Bc2 &&
                      compressedSheet.atlas.representations[0].levels[0].size() ==
                          compressedByteCount,
                  "an integer first-character value and DxtCompressed XNB request produce the exact canonical DXT3 block grid");

        Pipeline::ContentProcessorParameters emptyCharacter = sheetParameters;
        emptyCharacter.Set(Pipeline::FontTextureFirstCharacterParameter, std::string());
        RecordingLogger refusalLogger;
        Pipeline::ContentProcessorContext refusalContext(
            assetRoot, source, "Fonts/refused-sheet", sheetProcessor.Identity().name,
            emptyCharacter, externalRoots, sheetDependencies, refusalLogger,
            Pipeline::ContentOutputFormat::Cnb, sheetEnvironment);
        const Pipeline::ContentValue emptySheet = Pipeline::ContentValue::Create(
            Pipeline::ImportedImageType, Pipeline::ImportedImage{});
        check(Refuses([&] { (void)sheetProcessor.Process(smallValue, refusalContext); }) &&
                      Refuses([&]
                      {
                          (void)Pipeline::BuildFontFromTextureSheet(
                              0u, 0u, {}, u' ', "empty sheet");
                      }) &&
                      Refuses([&]
                      {
                          Pipeline::ContentProcessorParameters ordinary;
                          RecordingLogger ignoredLogger;
                          Pipeline::ContentProcessorContext emptyContext(
                              assetRoot, source, "Fonts/empty-sheet",
                              sheetProcessor.Identity().name, ordinary, externalRoots,
                              sheetDependencies, ignoredLogger);
                          (void)sheetProcessor.Process(emptySheet, emptyContext);
                      }),
                  "an empty first character and glyphless direct or processed sheets are refused before output");

        std::cout << "Rasterized " << rasterized.characters.size()
                  << " TrueType glyphs and packed " << hiDefSheet.characters.size()
                  << " synthetic sheet glyphs.\n";
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
        std::cerr << "SpriteFont pipeline audit failed: " << error.what() << '\n';
        return 1;
    }
}
