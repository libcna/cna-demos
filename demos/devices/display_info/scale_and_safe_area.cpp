// SPDX-License-Identifier: MS-PL
// CNA::Devices::DisplayInfo: how large to draw (content scale) and where nothing covers the
// window (safe area: notches, rounded corners, system bars on phones).
#include <memory>
#include <optional>
#include <string>

#include "CNA/Devices/DisplayInfo.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
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
    class ScaleAndSafeAreaDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Content scale: 1 on a standard display, 2 on a "retina"/200 % one. A game sizes its
            // text and touch targets by it, so they keep their physical size.
            GameWindow& window = getWindowProperty();
            scale_ = CNA::Devices::DisplayInfo::getContentScaleProperty(window);
            // Safe area: the part of the window's client area that nothing covers, in the same
            // coordinates as ClientBounds' size. On a desktop window it is the whole client area.
            safe_ = CNA::Devices::DisplayInfo::getSafeAreaProperty(window);
            const Rectangle client = window.getClientBoundsProperty();
            ctx.Print("content scale {}, client area {}x{}, safe area {}", scale_, client.Width, client.Height,
                      safe_.ToString());
            ctx.Check(scale_ >= 1.0f, "the content scale is at least 1");
            ctx.Check(!safe_.getIsEmptyProperty() && safe_.X >= 0 && safe_.Y >= 0 &&
                          safe_.getRightProperty() <= client.Width && safe_.getBottomProperty() <= client.Height,
                      "the safe area lies within the client area");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 34, 44));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // Outline the safe area; the HUD goes inside it, scaled by the content scale.
            const int t = 4;
            const Rectangle& s = safe_;
            spriteBatch_->Draw(*pixel_, Rectangle(s.X, s.Y, s.Width, t), Color::LimeGreen);
            spriteBatch_->Draw(*pixel_, Rectangle(s.X, s.getBottomProperty() - t, s.Width, t), Color::LimeGreen);
            spriteBatch_->Draw(*pixel_, Rectangle(s.X, s.Y, t, s.Height), Color::LimeGreen);
            spriteBatch_->Draw(*pixel_, Rectangle(s.getRightProperty() - t, s.Y, t, s.Height), Color::LimeGreen);
            const Vector2 hud(static_cast<float>(s.X + 20), static_cast<float>(s.Y + 20));
            spriteBatch_->DrawString(*font_, "Score 1200", hud, Color::White, 0.0f, Vector2::Zero, scale_,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_, "content scale " + std::to_string(scale_).substr(0, 4),
                                     Vector2(hud.X, hud.Y + 40.0f * scale_), Color::LightGray, 0.0f, Vector2::Zero,
                                     scale_, SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_, "safe area " + safe_.ToString(), Vector2(hud.X, hud.Y + 70.0f * scale_),
                                     Color::LightGray, 0.0f, Vector2::Zero, scale_, SpriteEffects::None, 0.0f);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        float scale_ = 1.0f;
        Rectangle safe_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(ScaleAndSafeAreaDemo, {
    .id = "devices.display_info.scale_and_safe_area",
    .title = "Content scale and safe area",
    .description = "Asks CNA::Devices::DisplayInfo for the window's content scale (sizes the HUD) and safe area "
                   "(where no notch or system bar covers the window), and draws the HUD inside the safe area at "
                   "that scale.",
    .apis = {"CNA::Devices::DisplayInfo::ContentScale", "CNA::Devices::DisplayInfo::SafeArea",
             "GameWindow::ClientBounds", "SpriteBatch::DrawString"},
    .tags = {"devices", "display", "dpi", "safe-area", "hud", "cnaext"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
})
