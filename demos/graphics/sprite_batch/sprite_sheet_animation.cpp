// SPDX-License-Identifier: MS-PL
// Frame animation from a sprite strip: change the source rectangle over time.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SpriteSheetAnimationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            strip_ = getContentProperty().Load<Texture2D>("textures/sprite_strip");
        }

        void Update(GameTime& gameTime) override
        {
            // Advance one animation frame every 0.25 s of game time (deterministic: fixed step).
            const double seconds = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty();
            frame_ = static_cast<int>(seconds / 0.25) % 4;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);

            spriteBatch_->Begin();
            spriteBatch_->Draw(strip_, Rectangle(336, 176, 128, 128), std::optional<Rectangle>(Rectangle(frame_ * 32, 0, 32, 32)),
                               Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D strip_;
        int frame_ = 0;
    };
}

CNA_DEMOS_GAME(SpriteSheetAnimationDemo, {
    .id = "graphics.sprite_batch.sprite_sheet_animation",
    .title = "Sprite-sheet animation",
    .description = "Plays a four-frame animation by moving SpriteBatch::Draw's source rectangle along a sprite strip "
                   "as game time advances.",
    .apis = {"SpriteBatch::Draw", "GameTime::TotalGameTime"},
    .tags = {"sprite", "animation", "sprite-sheet", "2d"},
    .frames = 40,
    .assets = {"textures/sprite_strip.png"},
})
