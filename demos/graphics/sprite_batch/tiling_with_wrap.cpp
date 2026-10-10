// SPDX-License-Identifier: MS-PL
// Repeating a texture across a large area: SamplerState::LinearWrap and a big source rectangle.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class TilingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);

            // A source rectangle larger than the texture reads past its edges. With a Wrap
            // sampler the texture repeats: 4 x 2 copies of the 64x64 checker here.
            const Rectangle fourByTwo(0, 0, 64 * 4, 64 * 2);
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointWrap, nullptr, nullptr);
            spriteBatch_->Draw(checker_, Rectangle(16, 16, 768, 384), std::optional<Rectangle>(fourByTwo), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(TilingDemo, {
    .id = "graphics.sprite_batch.tiling_with_wrap",
    .title = "Tile a texture",
    .description = "Repeats a texture across a large rectangle by combining a source rectangle bigger than the "
                   "texture with SamplerState::PointWrap.",
    .apis = {"SpriteBatch::Begin", "SpriteBatch::Draw", "SamplerState::PointWrap", "BlendState::Opaque"},
    .tags = {"sprite", "tiling", "wrap", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
