// SPDX-License-Identifier: MS-PL
// SpriteFont::Spacing and SpriteFont::LineSpacing: adjust letter and line spacing at run time.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
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
    class SpacingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            // Two copies of the same loaded font, adjusted differently.
            normal_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            spaced_.emplace(*normal_);

            // Spacing is added between characters; LineSpacing is the distance between baselines.
            spaced_->setSpacingProperty(6.0f);
            spaced_->setLineSpacingProperty(normal_->getLineSpacingProperty() * 2);

            const Vector2 a = normal_->MeasureString("ABC");
            const Vector2 b = spaced_->MeasureString("ABC");
            Context().Print("\"ABC\": {} with Spacing 0, {} with Spacing 6", a.ToString(), b.ToString());
            Context().Check(b.X == a.X + 2.0f * 6.0f, "Spacing is added between characters (2 gaps in \"ABC\")");
            Context().Check(spaced_->MeasureString("A\nB").Y == 2.0f * spaced_->getLineSpacingProperty(),
                            "MeasureString uses the new LineSpacing");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateBlue);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*normal_, "Spacing 0\nLineSpacing as built\nthird line", Vector2(40.0f, 60.0f),
                                     Color::White);
            spriteBatch_->DrawString(*spaced_, "Spacing 6\nLineSpacing doubled\nthird line", Vector2(40.0f, 220.0f),
                                     Color::Gold);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> normal_;
        std::optional<SpriteFont> spaced_;
    };
}

CNA_DEMOS_GAME(SpacingDemo, {
    .id = "graphics.sprite_font.spacing",
    .title = "Letter and line spacing",
    .description = "Changes SpriteFont::Spacing and SpriteFont::LineSpacing on a copy of a loaded font and shows the "
                   "effect on drawing and on MeasureString.",
    .apis = {"SpriteFont::Spacing", "SpriteFont::LineSpacing", "SpriteFont::MeasureString"},
    .tags = {"text", "font", "spacing", "layout"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
