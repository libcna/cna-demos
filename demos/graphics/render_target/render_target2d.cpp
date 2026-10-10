// SPDX-License-Identifier: MS-PL
// Rendering into a RenderTarget2D, then drawing that render target as a texture.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class RenderTarget2DDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");

            // A 128x128 off-screen colour buffer. RenderTarget2D derives from Texture2D, so once
            // rendered it can be drawn like any texture.
            target_ = std::make_unique<RenderTarget2D>(device, 128, 128);
        }

        void UnloadContent() override
        {
            // GPU resources are released explicitly with Dispose(), as in XNA.
            target_->Dispose();
            Context().Check(target_->getIsDisposedProperty(), "the render target reports itself disposed");
        }

        void Draw(const GameTime& gameTime) override
        {
            auto& device = getGraphicsDeviceProperty();

            // 1. Draw into the render target.
            device.SetRenderTarget(target_.get());
            device.Clear(Color::DarkSlateGray);
            spriteBatch_->Begin();
            spriteBatch_->Draw(arrow_, Rectangle(32, 32, 64, 64), Color::White);
            spriteBatch_->End();

            // 2. Back to the back buffer, then draw the render target's contents four times.
            device.SetRenderTarget(nullptr);
            device.Clear(Color::CornflowerBlue);
            spriteBatch_->Begin();
            spriteBatch_->Draw(*target_, Rectangle(40, 40, 128, 128), Color::White);
            spriteBatch_->Draw(*target_, Rectangle(200, 40, 256, 256), Color::White);
            spriteBatch_->Draw(*target_, Rectangle(480, 40, 64, 64), Color::White);
            spriteBatch_->Draw(*target_, Rectangle(480, 120, 64, 64), Color::Orange);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<RenderTarget2D> target_;
        Texture2D arrow_;
    };
}

CNA_DEMOS_GAME(RenderTarget2DDemo, {
    .id = "graphics.render_target.render_target2d",
    .title = "Render to a texture",
    .description = "Draws a sprite into a RenderTarget2D, switches back to the back buffer with "
                   "SetRenderTarget(nullptr) and draws the render target as an ordinary texture at several sizes and with a tint.",
    .apis = {"RenderTarget2D::RenderTarget2D", "GraphicsDevice::SetRenderTarget", "GraphicsResource::Dispose",
             "GraphicsResource::IsDisposed"},
    .tags = {"offscreen", "render-to-texture"},
    .frames = 10,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
})
