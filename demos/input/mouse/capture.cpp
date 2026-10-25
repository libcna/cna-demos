// SPDX-License-Identifier: MS-PL
// Mouse capture outside the client area, explicitly toggled and always released.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class MouseCaptureDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            setIsMouseVisibleProperty(true);
            DemoGame::Initialize();
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            cWasDown_ = Keyboard::GetState().IsKeyDown(Keys::C);
        }

        void UnloadContent() override { ReleaseCapture(); }

        void Update(GameTime& gameTime) override
        {
            mouse_ = Mouse::GetState();
            if (captured_ && !getIsActiveProperty())
            {
                ReleaseCapture();
                status_ = "Capture released because the game lost activation.";
            }

            const bool cDown = Keyboard::GetState().IsKeyDown(Keys::C);
            if (!Context().Automated() && cDown && !cWasDown_)
            {
                if (captured_)
                {
                    ReleaseCapture();
                    status_ = "Capture released.";
                }
                else
                {
                    const bool accepted = Mouse::SetCaptureEXT(true);
                    captured_ = accepted;
                    status_ = accepted ? "Captured: drag outside the window and keep observing coordinates."
                                       : "The platform refused mouse capture.";
                }
            }
            cWasDown_ = cDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(captured_ ? Color::DarkGreen : Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Press C to capture/release the real mouse.", Vector2(24.0f, 30.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "Capture keeps events flowing during a drag outside the client area.",
                                     Vector2(24.0f, 65.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, std::string("Captured: ") + (captured_ ? "yes" : "no") +
                                         "  pointer: (" + std::to_string(mouse_.getXProperty()) + ", " +
                                         std::to_string(mouse_.getYProperty()) + ")",
                                     Vector2(24.0f, 120.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, status_, Vector2(24.0f, 160.0f), Color::LightGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void ReleaseCapture()
        {
            if (captured_)
                Mouse::SetCaptureEXT(false);
            captured_ = false;
        }

        bool captured_ = false;
        bool cWasDown_ = false;
        MouseState mouse_;
        std::string status_ = "Capture is off until C is pressed.";
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(MouseCaptureDemo, {
    .id = "input.mouse.capture",
    .title = "Mouse capture outside the window",
    .description = "Press C to call Mouse::SetCaptureEXT so drag input continues outside the client area; capture is "
                   "released on a second press, loss of activation and unload.",
    .apis = {"Mouse::SetCaptureEXT", "Mouse::GetState", "Game::IsActive"},
    .tags = {"mouse", "capture", "drag", "window", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Mouse | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: C changes real host mouse capture; cleanup releases capture on deactivation and exit.",
})
