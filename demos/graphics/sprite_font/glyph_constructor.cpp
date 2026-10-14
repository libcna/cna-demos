// SPDX-License-Identifier: MS-PL
// CNAEXT SpriteFont constructor: build a font in code from a texture and glyph tables.
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A 3x5 font for the digits 0-9, one string of 15 cells per digit, row by row.
    constexpr std::array<const char*, 10> Digits = {
        "###" "#.#" "#.#" "#.#" "###", "..#" "..#" "..#" "..#" "..#", "###" "..#" "###" "#.." "###",
        "###" "..#" "###" "..#" "###", "#.#" "#.#" "###" "..#" "..#", "###" "#.." "###" "..#" "###",
        "###" "#.." "###" "#.#" "###", "###" "..#" "..#" "..#" "..#", "###" "#.#" "###" "#.#" "###",
        "###" "#.#" "###" "..#" "###",
    };
    constexpr int GlyphWidth = 3;
    constexpr int GlyphHeight = 5;

    class GlyphConstructorDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);

            // 1. The atlas: the ten digits side by side, white ink on transparent.
            constexpr int atlasWidth = GlyphWidth * 10;
            std::vector<Color> pixels(static_cast<std::size_t>(atlasWidth) * GlyphHeight, Color::Transparent);
            for (int digit = 0; digit < 10; ++digit)
                for (int y = 0; y < GlyphHeight; ++y)
                    for (int x = 0; x < GlyphWidth; ++x)
                        if (Digits[static_cast<std::size_t>(digit)][y * GlyphWidth + x] == '#')
                            pixels[static_cast<std::size_t>(y) * atlasWidth + digit * GlyphWidth + x] = Color::White;
            Texture2D atlas(device, atlasWidth, GlyphHeight);
            atlas.SetData(pixels.data(), static_cast<int>(pixels.size()));

            // 2. The tables a compiled font carries, one entry per character:
            //    glyph bounds -- where the glyph is in the atlas;
            //    cropping     -- where it sits in its cell (offset) and the cell size;
            //    kerning      -- (left bearing, glyph width, right bearing): the advance is their sum.
            std::vector<Rectangle> bounds;
            std::vector<Rectangle> cropping;
            std::vector<SharpRuntime::charcs> characters;
            std::vector<Vector3> kerning;
            for (int digit = 0; digit < 10; ++digit)
            {
                bounds.emplace_back(digit * GlyphWidth, 0, GlyphWidth, GlyphHeight);
                cropping.emplace_back(0, 0, GlyphWidth, GlyphHeight);
                characters.push_back(static_cast<SharpRuntime::charcs>(u'0' + digit));
                kerning.emplace_back(0.0f, static_cast<float>(GlyphWidth), 1.0f);
            }
            // Give '1' deliberately asymmetric layout metrics. Cropping is a draw offset/cell,
            // while kerning carries left bearing, ink width and right bearing.
            cropping[1] = Rectangle(-2, 1, GlyphWidth, GlyphHeight + 2);
            kerning[1] = Vector3(-1.0f, static_cast<float>(GlyphWidth), -2.0f);

            const std::vector<Rectangle> expectedBounds = bounds;
            const std::vector<Rectangle> expectedCropping = cropping;
            const std::vector<SharpRuntime::charcs> expectedCharacters = characters;
            const std::vector<Vector3> expectedKerning = kerning;
            font_.emplace(atlas, bounds, cropping, characters, GlyphHeight + 1, 0.0f, kerning, std::nullopt);

            // The constructor receives its tables by value. Changing the caller's vectors cannot
            // rewrite the font, and the four inspection extensions expose the retained layout.
            bounds[0] = Rectangle(99, 99, 99, 99);
            cropping[0] = Rectangle(99, 99, 99, 99);
            characters[0] = u'X';
            kerning[0] = Vector3(99.0f);
            std::vector<Color> retainedPixels(pixels.size());
            font_->getTextureEXT().GetData(retainedPixels.data(), static_cast<int>(retainedPixels.size()));
            Context().Check(font_->getTextureEXT().getWidthProperty() == atlasWidth &&
                                font_->getTextureEXT().getHeightProperty() == GlyphHeight &&
                                retainedPixels == pixels,
                            "the inspected atlas retains its exact dimensions and texels");
            Context().Check(font_->getGlyphBoundsEXT() == expectedBounds &&
                                font_->getCharactersProperty() == expectedCharacters,
                            "glyph bounds and characters preserve their common order");
            Context().Check(font_->getCroppingEXT() == expectedCropping &&
                                font_->getKerningEXT() == expectedKerning,
                            "asymmetric cropping and signed bearings round-trip exactly");

            const Vector2 size = font_->MeasureString("2026");
            Context().Print("MeasureString(\"2026\") = {}", size.ToString());
            Context().Check(size.X == 4.0f * (GlyphWidth + 1), "each digit advances 3 + 1 pixels");
            Context().Check(font_->getCharactersProperty().size() == 10, "the font has the ten digits");
            Context().Check(font_->MeasureString("1") == Vector2(3.0f, 7.0f),
                            "negative edge bearings do not expand a single glyph's measured width");
            Context().Check(font_->MeasureString("11") == Vector2(3.0f, 7.0f),
                            "signed interior bearings and the taller crop drive repeated-glyph layout");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "0123456789", Vector2(40.0f, 60.0f), Color::Lime, 0.0f, Vector2::Zero,
                                     12.0f, SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_, "2026", Vector2(40.0f, 200.0f), Color::Orange, 0.0f, Vector2::Zero, 24.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const auto& presentation = device.getPresentationParametersProperty();
                const int width = presentation.getBackBufferWidthProperty();
                const int height = presentation.getBackBufferHeightProperty();
                std::vector<Color> backBuffer(static_cast<std::size_t>(width) * height);
                device.GetBackBufferData(backBuffer.data(), static_cast<int>(backBuffer.size()));
                const Color shiftedInk = backBuffer[static_cast<std::size_t>(90) * width + 82];
                const Color aboveCrop = backBuffer[static_cast<std::size_t>(65) * width + 82];
                Context().Check(shiftedInk == Color::Lime && aboveCrop == Color::Black,
                                "signed bearings and cropping place the '1' ink at its audited offset");
            }
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(GlyphConstructorDemo, {
    .id = "graphics.sprite_font.glyph_constructor",
    .title = "Build a font in code",
    .description = "Creates a SpriteFont without the content pipeline through CNA's public SpriteFont constructor "
                   "(CNAEXT): an atlas texture plus glyph bounds, cropping, characters and kerning.",
    .apis = {"SpriteFont::SpriteFont", "SpriteFont::Characters", "SpriteFont::MeasureString",
             "SpriteFont::getTextureEXT", "SpriteFont::getGlyphBoundsEXT", "SpriteFont::getCroppingEXT",
             "SpriteFont::getKerningEXT", "SpriteBatch::DrawString"},
    .tags = {"text", "font", "procedural", "cnaext"},
    .frames = 3,
    .deterministic = true,
})
