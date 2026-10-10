// SPDX-License-Identifier: MS-PL
// Content pipeline colour key: the texture processor turns one colour (magenta) transparent.
#include <memory>
#include <optional>
#include <vector>

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
    class ColorKeyDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");

            // content/textures/keyed_ship.png is a ship on a magenta background. Its entry in
            // content/.cna-content.json gives the texture processor colorKey "255,0,255", so the
            // compiled asset has transparent pixels wherever the source was magenta -- the way
            // XNA-era sprite sheets marked their background.
            ship_ = getContentProperty().Load<Texture2D>("textures/keyed_ship");
            std::vector<Color> pixels(static_cast<std::size_t>(ship_.getWidthProperty() * ship_.getHeightProperty()));
            ship_.GetData(pixels.data(), static_cast<int>(pixels.size()));
            const Color corner = pixels.front();
            const Color centre = pixels[static_cast<std::size_t>(30 * ship_.getWidthProperty() + 32)];
            Context().Print("corner {} {} {} {}, centre {} {} {} {}", corner.getRProperty(), corner.getGProperty(),
                            corner.getBProperty(), corner.getAProperty(), centre.getRProperty(), centre.getGProperty(),
                            centre.getBProperty(), centre.getAProperty());
            Context().Check(corner == Color::Transparent, "the magenta background became transparent black (0, 0, 0, 0)");
            Context().Check(centre.getAProperty() == 255, "the ship itself stays opaque");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // A checkerboard behind the ship shows where it is transparent.
            spriteBatch_->Draw(checker_, Rectangle(240, 100, 320, 320), Color::White);
            spriteBatch_->Draw(ship_, Rectangle(272, 132, 256, 256), Color::White);
            spriteBatch_->DrawString(*font_, "keyed_ship.png, compiled with colorKey 255,0,255", Vector2(20.0f, 30.0f),
                                     Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D checker_;
        Texture2D ship_;
    };
}

CNA_DEMOS_GAME(ColorKeyDemo, {
    .id = "content.texture_processor.color_key",
    .title = "Colour key",
    .description = "Loads a sprite compiled by CNA's content pipeline with the TextureProcessor parameter colorKey "
                   "255,0,255: its magenta background is transparent, checked with GetData and shown over a "
                   "checkerboard.",
    .apis = {"ContentManager::Load", "Texture2D::GetData"},
    .tags = {"content", "pipeline", "colour-key", "magenta", "transparency", "texture"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/checker.png", "textures/keyed_ship.cnb"},
})
