// SPDX-License-Identifier: MS-PL
// "Pressed this frame": compare the current KeyboardState with the previous frame's.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class KeyTransitionsDemo final : public CnaDemos::DemoGame
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

        void Update(GameTime& gameTime) override
        {
            // XNA has no key events. A key went down this frame when it is down in this frame's
            // snapshot and was up in the previous one; it went up when the reverse is true.
            const KeyboardState current = Keyboard::GetState();
            for (const Keys key : current.GetPressedKeys())
            {
                if (previous_.IsKeyUp(key))
                {
                    lastPressed_ = key;
                    ++presses_;
                }
            }
            for (const Keys key : previous_.GetPressedKeys())
            {
                if (current.IsKeyUp(key))
                    lastReleased_ = key;
            }

            // The difference in practice: one step per press, or one step per frame while held.
            if (current.IsKeyDown(Keys::Right) && previous_.IsKeyUp(Keys::Right))
                stepX_ += 20;
            if (current.IsKeyDown(Keys::Left) && previous_.IsKeyUp(Keys::Left))
                stepX_ -= 20;
            if (current.IsKeyDown(Keys::Right))
                heldX_ += 4;
            if (current.IsKeyDown(Keys::Left))
                heldX_ -= 4;
            stepX_ = (stepX_ % 720 + 720) % 720;
            heldX_ = (heldX_ % 720 + 720) % 720;

            previous_ = current;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Left/Right arrows:", Vector2(40.0f, 40.0f), Color::White);
            spriteBatch_->DrawString(*font_, "pressed -- one step per press", Vector2(40.0f, 70.0f), Color::LightGreen);
            spriteBatch_->Draw(*pixel_, Rectangle(40 + stepX_, 100, 40, 40), Color::LimeGreen);
            spriteBatch_->DrawString(*font_, "down -- moves every frame the key is held", Vector2(40.0f, 160.0f),
                                     Color::LightSkyBlue);
            spriteBatch_->Draw(*pixel_, Rectangle(40 + heldX_, 190, 40, 40), Color::DeepSkyBlue);

            spriteBatch_->DrawString(*font_, "Key presses so far: " + std::to_string(presses_), Vector2(40.0f, 280.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "Last key pressed:  " + std::string(Describe(lastPressed_)),
                                     Vector2(40.0f, 310.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Last key released: " + std::string(Describe(lastReleased_)),
                                     Vector2(40.0f, 340.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        static std::string_view Describe(std::optional<Keys> key) { return key ? CnaDemos::Name(*key) : "-"; }

        KeyboardState previous_;
        int presses_ = 0;
        std::optional<Keys> lastPressed_;
        std::optional<Keys> lastReleased_;
        int stepX_ = 0;
        int heldX_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(KeyTransitionsDemo, {
    .id = "input.keyboard.key_transitions",
    .title = "Key pressed and released",
    .description = "Detects the frame a key goes down or up by comparing Keyboard::GetState with the previous "
                   "frame's KeyboardState; one box steps once per arrow press, the other moves while it is held.",
    .apis = {"Keyboard::GetState", "KeyboardState::IsKeyDown", "KeyboardState::IsKeyUp",
             "KeyboardState::GetPressedKeys"},
    .tags = {"keys", "polling", "pressed", "released", "edge"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
})
