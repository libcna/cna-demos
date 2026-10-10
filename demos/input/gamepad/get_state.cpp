// SPDX-License-Identifier: MS-PL
// GamePad::GetState: sticks, triggers, buttons and D-pad of a controller, polled once per frame.
#include <array>
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
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class GamePadStateDemo final : public CnaDemos::DemoGame
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
            // One snapshot per player and frame. A missing controller is not an error: its state
            // says IsConnected == false and everything else is at rest.
            for (int i = 0; i < 4; ++i)
                connected_[static_cast<std::size_t>(i)] =
                    GamePad::GetState(static_cast<PlayerIndex>(i)).getIsConnectedProperty();
            pad_ = GamePad::GetState(PlayerIndex::One);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);

            std::string players = "Connected:";
            for (int i = 0; i < 4; ++i)
                players += connected_[static_cast<std::size_t>(i)] ? " " + std::to_string(i + 1) : " -";
            spriteBatch_->DrawString(*font_, players, Vector2(20.0f, 15.0f), Color::White);
            spriteBatch_->DrawString(*font_,
                                     pad_.getIsConnectedProperty()
                                         ? "Player one, PacketNumber " + std::to_string(pad_.getPacketNumberProperty())
                                         : std::string("Player one: no controller connected"),
                                     Vector2(20.0f, 45.0f), pad_.getIsConnectedProperty() ? Color::LightGreen : Color::Gray);

            const GamePadButtons& buttons = pad_.getButtonsProperty();
            const GamePadThumbSticks& sticks = pad_.getThumbSticksProperty();
            const GamePadTriggers& triggers = pad_.getTriggersProperty();
            const GamePadDPad& dpad = pad_.getDPadProperty();

            // Shoulders, triggers (0..1) and sticks (-1..1, Y points up).
            Button(Rectangle(70, 90, 160, 24), buttons.getLeftShoulderProperty(), "LB");
            Button(Rectangle(570, 90, 160, 24), buttons.getRightShoulderProperty(), "RB");
            Bar(Rectangle(30, 130, 20, 160), triggers.getLeftProperty());
            Bar(Rectangle(750, 130, 20, 160), triggers.getRightProperty());
            Stick(Rectangle(70, 130, 160, 160), sticks.getLeftProperty(), buttons.getLeftStickProperty());
            Stick(Rectangle(420, 300, 160, 160), sticks.getRightProperty(), buttons.getRightStickProperty());

            Button(Rectangle(185, 320, 40, 40), dpad.getUpProperty(), "U");
            Button(Rectangle(185, 410, 40, 40), dpad.getDownProperty(), "D");
            Button(Rectangle(140, 365, 40, 40), dpad.getLeftProperty(), "L");
            Button(Rectangle(230, 365, 40, 40), dpad.getRightProperty(), "R");

            Button(Rectangle(320, 180, 70, 30), buttons.getBackProperty(), "Back");
            Button(Rectangle(410, 180, 70, 30), buttons.getStartProperty(), "Start");
            Button(Rectangle(345, 230, 110, 30), buttons.getBigButtonProperty(), "Big");

            Button(Rectangle(630, 140, 40, 40), buttons.getYProperty(), "Y", Color::Gold);
            Button(Rectangle(630, 230, 40, 40), buttons.getAProperty(), "A", Color::LimeGreen);
            Button(Rectangle(585, 185, 40, 40), buttons.getXProperty(), "X", Color::DodgerBlue);
            Button(Rectangle(675, 185, 40, 40), buttons.getBProperty(), "B", Color::Red);

            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void Button(Rectangle bounds, ButtonState state, const std::string& label, Color on = Color::LimeGreen)
        {
            spriteBatch_->Draw(*pixel_, bounds, state == ButtonState::Pressed ? on : Color::DimGray);
            spriteBatch_->DrawString(*font_, label, Vector2(static_cast<float>(bounds.X + 6), static_cast<float>(bounds.Y + 6)),
                                     Color::White);
        }

        void Bar(Rectangle bounds, float value)
        {
            spriteBatch_->Draw(*pixel_, bounds, Color::DimGray);
            const int filled = static_cast<int>(static_cast<float>(bounds.Height) * value);
            spriteBatch_->Draw(*pixel_, Rectangle(bounds.X, bounds.Y + bounds.Height - filled, bounds.Width, filled),
                               Color::Orange);
        }

        void Stick(Rectangle bounds, Vector2 position, ButtonState click)
        {
            spriteBatch_->Draw(*pixel_, bounds, click == ButtonState::Pressed ? Color::SeaGreen : Color::DimGray);
            const int centreX = bounds.X + bounds.Width / 2;
            const int centreY = bounds.Y + bounds.Height / 2;
            spriteBatch_->Draw(*pixel_, Rectangle(centreX, bounds.Y, 1, bounds.Height), Color::Gray);
            spriteBatch_->Draw(*pixel_, Rectangle(bounds.X, centreY, bounds.Width, 1), Color::Gray);
            const int x = centreX + static_cast<int>(position.X * static_cast<float>(bounds.Width / 2 - 8));
            const int y = centreY - static_cast<int>(position.Y * static_cast<float>(bounds.Height / 2 - 8));
            spriteBatch_->Draw(*pixel_, Rectangle(x - 8, y - 8, 16, 16), Color::White);
        }

        GamePadState pad_;
        std::array<bool, 4> connected_{};
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(GamePadStateDemo, {
    .id = "input.gamepad.get_state",
    .title = "Gamepad state",
    .description = "Polls GamePad::GetState for player one every frame and draws its thumbsticks, triggers, "
                   "buttons and D-pad, with the connected players and the PacketNumber.",
    .apis = {"GamePad::GetState", "GamePadState::IsConnected", "GamePadState::PacketNumber", "GamePadState::Buttons",
             "GamePadState::ThumbSticks", "GamePadState::Triggers", "GamePadState::DPad", "GamePadButtons::A",
             "GamePadButtons::B", "GamePadButtons::X", "GamePadButtons::Y", "GamePadButtons::Back",
             "GamePadButtons::Start", "GamePadButtons::BigButton", "GamePadButtons::LeftShoulder",
             "GamePadButtons::RightShoulder", "GamePadButtons::LeftStick", "GamePadButtons::RightStick",
             "GamePadThumbSticks::Left", "GamePadThumbSticks::Right", "GamePadTriggers::Left",
             "GamePadTriggers::Right", "GamePadDPad::Up", "GamePadDPad::Down", "GamePadDPad::Left",
             "GamePadDPad::Right", "PlayerIndex"},
    .tags = {"gamepad", "controller", "xbox", "thumbstick", "trigger", "polling"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
