// SPDX-License-Identifier: MS-PL
// SpriteFont::MeasureString: the size text will occupy, for centring, aligning and boxes.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class MeasureStringDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Width: the sum of the characters' advances (plus Spacing between them); for several
            // lines, the widest line.
            // Height: LineSpacing for every "\n", plus the last line's height -- LineSpacing, or the
            // tallest glyph cell on it if that is taller (this font's cells are 20 pixels, its
            // LineSpacing 18).
            const float lineSpacing = static_cast<float>(font_->getLineSpacingProperty());
            const Vector2 one = font_->MeasureString("A");
            const Vector2 four = font_->MeasureString("ABCD");
            const Vector2 lines = font_->MeasureString("AB\nCD\nEF");
            Context().Print("LineSpacing {}; MeasureString(\"A\") = {}, (\"ABCD\") = {}, (\"AB\\nCD\\nEF\") = {}",
                            lineSpacing, one.ToString(), four.ToString(), lines.ToString());
            Context().Check(four.X == 4.0f * one.X, "a fixed-width font: four characters are four advances wide");
            Context().Check(one.Y >= lineSpacing, "one line is at least LineSpacing high");
            Context().Check(lines.Y == one.Y + 2.0f * lineSpacing, "each \\n adds LineSpacing");
            Context().Check(lines.X == font_->MeasureString("AB").X, "the width is the widest line's");

            // A trailing newline still counts a whole (empty) line.
            const Vector2 trailing = font_->MeasureString("AB\n");
            Context().Print("MeasureString(\"AB\\n\") = {}", trailing.ToString());
            Context().Check(trailing.Y == 2.0f * lineSpacing, "a trailing \\n adds an empty line");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);

            // Centred: position = centre - size / 2.
            const std::string title = "Centred with MeasureString";
            const Vector2 size = font_->MeasureString(title);
            const Vector2 centred = Vector2(400.0f, 80.0f) - size / 2.0f;
            DrawBox(centred, size, Color::SteelBlue);
            spriteBatch_->DrawString(*font_, title, centred, Color::White);

            // Right-aligned against x = 760.
            for (int i = 0; i < 3; ++i)
            {
                const std::string text = std::string("Score: ") + std::to_string(5 * i * i * i * 100);
                const Vector2 extent = font_->MeasureString(text);
                const Vector2 position(760.0f - extent.X, 180.0f + 30.0f * i);
                DrawBox(position, extent, Color::DarkOliveGreen);
                spriteBatch_->DrawString(*font_, text, position, Color::White);
            }

            // A box around multi-line text.
            const std::string block = "MeasureString measures\nevery line and returns\nthe widest one.";
            const Vector2 blockSize = font_->MeasureString(block);
            DrawBox(Vector2(40.0f, 300.0f), blockSize, Color::Maroon);
            spriteBatch_->DrawString(*font_, block, Vector2(40.0f, 300.0f), Color::White);

            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void DrawBox(Vector2 position, Vector2 size, Color color)
        {
            spriteBatch_->Draw(*pixel_,
                               Rectangle(static_cast<int>(position.X), static_cast<int>(position.Y),
                                         static_cast<int>(size.X), static_cast<int>(size.Y)),
                               color);
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(MeasureStringDemo, {
    .id = "graphics.sprite_font.measure_string",
    .title = "Measure text",
    .description = "Uses SpriteFont::MeasureString to centre a title, right-align numbers and draw boxes behind "
                   "text, and checks how characters, newlines and LineSpacing make up the result.",
    .apis = {"SpriteFont::MeasureString", "SpriteFont::LineSpacing"},
    .tags = {"text", "font", "layout", "alignment", "ui"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
