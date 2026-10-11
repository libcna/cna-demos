// SPDX-License-Identifier: MS-PL
// Game::IsActive, OnActivated and OnDeactivated: pausing when the player switches away.
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
#include "System/EventArgs.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ActivationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        // Overriding the On* methods is the XNA way for the game itself; other objects subscribe
        // to the Activated and Deactivated events instead.
        void OnActivated(System::Object* sender, const System::EventArgs& args) override
        {
            ++activations_;
            Game::OnActivated(sender, args);
        }

        void OnDeactivated(System::Object* sender, const System::EventArgs& args) override
        {
            ++deactivations_;
            paused_ = true; // a game opens its pause menu when the player switches away
            Game::OnDeactivated(sender, args);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (paused_ && getIsActiveProperty() &&
                Input::Keyboard::GetState().IsKeyDown(Input::Keys::Space))
            {
                paused_ = false;
            }
            // The usual rule: no game play while another window has the focus.
            if (!getIsActiveProperty() || paused_)
            {
                return;
            }
            seconds_ += gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty();
        }

        void Draw(const GameTime& gameTime) override
        {
            const bool active = getIsActiveProperty();
            getGraphicsDeviceProperty().Clear(active ? Color(28, 44, 36) : Color(44, 30, 30));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, std::string("IsActive: ") + (active ? "yes" : "no -- click another window"),
                                     Vector2(20.0f, 20.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Activated " + std::to_string(activations_) + "x, Deactivated " +
                                                 std::to_string(deactivations_) + "x",
                                     Vector2(20.0f, 50.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, paused_ ? "PAUSED (press Space to resume)" : "Playing: " + std::to_string(seconds_).substr(0, 5) + " s",
                                     Vector2(20.0f, 90.0f), paused_ ? Color::Orange : Color::LightGreen);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

        void EndRun() override
        {
            Context().Print("IsActive at the end: {}, {} activation(s), {} deactivation(s)", getIsActiveProperty(),
                            activations_, deactivations_);
            DemoGame::EndRun();
        }

    private:
        int activations_ = 0;
        int deactivations_ = 0;
        bool paused_ = false;
        double seconds_ = 0.0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ActivationDemo, {
    .id = "game.lifecycle.activation",
    .title = "Focus and pausing",
    .description = "Pauses game play while the window is not active (Game::IsActive) and opens a pause state from "
                   "OnDeactivated; OnActivated counts the returns. Switch to another window and back to see it.",
    .apis = {"Game::IsActive", "Game::OnActivated", "Game::OnDeactivated", "Game::Activated", "Game::Deactivated"},
    .tags = {"lifecycle", "focus", "pause", "window"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Window,
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
})
