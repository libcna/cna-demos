// SPDX-License-Identifier: MS-PL
// RenderTargetUsage: DiscardContents clears a target when rebound; Preserve/PlatformContents retain it.
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class UsageDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            discard_ = std::make_unique<RenderTarget2D>(device, 256, 256, false, SurfaceFormat::Color, DepthFormat::None,
                                                        0, RenderTargetUsage::DiscardContents);
            preserve_ = std::make_unique<RenderTarget2D>(device, 256, 256, false, SurfaceFormat::Color,
                                                         DepthFormat::None, 0, RenderTargetUsage::PreserveContents);
            platform_ = std::make_unique<RenderTarget2D>(device, 256, 256, false, SurfaceFormat::Color,
                                                         DepthFormat::None, 0, RenderTargetUsage::PlatformContents);
            Context().Check(platform_->getRenderTargetUsageProperty() == RenderTargetUsage::PlatformContents,
                            "the target retains the platform-selected contents policy");

            // Frame 0 paints a red square into both targets, once. Every later frame only adds a
            // blue square, without clearing -- so what survives of the red one is the usage.
            for (RenderTarget2D* target : {discard_.get(), preserve_.get(), platform_.get()})
            {
                device.SetRenderTarget(target);
                device.Clear(Color::DimGray);
                Paint(Rectangle(30, 30, 120, 120), Color::Red);
            }
            device.SetRenderTarget(nullptr);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            for (RenderTarget2D* target : {discard_.get(), preserve_.get(), platform_.get()})
            {
                device.SetRenderTarget(target);  // set again: DiscardContents loses what was there
                Paint(Rectangle(106, 106, 120, 120), Color::Blue);
            }
            device.SetRenderTarget(nullptr);

            if (FramesDrawn() == 0)
            {
                Context().Check(!HasRed(*discard_), "DiscardContents: the earlier drawing is gone");
                Context().Check(HasRed(*preserve_), "PreserveContents: the earlier drawing is still there");
                Context().Check(HasRed(*platform_),
                                "PlatformContents: this desktop platform preserves the earlier drawing");
            }

            device.Clear(Color::DarkSlateBlue);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*discard_, Rectangle(20, 90, 240, 240), Color::White);
            spriteBatch_->Draw(*preserve_, Rectangle(280, 90, 240, 240), Color::White);
            spriteBatch_->Draw(*platform_, Rectangle(540, 90, 240, 240), Color::White);
            spriteBatch_->DrawString(*font_, "DiscardContents", Vector2(20.0f, 350.0f), Color::White);
            spriteBatch_->DrawString(*font_, "PreserveContents", Vector2(280.0f, 350.0f), Color::White);
            spriteBatch_->DrawString(*font_, "PlatformContents", Vector2(540.0f, 350.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void Paint(Rectangle area, Color color)
        {
            spriteBatch_->Begin();
            spriteBatch_->Draw(*pixel_, area, color);
            spriteBatch_->End();
        }

        // A render target is a Texture2D, so GetData reads what was rendered into it.
        static bool HasRed(const RenderTarget2D& target)
        {
            Color pixel;
            const Rectangle inRedOnly(50, 50, 1, 1);
            target.GetData(0, &inRedOnly, &pixel, 0, 1);
            return pixel == Color::Red;
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
        std::unique_ptr<RenderTarget2D> discard_;
        std::unique_ptr<RenderTarget2D> preserve_;
        std::unique_ptr<RenderTarget2D> platform_;
    };
}

CNA_DEMOS_GAME(UsageDemo, {
    .id = "graphics.render_target.usage",
    .title = "Discard or preserve contents",
    .description = "Paints into three render targets once, then sets them again every frame: DiscardContents loses "
                   "the earlier drawing while PreserveContents and desktop PlatformContents retain it.",
    .apis = {"RenderTargetUsage::DiscardContents", "RenderTargetUsage::PreserveContents",
             "RenderTargetUsage::PlatformContents", "RenderTarget2D::RenderTarget2D",
             "RenderTarget2D::RenderTargetUsage", "GraphicsDevice::SetRenderTarget"},
    .tags = {"render-target", "offscreen", "usage", "accumulate"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
