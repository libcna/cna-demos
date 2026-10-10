// SPDX-License-Identifier: MS-PL
// Magnifying a small texture: SamplerState::PointClamp versus LinearClamp.
#include <memory>

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
    class SamplerStatesDemo final : public CnaDemos::DemoGame
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

            // Point sampling picks the nearest texel: crisp, blocky pixels (pixel art).
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Rectangle(20, 40, 370, 370), Color::White);
            spriteBatch_->End();

            // Linear sampling blends neighbouring texels: smooth, blurred edges when magnified.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::LinearClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Rectangle(410, 40, 370, 370), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(SamplerStatesDemo, {
    .id = "graphics.sprite_batch.sampler_states",
    .title = "Point and linear sampling",
    .description = "Magnifies a small texture once with SamplerState::PointClamp (blocky) and once with "
                   "SamplerState::LinearClamp (smooth), passed to SpriteBatch::Begin.",
    .apis = {"SpriteBatch::Begin", "SamplerState::PointClamp", "SamplerState::LinearClamp"},
    .tags = {"sprite", "filtering", "pixel-art", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
