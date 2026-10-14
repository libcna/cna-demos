// SPDX-License-Identifier: MS-PL
// SpriteFont: load a compiled font and draw text with SpriteBatch::DrawString.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DrawStringDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // A SpriteFont is compiled content: content/fonts/pixel_font.png went through CNA's
            // content pipeline (FontTextureProcessor) at build time and is loaded by name.
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            Context().Print("{} characters, LineSpacing {}, Spacing {}", font_->getCharactersProperty().size(),
                            font_->getLineSpacingProperty(), font_->getSpacingProperty());
            Context().Check(font_->getCharactersProperty().size() == 95, "the font covers printable ASCII");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::MidnightBlue);

            // A pixel font stays crisp with point sampling.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Hello, CNA!", Vector2(40.0f, 40.0f), Color::White);
            spriteBatch_->DrawString(*font_, "The colour argument tints the glyphs.", Vector2(40.0f, 90.0f),
                                     Color::Gold);
            // "\n" starts a new line, LineSpacing pixels lower.
            spriteBatch_->DrawString(*font_, "Line one\r\nLine two\nLine three", Vector2(40.0f, 140.0f),
                                     Color::LightGreen);
            spriteBatch_->DrawString(*font_, "0123456789 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
                                     Vector2(40.0f, 260.0f), Color::LightSkyBlue);
            spriteBatch_->DrawString(*font_, "The quick brown fox jumps over the lazy dog.", Vector2(40.0f, 300.0f),
                                     Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                const int lineSpacing = font_->getLineSpacingProperty();
                const int prefixWidth = static_cast<int>(font_->MeasureString("Line ").X);
                int prefixInk = 0;
                bool repeatedPrefixMatches = true;
                for (int y = 0; y < lineSpacing; ++y)
                {
                    for (int x = 0; x < prefixWidth; ++x)
                    {
                        const bool first = pixels.At(40 + x, 140 + y) == Color::LightGreen;
                        prefixInk += first ? 1 : 0;
                        repeatedPrefixMatches = repeatedPrefixMatches &&
                            (pixels.At(40 + x, 140 + lineSpacing + y) == Color::LightGreen) == first &&
                            (pixels.At(40 + x, 140 + lineSpacing * 2 + y) == Color::LightGreen) == first;
                    }
                }
                Context().Check(prefixInk > 0 && repeatedPrefixMatches,
                                "CRLF and LF place the repeated glyph prefix at exact LineSpacing intervals");

                int whiteInk = 0;
                int goldInk = 0;
                int greenInk = 0;
                int blueInk = 0;
                for (int y = 35; y < 330; ++y)
                {
                    for (int x = 35; x < 780; ++x)
                    {
                        const Color sample = pixels.At(x, y);
                        whiteInk += sample == Color::White ? 1 : 0;
                        goldInk += sample == Color::Gold ? 1 : 0;
                        greenInk += sample == Color::LightGreen ? 1 : 0;
                        blueInk += sample == Color::LightSkyBlue ? 1 : 0;
                    }
                }
                Context().Check(whiteInk > 0 && goldInk > 0 && greenInk > 0 && blueInk > 0,
                                "compiled glyph ink reaches the framebuffer under every displayed text tint");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(DrawStringDemo, {
    .id = "graphics.sprite_font.draw_string",
    .title = "Draw text",
    .description = "Loads a SpriteFont compiled by CNA's content pipeline and draws coloured, multi-line text with "
                   "SpriteBatch::DrawString.",
    .apis = {"ContentManager::Load", "SpriteBatch::DrawString", "SpriteFont::Characters", "SpriteFont::LineSpacing",
             "SpriteFont::Spacing"},
    .tags = {"text", "font", "string", "2d", "hello"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
