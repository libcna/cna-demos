// SPDX-License-Identifier: MS-PL
// Mouse::SetPosition: move the cursor -- and the classic XNA mouse look built on re-centring it.
#include <cmath>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    constexpr int CentreX = 400;
    constexpr int CentreY = 240;

    class SetPositionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            setIsMouseVisibleProperty(true);
            rightWasDown_ = Mouse::GetState().getRightButtonProperty() == ButtonState::Pressed;
            Context().Check(CentreX == 400 && CentreY == 240,
                            "the deterministic look target is the centre of the 800x480 demo window");
        }

        void UnloadContent() override { EndOwnedLook(); }

        void Update(GameTime& gameTime) override
        {
            // Mouse look, the XNA way: while the right button is held, read how far the cursor
            // moved from the window centre, then put it back there. The cursor never reaches the
            // edge of the window, so the view can turn forever.
            const MouseState mouse = Mouse::GetState();
            const bool rightDown = mouse.getRightButtonProperty() == ButtonState::Pressed;
            if (!Context().Automated() && getIsActiveProperty() && rightDown && !rightWasDown_)
            {
                originalX_ = mouse.getXProperty();
                originalY_ = mouse.getYProperty();
                ownsLook_ = true;
                Mouse::SetPosition(CentreX, CentreY);
                setIsMouseVisibleProperty(false);
            }
            if (ownsLook_ && rightDown && rightWasDown_)
            {
                yaw_ += static_cast<float>(mouse.getXProperty() - CentreX);
                pitch_ += static_cast<float>(mouse.getYProperty() - CentreY);
                Mouse::SetPosition(CentreX, CentreY);
            }
            if (ownsLook_ && (!rightDown || !getIsActiveProperty()))
                EndOwnedLook();
            rightWasDown_ = rightDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // A grid that scrolls with the accumulated movement stands in for a 3D view.
            const int offsetX = static_cast<int>(std::fmod(-yaw_, 80.0f) + 80.0f) % 80;
            const int offsetY = static_cast<int>(std::fmod(-pitch_, 80.0f) + 80.0f) % 80;
            for (int x = offsetX - 80; x < 800; x += 80)
                spriteBatch_->Draw(*pixel_, Rectangle(x, 0, 2, 480), Color::DarkGreen);
            for (int y = offsetY - 80; y < 480; y += 80)
                spriteBatch_->Draw(*pixel_, Rectangle(0, y, 800, 2), Color::DarkGreen);
            spriteBatch_->Draw(*pixel_, Rectangle(CentreX - 10, CentreY, 21, 1), Color::White);
            spriteBatch_->Draw(*pixel_, Rectangle(CentreX, CentreY - 10, 1, 21), Color::White);
            spriteBatch_->DrawString(*font_, "Hold the right mouse button and move the mouse to look around",
                                     Vector2(20.0f, 20.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void EndOwnedLook()
        {
            if (!ownsLook_)
                return;
            Mouse::SetPosition(originalX_, originalY_);
            setIsMouseVisibleProperty(true);
            ownsLook_ = false;
        }

        float yaw_ = 0.0f;
        float pitch_ = 0.0f;
        bool rightWasDown_ = false;
        bool ownsLook_ = false;
        int originalX_ = 0;
        int originalY_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(SetPositionDemo, {
    .id = "input.mouse.set_position",
    .title = "Move the cursor (mouse look)",
    .description = "A fresh right-button press in an attended run starts classic XNA mouse look: SetPosition recentres "
                   "the pointer while held, and release/deactivation/shutdown restores the pre-look position.",
    .apis = {"Mouse::SetPosition", "Mouse::GetState", "Game::IsActive", "Game::IsMouseVisible"},
    .tags = {"mouse", "cursor", "warp", "mouse-look", "camera", "fps", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Mouse,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs never call SetPosition; an attended press owns and restores the pointer warp."},
})
