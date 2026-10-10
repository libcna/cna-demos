// SPDX-License-Identifier: MS-PL
// CNAEXT extra controller buttons: back paddles and the share/capture button (Misc1).
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

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
    struct Extra
    {
        Buttons button;
        bool (GamePadCapabilities::*has)() const;
        std::string_view label;
    };

    constexpr std::array<Extra, 5> Extras = {{
        {Buttons::Paddle1EXT, &GamePadCapabilities::getHasPaddle1EXTProperty, "Paddle1EXT"},
        {Buttons::Paddle2EXT, &GamePadCapabilities::getHasPaddle2EXTProperty, "Paddle2EXT"},
        {Buttons::Paddle3EXT, &GamePadCapabilities::getHasPaddle3EXTProperty, "Paddle3EXT"},
        {Buttons::Paddle4EXT, &GamePadCapabilities::getHasPaddle4EXTProperty, "Paddle4EXT"},
        {Buttons::Misc1EXT, &GamePadCapabilities::getHasMisc1EXTProperty, "Misc1EXT (share, capture, mic)"},
    }};

    class ExtraButtonsDemo final : public CnaDemos::DemoGame
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
            caps_ = GamePad::GetCapabilities(PlayerIndex::One);
            pad_ = GamePad::GetState(PlayerIndex::One);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Buttons beyond the Xbox 360 layout, on controller one", Vector2(30.0f, 25.0f),
                                     Color::White);
            for (std::size_t i = 0; i < Extras.size(); ++i)
            {
                // The capability says whether the button exists; IsButtonDown reads it like any other.
                const bool exists = (caps_.*Extras[i].has)();
                const bool down = pad_.IsButtonDown(Extras[i].button);
                const int y = 80 + static_cast<int>(i) * 50;
                spriteBatch_->Draw(*pixel_, Rectangle(30, y, 36, 36), down ? Color::LimeGreen : Color::DimGray);
                spriteBatch_->DrawString(*font_, std::string(Extras[i].label) + (exists ? "" : "  -- not on this controller"),
                                         Vector2(80.0f, static_cast<float>(y + 8)), exists ? Color::White : Color::Gray);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        GamePadCapabilities caps_;
        GamePadState pad_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(ExtraButtonsDemo, {
    .id = "input.gamepad.extra_buttons",
    .title = "Paddles and extra buttons (CNAEXT)",
    .description = "Shows the back paddles (Buttons::Paddle1EXT to Paddle4EXT) and the share/capture button "
                   "(Misc1EXT) of controller one, with the HasPaddle and HasMisc1 capabilities saying which exist.",
    .apis = {"Buttons::Paddle1EXT", "Buttons::Paddle2EXT", "Buttons::Paddle3EXT", "Buttons::Paddle4EXT",
             "Buttons::Misc1EXT", "GamePadCapabilities::HasPaddle1EXT", "GamePadCapabilities::HasPaddle2EXT",
             "GamePadCapabilities::HasPaddle3EXT", "GamePadCapabilities::HasPaddle4EXT",
             "GamePadCapabilities::HasMisc1EXT", "GamePadState::IsButtonDown"},
    .tags = {"gamepad", "controller", "paddles", "elite", "share", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
