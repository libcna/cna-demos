// SPDX-License-Identifier: MS-PL
// A focused screen-transition recipe: render two scenes off-screen and cross-fade them.
#include <cmath>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int SceneWidth = 256;
    constexpr int SceneHeight = 192;

    class FadeTransitionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            pixel_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            first_ = std::make_unique<RenderTarget2D>(device, SceneWidth, SceneHeight);
            second_ = std::make_unique<RenderTarget2D>(device, SceneWidth, SceneHeight);
        }

        void Draw(const GameTime& gameTime) override
        {
            auto& device = getGraphicsDeviceProperty();

            RenderScene(*first_, Color::DarkBlue, Color::Gold, Rectangle(28, 38, 84, 116));
            RenderScene(*second_, Color::DarkRed, Color::LightGreen, Rectangle(144, 38, 84, 116));

            if (!surfacesChecked_)
            {
                const Rectangle origin(0, 0, 1, 1);
                Color firstPixel = Color::Black;
                Color secondPixel = Color::Black;
                first_->GetData(0, &origin, &firstPixel, 0, 1);
                second_->GetData(0, &origin, &secondPixel, 0, 1);
                Context().Check(firstPixel == Color::DarkBlue && secondPixel == Color::DarkRed,
                                "both independently rendered scenes are readable after their targets are unbound");
                surfacesChecked_ = true;
            }

            // A cosine gives an ease-in/ease-out cross-fade. GameTime, not frame count, keeps the
            // transition speed stable when rendering stalls.
            const double seconds = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty();
            const float alpha = static_cast<float>(0.5 - 0.5 * std::cos(seconds * 1.5));
            const Rectangle destination(80, 48, 640, 384);
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend,
                                &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*first_, destination, Color::White);
            spriteBatch_->Draw(*second_, destination, Color::White * alpha);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void RenderScene(RenderTarget2D& target, Color background, Color foreground, const Rectangle& shape)
        {
            auto& device = getGraphicsDeviceProperty();
            device.SetRenderTarget(&target);
            device.Clear(background);
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque,
                                &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*pixel_, shape, foreground);
            spriteBatch_->End();
            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
        }

        bool surfacesChecked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
        std::unique_ptr<RenderTarget2D> first_;
        std::unique_ptr<RenderTarget2D> second_;
    };
}

CNA_DEMOS_GAME(FadeTransitionDemo, {
    .id = "graphics.render_target.fade_transition",
    .title = "Render-target fade transition",
    .description = "Renders two small scenes into separate RenderTarget2D surfaces every frame, then uses "
                   "SpriteBatch alpha blending and GameTime to cross-fade them as one focused transition recipe.",
    .apis = {"RenderTarget2D::RenderTarget2D", "GraphicsDevice::SetRenderTarget", "SpriteBatch::Draw",
             "BlendState::AlphaBlend", "GameTime::TotalGameTime"},
    .tags = {"render-target", "transition", "fade", "cross-fade", "recipe"},
    .frames = 20,
})
