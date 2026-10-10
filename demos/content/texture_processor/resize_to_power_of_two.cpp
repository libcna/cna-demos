// SPDX-License-Identifier: MS-PL
// Content pipeline resizing: resizeToPowerOfTwo scales a 100x60 image to 128x64 at build time.
#include <memory>
#include <optional>
#include <string>

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

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ResizeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // Reach-profile hardware (and old GPUs) need power-of-two sizes for wrapping and mipmaps.
            // The source is 100x60; each side grows to the next power of two, stretching the image.
            texture_ = getContentProperty().Load<Texture2D>("textures/odd_size");
            Context().Print("odd_size: {}x{}", texture_.getWidthProperty(), texture_.getHeightProperty());
            Context().Check(texture_.getWidthProperty() == 128 && texture_.getHeightProperty() == 64,
                            "100x60 became 128x64");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // Drawn four times its size.
            spriteBatch_->Draw(texture_, Vector2(40.0f, 120.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero, 4.0f,
                               SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_,
                                     "odd_size.png (100x60), compiled with resizeToPowerOfTwo: " +
                                         std::to_string(texture_.getWidthProperty()) + "x" +
                                         std::to_string(texture_.getHeightProperty()),
                                     Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "(the white frame shows the stretch)", Vector2(40.0f, 60.0f),
                                     Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D texture_;
    };
}

CNA_DEMOS_GAME(ResizeDemo, {
    .id = "content.texture_processor.resize_to_power_of_two",
    .title = "Resize to a power of two",
    .description = "Loads a 100x60 image compiled with the TextureProcessor parameter resizeToPowerOfTwo: the "
                   "texture is 128x64, stretched at build time, and drawn enlarged.",
    .apis = {"ContentManager::Load", "Texture2D::Width", "Texture2D::Height"},
    .tags = {"content", "pipeline", "power-of-two", "resize", "texture"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/odd_size.cnb"},
})
