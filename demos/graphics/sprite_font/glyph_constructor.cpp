// SPDX-License-Identifier: MS-PL
// CNAEXT SpriteFont constructor: build a font in code from a texture and glyph tables.
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
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
            font_.emplace(atlas, bounds, cropping, characters, GlyphHeight + 1, 0.0f, kerning, std::nullopt);

            const Vector2 size = font_->MeasureString("2026");
            Context().Print("MeasureString(\"2026\") = {}", size.ToString());
            Context().Check(size.X == 4.0f * (GlyphWidth + 1), "each digit advances 3 + 1 pixels");
            Context().Check(font_->getCharactersProperty().size() == 10, "the font has the ten digits");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "0123456789", Vector2(40.0f, 60.0f), Color::Lime, 0.0f, Vector2::Zero,
                                     12.0f, SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_, "2026", Vector2(40.0f, 200.0f), Color::Orange, 0.0f, Vector2::Zero, 24.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->End();
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
    .apis = {"SpriteFont::SpriteFont", "SpriteFont::MeasureString", "SpriteBatch::DrawString"},
    .tags = {"text", "font", "procedural", "cnaext"},
    .frames = 3,
    .deterministic = true,
})
