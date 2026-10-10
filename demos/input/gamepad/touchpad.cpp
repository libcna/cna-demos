// SPDX-License-Identifier: MS-PL
// CNAEXT controller touchpads: the fingers on a PlayStation controller's touch surface.
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
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadCapabilities.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    const Rectangle Pad(150, 120, 500, 250);

    class TouchpadDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);

            const bool hasTouchpad = GamePad::GetCapabilities(PlayerIndex::One).getHasTouchPadEXTProperty();
            // Pressing the surface is a button of its own.
            const bool clicked = GamePad::GetState(PlayerIndex::One).IsButtonDown(Buttons::TouchPadEXT);
            const int touchpads = GamePad::GetTouchpadCountEXT(PlayerIndex::One);
            spriteBatch_->DrawString(*font_,
                                     hasTouchpad ? std::to_string(touchpads) + " touchpad(s) on controller one"
                                                 : std::string("Controller one reports no touchpad (HasTouchPadEXT)"),
                                     Vector2(30.0f, 30.0f), hasTouchpad ? Color::White : Color::Gray);
            spriteBatch_->Draw(*pixel_, Pad, clicked ? Color::SlateGray : Color::DimGray);

            // Finger positions are normalized: 0..1 across the surface, with a pressure value.
            if (touchpads > 0)
            {
                const int fingers = GamePad::GetTouchpadFingerCountEXT(PlayerIndex::One, 0);
                for (int finger = 0; finger < fingers; ++finger)
                {
                    bool down = false;
                    float x = 0.0f;
                    float y = 0.0f;
                    float pressure = 0.0f;
                    if (GamePad::GetTouchpadFingerEXT(PlayerIndex::One, 0, finger, down, x, y, pressure) && down)
                    {
                        const int px = Pad.X + static_cast<int>(x * static_cast<float>(Pad.Width));
                        const int py = Pad.Y + static_cast<int>(y * static_cast<float>(Pad.Height));
                        const int size = 16 + static_cast<int>(pressure * 16.0f);
                        spriteBatch_->Draw(*pixel_, Rectangle(px - size / 2, py - size / 2, size, size),
                                           finger == 0 ? Color::Orange : Color::Yellow);
                    }
                }
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(TouchpadDemo, {
    .id = "input.gamepad.touchpad",
    .title = "Controller touchpad (CNAEXT)",
    .description = "Draws the fingers on controller one's touchpad from GamePad::GetTouchpadCountEXT, "
                   "GetTouchpadFingerCountEXT and GetTouchpadFingerEXT (normalized position and pressure); a press of "
                   "the surface is Buttons::TouchPadEXT.",
    .apis = {"GamePad::GetTouchpadCountEXT", "GamePad::GetTouchpadFingerCountEXT", "GamePad::GetTouchpadFingerEXT",
             "GamePadCapabilities::HasTouchPadEXT", "Buttons::TouchPadEXT"},
    .tags = {"gamepad", "controller", "touchpad", "playstation", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
