// SPDX-License-Identifier: MS-PL
// Content pipeline mipmaps: generateMipmaps makes the processor build the whole mip chain.
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
    class MipmapsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // content/.cna-content.json sets generateMipmaps for this 128x128 checkerboard: the
            // compiled asset carries levels 128, 64, ..., 1, made at build time instead of load time.
            checker_ = getContentProperty().Load<Texture2D>("textures/mipmapped_checker");
            Context().Print("{}x{}, LevelCount {}", checker_.getWidthProperty(), checker_.getHeightProperty(),
                            checker_.getLevelCountProperty());
            Context().Check(checker_.getLevelCountProperty() == 8, "a 128x128 texture with mipmaps has 8 levels");

            // The 1x1 level is the average of the whole image: half white, half near black.
            Color smallest;
            checker_.GetData(7, nullptr, &smallest, 0, 1);
            Context().Print("level 7 (1x1): {} {} {}", smallest.getRProperty(), smallest.getGProperty(),
                            smallest.getBProperty());
            Context().Check(smallest.getRProperty() > 110 && smallest.getRProperty() < 160,
                            "the last level is the image's average grey");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            // Shrinking copies: with mipmaps and linear filtering the fine checkerboard fades to
            // grey instead of breaking up into moire.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            int x = 20;
            for (int size = 256; size >= 16; size /= 2)
            {
                spriteBatch_->Draw(checker_, Rectangle(x, 120, size, size), Color::White);
                x += size + 12;
            }
            spriteBatch_->End();
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_,
                                     "mipmapped_checker: compiled with generateMipmaps, LevelCount " +
                                         std::to_string(checker_.getLevelCountProperty()),
                                     Vector2(20.0f, 30.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(MipmapsDemo, {
    .id = "content.texture_processor.mipmaps",
    .title = "Mipmaps built by the content pipeline",
    .description = "Loads a checkerboard compiled with the TextureProcessor parameter generateMipmaps: LevelCount is 8 "
                   "and the last level is the average grey; shrinking copies fade smoothly with linear filtering.",
    .apis = {"ContentManager::Load", "Texture::LevelCount", "Texture2D::GetData", "SamplerState::LinearClamp"},
    .tags = {"content", "pipeline", "mipmaps", "filtering", "texture"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/mipmapped_checker.cnb"},
})
