// SPDX-License-Identifier: MS-PL
// A SpriteFont authored as CNJ: an artist's glyph atlas plus a JSON document that names each
// glyph's rectangle, cropping and kerning, compiled by CNA's content pipeline.
#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "CNA/Content/Cnb/CnbSpriteFontCodec.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Cnb = CNA::Content::Cnb;
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Lcd(90, 230, 120);
    constexpr float Scale = 3.0f;

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
    bool RefusesContent(Function&& function)
    {
        try
        {
            function();
            return false;
        }
        catch (const Content::ContentLoadException&)
        {
            return true;
        }
    }

    class CnjSpriteFontDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            // content/fonts/score_digits.cnj names score_digits_atlas.png and, per glyph, its
            // source rectangle, cropping and kerning (tools/generate_assets.py writes both). The
            // pipeline compiles it to fonts/score_digits.cnb; the atlas also builds as a texture.
            digits_.emplace(getContentProperty().Load<SpriteFont>("fonts/score_digits"));
            atlas_.emplace(getContentProperty().Load<Texture2D>("fonts/score_digits_atlas"));

            const auto& characters = digits_->getCharactersProperty();
            ctx.Print("{} characters, LineSpacing {}, Spacing {}", characters.size(),
                      digits_->getLineSpacingProperty(), digits_->getSpacingProperty());
            ctx.Check(characters.size() == 11 && characters.front() == u'0' && characters.back() == u':',
                      "the font has the eleven authored characters, 0-9 and ':'");
            ctx.Check(digits_->getLineSpacingProperty() == 36, "LineSpacing is the authored 36");
            ctx.Check(digits_->getDefaultCharacterProperty() == u'0', "DefaultCharacter is the authored '0'");

            // A digit advances 2 + 20 + 2 pixels (left bearing, width, right bearing), the colon 2 + 4 + 2.
            const Vector2 size = digits_->MeasureString("12:34");
            ctx.Print("MeasureString(\"12:34\") = {}", size.ToString());
            ctx.Check(size.X == 4.0f * 24.0f + 8.0f && size.Y == 36.0f,
                      "MeasureString adds the authored kerning: 104 x 36");
            ctx.Check(digits_->MeasureString("A") == digits_->MeasureString("0"),
                      "a character outside the font measures as the DefaultCharacter");

            ctx.Check(Cnb::CnbSpriteFontChunk::Header == Cnb::MakeChunkId('F', 'O', 'N', 'T') &&
                          Cnb::CnbSpriteFontChunk::GlyphBounds ==
                              Cnb::MakeChunkId('G', 'L', 'Y', 'P') &&
                          Cnb::CnbSpriteFontChunk::Cropping ==
                              Cnb::MakeChunkId('C', 'R', 'O', 'P') &&
                          Cnb::CnbSpriteFontChunk::Kerning ==
                              Cnb::MakeChunkId('K', 'E', 'R', 'N') &&
                          Cnb::CnbSpriteFontChunk::Characters ==
                              Cnb::MakeChunkId('C', 'H', 'A', 'R') &&
                          Cnb::CnbSpriteFontSchemaVersion == 1u &&
                          Cnb::CnbSpriteFontHeaderStride == 24u &&
                          Cnb::CnbSpriteFontRectangleStride == 16u &&
                          Cnb::CnbSpriteFontKerningStride == 12u &&
                          Cnb::CnbSpriteFontCharacterStride == 4u &&
                          Cnb::CnbMaxSpriteFontGlyphs == 65536u,
                      "the SpriteFont codec publishes its five chunks, schema, table strides and glyph ceiling exactly");

            const Cnb::CnbSpriteFontData safeFont;
            ctx.Check(safeFont.atlas.width == 0u && safeFont.atlas.height == 0u &&
                          safeFont.atlas.depth == 1u && safeFont.atlas.faceCount == 1u &&
                          safeFont.atlas.mipCount == 1u && safeFont.atlas.representations.empty() &&
                          safeFont.glyphBounds.empty() && safeFont.cropping.empty() &&
                          safeFont.kerning.empty() && safeFont.characters.empty() &&
                          safeFont.lineSpacing == 0 && safeFont.spacing == 0.0f &&
                          !safeFont.defaultCharacter.has_value(),
                      "a decoded SpriteFont carrier begins with safe empty atlas, table and metric values");

            const std::filesystem::path assetRoot =
                std::filesystem::path(TitleLocation::Path()) / "assets/fonts";
            const Cnb::CnbDocument document =
                Cnb::CnbDocument::ParseFile((assetRoot / "score_digits.cnb").string());
            const Cnb::CnbSpriteFontData compiled = Cnb::DecodeSpriteFontFromCnb(document);
            ctx.Check(document.AssetTypeId() == Cnb::CnbAssetTypeId::SpriteFont &&
                          document.AssetSchemaVersion() == Cnb::CnbSpriteFontSchemaVersion &&
                          document.Metadata().assetTypeName ==
                              "Microsoft.Xna.Framework.Graphics.SpriteFont" &&
                          document.Metadata().contentName == "fonts/score_digits" &&
                          document.FindAll(Cnb::CnbSpriteFontChunk::Header).size() == 1u &&
                          document.FindAll(Cnb::CnbSpriteFontChunk::GlyphBounds).size() == 1u &&
                          document.FindAll(Cnb::CnbSpriteFontChunk::Cropping).size() == 1u &&
                          document.FindAll(Cnb::CnbSpriteFontChunk::Kerning).size() == 1u &&
                          document.FindAll(Cnb::CnbSpriteFontChunk::Characters).size() == 1u,
                      "the real compiled font has the exact SpriteFont identity and one of every mandatory table");

            bool exactRows = compiled.characters.size() == 11u &&
                compiled.glyphBounds.size() == 11u && compiled.cropping.size() == 11u &&
                compiled.kerning.size() == 11u;
            for (std::size_t index = 0u; exactRows && index < 11u; ++index)
            {
                const bool colon = index == 10u;
                const int width = colon ? 4 : 20;
                const int x = colon ? 200 : static_cast<int>(index * 20u);
                const SharpRuntime::charcs character =
                    colon ? u':' : static_cast<SharpRuntime::charcs>(u'0' + index);
                exactRows = compiled.characters[index] == character &&
                    compiled.glyphBounds[index] == Rectangle(x, 0, width, 32) &&
                    compiled.cropping[index] == Rectangle(0, 0, width, 32) &&
                    compiled.kerning[index] == Vector3(2.0f, static_cast<float>(width), 2.0f);
            }
            ctx.Check(exactRows && compiled.lineSpacing == 36 && compiled.spacing == 0.0f &&
                          compiled.defaultCharacter == u'0',
                      "decoding restores every authored character, rectangle, crop, kerning row and whole-font metric");

            const Cnb::CnbTextureData standaloneAtlas = Cnb::DecodeTexture2DFromCnb(
                Cnb::CnbDocument::ParseFile((assetRoot / "score_digits_atlas.cnb").string()));
            ctx.Check(compiled.atlas.width == 204u && compiled.atlas.height == 32u &&
                          compiled.atlas.depth == 1u && compiled.atlas.faceCount == 1u &&
                          SameTexture(compiled.atlas, standaloneAtlas),
                      "the embedded 204x32 atlas is byte-identical to the independently compiled Texture2D");

            const std::vector<std::uint8_t> encoded =
                Cnb::EncodeSpriteFontToCnb(compiled, "fonts/score_digits");
            const Cnb::CnbSpriteFontData roundTrip = Cnb::DecodeSpriteFontFromCnb(
                Cnb::CnbDocument::Parse(encoded, "score_digits round trip"));
            ctx.Check(SameFont(compiled, roundTrip) &&
                          encoded == Cnb::EncodeSpriteFontToCnb(compiled, "fonts/score_digits"),
                      "native SpriteFont encoding is deterministic and round-trips the atlas and all parallel tables exactly");

            Cnb::CnbSpriteFontData noDefault = compiled;
            noDefault.defaultCharacter.reset();
            const Cnb::CnbSpriteFontData noDefaultRoundTrip = Cnb::DecodeSpriteFontFromCnb(
                Cnb::CnbDocument::Parse(Cnb::EncodeSpriteFontToCnb(noDefault),
                                        "font without default"));
            ctx.Check(!noDefaultRoundTrip.defaultCharacter.has_value() &&
                          noDefaultRoundTrip.characters.front() == u'0',
                      "an absent default character remains distinct from the first encoded character");

            Cnb::CnbSpriteFontData shortTable = compiled;
            shortTable.cropping.pop_back();
            Cnb::CnbSpriteFontData unsorted = compiled;
            std::swap(unsorted.characters[0], unsorted.characters[1]);
            Cnb::CnbSpriteFontData absentDefault = compiled;
            absentDefault.defaultCharacter = u'?';
            Cnb::CnbSpriteFontData cubeAtlas = compiled;
            cubeAtlas.atlas.faceCount = Cnb::CnbTextureCubeFaceCount;
            ctx.Check(RefusesContent([&] { (void)Cnb::EncodeSpriteFontToCnb(shortTable); }) &&
                          RefusesContent([&] { (void)Cnb::EncodeSpriteFontToCnb(unsorted); }) &&
                          RefusesContent([&] { (void)Cnb::EncodeSpriteFontToCnb(absentDefault); }) &&
                          RefusesContent([&] { (void)Cnb::EncodeSpriteFontToCnb(cubeAtlas); }),
                      "encoding refuses a short table, an unsorted map, an absent fallback and a non-2D atlas");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(16, 20, 18));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // The atlas as the artist drew it, and the score drawn with the font, three times larger.
            spriteBatch_->Draw(*atlas_, Vector2(298.0f, 40.0f), Color::Gray);
            const Vector2 size = digits_->MeasureString("12:34") * Scale;
            spriteBatch_->DrawString(*digits_, "12:34", Vector2((800.0f - size.X) / 2.0f, 150.0f), Lcd, 0.0f,
                                     Vector2::Zero, Scale, SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*digits_, "0123456789", Vector2(282.0f, 320.0f), Lcd);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                // The '1' of "12:34": its right-hand segments (b, c) lit, its left side dark.
                const CnaDemos::BackBufferPixels p(device);
                const int left = static_cast<int>((800.0f - size.X) / 2.0f);
                const Color segmentB = p.At(left + static_cast<int>(Scale * (2 + 17)), 150 + static_cast<int>(Scale * 8));
                const Color unlit = p.At(left + static_cast<int>(Scale * (2 + 1)), 150 + static_cast<int>(Scale * 8));
                Context().Print("'1': segment b {}, left side {}", segmentB.ToString(), unlit.ToString());
                Context().Check(segmentB == Lcd, "the 1's right segment is drawn from the atlas glyph");
                Context().Check(unlit == Color(16, 20, 18), "and its left side is empty");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> digits_;
        std::optional<Texture2D> atlas_;
    };
}

CNA_DEMOS_GAME(CnjSpriteFontDemo, {
    .id = "content.cnj.sprite_font",
    .title = "Bitmap font from CNJ content",
    .description = "Loads a seven-segment SpriteFont authored as CNJ, audits its complete native CNB codec contract "
                   "and draws it; MeasureString follows the authored kerning and DefaultCharacter stands in for "
                   "missing characters.",
    .apis = {"ContentManager::Load", "SpriteFont::Characters", "SpriteFont::LineSpacing", "SpriteFont::DefaultCharacter",
             "SpriteFont::MeasureString", "SpriteBatch::DrawString",
             "CNA::Content::Cnb::CnbSpriteFontChunk::Header",
             "CNA::Content::Cnb::CnbSpriteFontChunk::GlyphBounds",
             "CNA::Content::Cnb::CnbSpriteFontChunk::Cropping",
             "CNA::Content::Cnb::CnbSpriteFontChunk::Kerning",
             "CNA::Content::Cnb::CnbSpriteFontChunk::Characters",
             "CNA::Content::Cnb::CnbSpriteFontSchemaVersion",
             "CNA::Content::Cnb::CnbSpriteFontHeaderStride",
             "CNA::Content::Cnb::CnbSpriteFontRectangleStride",
             "CNA::Content::Cnb::CnbSpriteFontKerningStride",
             "CNA::Content::Cnb::CnbSpriteFontCharacterStride",
             "CNA::Content::Cnb::CnbMaxSpriteFontGlyphs",
             "CNA::Content::Cnb::CnbSpriteFontData",
             "CNA::Content::Cnb::EncodeSpriteFontToCnb",
             "CNA::Content::Cnb::DecodeSpriteFontFromCnb"},
    .tags = {"content", "cnj", "json", "font", "bitmap-font", "score", "cnb", "codec"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/score_digits.cnb", "fonts/score_digits_atlas.cnb"},
})
