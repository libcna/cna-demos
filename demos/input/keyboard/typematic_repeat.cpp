// SPDX-License-Identifier: MS-PL
// Build typematic key repeat from KeyboardState and GameTime.
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
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class Typematic
    {
    public:
        static constexpr double InitialDelay = 0.5;
        static constexpr double RepeatInterval = 0.1;

        int Update(bool isDown, double elapsedSeconds)
        {
            if (!isDown)
            {
                down_ = false;
                heldSeconds_ = 0.0;
                untilRepeat_ = InitialDelay;
                return 0;
            }
            if (!down_)
            {
                down_ = true;
                heldSeconds_ = 0.0;
                untilRepeat_ = InitialDelay;
                return 0;
            }

            heldSeconds_ += elapsedSeconds;
            untilRepeat_ -= elapsedSeconds;
            int repeats = 0;
            // Catch up after a long frame instead of silently dropping repeat pulses.
            while (untilRepeat_ <= 1e-9)
            {
                ++repeats;
                untilRepeat_ += RepeatInterval;
            }
            return repeats;
        }

        [[nodiscard]] double HeldSeconds() const { return heldSeconds_; }
        [[nodiscard]] bool IsDown() const { return down_; }

    private:
        bool down_ = false;
        double heldSeconds_ = 0.0;
        double untilRepeat_ = InitialDelay;
    };

    void CheckTiming(CnaDemos::DemoContext& context)
    {
        Typematic repeat;
        context.Check(repeat.Update(true, 0.0) == 0 && repeat.Update(true, 0.49) == 0,
                      "pressing and holding for 0.49 s produces no repeat");
        context.Check(repeat.Update(true, 0.01) == 1,
                      "the first repeat fires at the 0.5 s initial delay");
        context.Check(repeat.Update(true, 0.25) == 2,
                      "a 0.25 s frame catches up two 0.1 s repeat intervals");
        context.Check(repeat.Update(false, 0.0) == 0 && !repeat.IsDown() &&
                          repeat.Update(true, 0.0) == 0 && repeat.Update(true, 0.49) == 0,
                      "release and re-press reset the initial delay");
    }

    class TypematicRepeatDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            CheckTiming(Context());
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            const bool down = Keyboard::GetState().IsKeyDown(Keys::Space);
            pulsesThisFrame_ = repeat_.Update(
                down, gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
            repeatCount_ += pulsesThisFrame_;
            if (!down)
                repeatCount_ = 0;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(pulsesThisFrame_ > 0 ? Color::DarkOrange : Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Hold Space", Vector2(40.0f, 40.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Initial delay: 0.5 s", Vector2(40.0f, 90.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "Repeat interval: 0.1 s", Vector2(40.0f, 120.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "Held: " + std::to_string(repeat_.HeldSeconds()) + " s",
                                     Vector2(40.0f, 180.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Repeat pulses: " + std::to_string(repeatCount_),
                                     Vector2(40.0f, 215.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, "Release resets both the timer and count.",
                                     Vector2(40.0f, 270.0f), Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        Typematic repeat_;
        int repeatCount_ = 0;
        int pulsesThisFrame_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(TypematicRepeatDemo, {
    .id = "input.keyboard.typematic_repeat",
    .title = "Held-key typematic repeat",
    .description = "Builds typematic repeat from Keyboard::GetState and GameTime: an initial 0.5 second delay, "
                   "0.1 second repeat intervals, long-frame catch-up, and release/reset behavior. The timing logic "
                   "is checked deterministically before the live Space-key view starts.",
    .apis = {"Keyboard::GetState", "KeyboardState::IsKeyDown", "GameTime::ElapsedGameTime"},
    .tags = {"keyboard", "repeat", "typematic", "game-time", "recipe"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Automation verifies the repeat algorithm; a person is needed to demonstrate a physical held key.",
})
