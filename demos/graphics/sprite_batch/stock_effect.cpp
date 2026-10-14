// SPDX-License-Identifier: MS-PL
// A stock 3D Effect passed to SpriteBatch::Begin owns the sprite vertex transform.
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AlphaTestEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class StockEffectDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            texture_.emplace(device, 8, 8, false, SurfaceFormat::Color);
            const std::vector<Color> white(64, Color::White);
            texture_->SetData(white.data(), 0, static_cast<int>(white.size()));

            const Matrix projection =
                Matrix::CreateOrthographicOffCenter(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);
            alphaTest_ = std::make_unique<AlphaTestEffect>(device);
            alphaTest_->setProjectionProperty(projection);
            alphaTest_->setVertexColorEnabledProperty(true);

            basic_ = std::make_unique<BasicEffect>(device);
            basic_->setTextureEnabledProperty(true);
            basic_->setVertexColorEnabledProperty(true);
            basic_->setProjectionProperty(projection);
            basic_->setWorldProperty(Matrix::CreateScale(1.0f, -1.0f, 1.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // With a stock effect, SpriteBatch supplies sprite vertices and the effect's matrices
            // place them. An 8-pixel sprite scaled to one model unit covers the top-right quadrant.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, nullptr,
                                &DepthStencilState::None, &RasterizerState::CullNone,
                                alphaTest_.get());
            spriteBatch_->Draw(*texture_, Vector2::Zero, std::nullopt, Color::Red, 0.0f,
                               Vector2::Zero, 1.0f / 8.0f, SpriteEffects::None, -1.0f);
            spriteBatch_->End();

            // BasicEffect's World matrix owns the same vertices too: flipping Y moves the second
            // unit square to the bottom-right quadrant instead of using SpriteBatch pixel space.
            spriteBatch_->Begin(SpriteSortMode::Immediate, &BlendState::Opaque, nullptr,
                                &DepthStencilState::None, &RasterizerState::CullNone, basic_.get());
            spriteBatch_->Draw(*texture_, Vector2::Zero, std::nullopt, Color::Blue, 0.0f,
                               Vector2::Zero, 1.0f / 8.0f, SpriteEffects::None, -1.0f);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(pixels.At(600, 120) == Color::Red &&
                                    pixels.At(600, 360) == Color::Blue,
                                "stock Projection and World matrices place sprites in opposite right quadrants");
                Context().Check(pixels.At(200, 120) == Color::Black &&
                                    pixels.At(200, 360) == Color::Black &&
                                    pixels.At(20, 20) == Color::Black,
                                "the stock effects replace ordinary SpriteBatch pixel-space placement");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<Texture2D> texture_;
        std::unique_ptr<AlphaTestEffect> alphaTest_;
        std::unique_ptr<BasicEffect> basic_;
    };
}

CNA_DEMOS_GAME(StockEffectDemo, {
    .id = "graphics.sprite_batch.stock_effect",
    .title = "Stock effect vertex transforms",
    .description = "Passes AlphaTestEffect and BasicEffect to SpriteBatch::Begin so their Projection and World "
                   "matrices place sprite vertices in 3D coordinates instead of ordinary pixel space.",
    .apis = {"SpriteBatch::Begin", "AlphaTestEffect::Projection", "BasicEffect::World",
             "BasicEffect::Projection"},
    .tags = {"sprite", "effect", "transform", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
