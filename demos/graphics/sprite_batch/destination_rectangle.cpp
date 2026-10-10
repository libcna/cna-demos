// SPDX-License-Identifier: MS-PL
// Stretching a texture into a rectangle: SpriteBatch::Draw(texture, Rectangle, Color).
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DestinationRectangleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            // The 64x64 texture is scaled to fill each rectangle, whatever its size or shape.
            spriteBatch_->Begin();
            spriteBatch_->Draw(checker_, Rectangle(20, 20, 64, 64), Color::White);     // native size
            spriteBatch_->Draw(checker_, Rectangle(120, 20, 256, 128), Color::White);  // stretched
            spriteBatch_->Draw(checker_, Rectangle(420, 20, 48, 400), Color::White);   // squashed
            spriteBatch_->Draw(checker_, Rectangle(520, 200, 240, 240), Color::White); // enlarged
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(DestinationRectangleDemo, {
    .id = "graphics.sprite_batch.destination_rectangle",
    .title = "Draw into a rectangle",
    .description = "Draws one texture into destination rectangles of different sizes and shapes with the "
                   "SpriteBatch::Draw overload that takes a Rectangle.",
    .apis = {"SpriteBatch::Draw"},
    .tags = {"sprite", "stretch", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
