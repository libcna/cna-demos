// SPDX-License-Identifier: MS-PL
// CNAEXT relative mouse mode: GetState reports movement instead of a position, without a cursor.
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
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class RelativeModeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            originalRelative_ = Mouse::getIsRelativeMouseModeEXTProperty();
            previousKeyboard_ = Keyboard::GetState();
        }

        void UnloadContent() override
        {
            if (changedByDemo_ && Mouse::getIsRelativeMouseModeEXTProperty() != originalRelative_)
                Mouse::setIsRelativeMouseModeEXTProperty(originalRelative_);
        }

        void Update(GameTime& gameTime) override
        {
            // Space switches the mode. It grabs the pointer (Escape still leaves the demo).
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && keyboard.IsKeyDown(Keys::Space) &&
                previousKeyboard_.IsKeyUp(Keys::Space))
            {
                Mouse::setIsRelativeMouseModeEXTProperty(!Mouse::getIsRelativeMouseModeEXTProperty());
                changedByDemo_ = true;
            }
            previousKeyboard_ = keyboard;

            // In relative mode X and Y are the motion since the previous GetState -- not limited by
            // the window or screen edge, no SetPosition re-centring needed.
            const MouseState mouse = Mouse::GetState();
            relative_ = Mouse::getIsRelativeMouseModeEXTProperty();
            if (relative_)
            {
                deltaX_ = mouse.getXProperty();
                deltaY_ = mouse.getYProperty();
                ship_.X = MathHelper::Clamp(ship_.X + static_cast<float>(deltaX_), 20.0f, 780.0f);
                ship_.Y = MathHelper::Clamp(ship_.Y + static_cast<float>(deltaY_), 120.0f, 460.0f);
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(relative_ ? Color::DarkSlateBlue : Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Space: relative mode on/off", Vector2(30.0f, 20.0f), Color::White);
            spriteBatch_->DrawString(*font_,
                                     relative_ ? "Relative: the mouse steers the square"
                                               : "Absolute: X and Y are the cursor position",
                                     Vector2(30.0f, 50.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_,
                                     "Motion this frame: " + std::to_string(deltaX_) + ", " + std::to_string(deltaY_),
                                     Vector2(30.0f, 80.0f), Color::LightGray);
            spriteBatch_->Draw(*pixel_,
                               Rectangle(static_cast<int>(ship_.X) - 12, static_cast<int>(ship_.Y) - 12, 24, 24),
                               Color::Orange);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        KeyboardState previousKeyboard_;
        bool relative_ = false;
        bool originalRelative_ = false;
        bool changedByDemo_ = false;
        int deltaX_ = 0;
        int deltaY_ = 0;
        Vector2 ship_{400.0f, 300.0f};
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(RelativeModeDemo, {
    .id = "input.mouse.relative_mode",
    .title = "Relative mouse mode (CNAEXT)",
    .description = "A fresh Space press in an attended run toggles Mouse::IsRelativeMouseModeEXT. The demo records "
                   "the original process-wide mode, restores it if changed, and uses relative motion to steer a square.",
    .apis = {"Mouse::IsRelativeMouseModeEXT", "Mouse::GetState"},
    .tags = {"mouse", "relative", "mouse-look", "grab", "fps", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Mouse | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs only read state; relative pointer capture requires Space and restores the "
                     "original mode on shutdown."},
})
