// SPDX-License-Identifier: MS-PL
// Mouse::GetState: cursor position, five buttons and the scroll wheel, polled once per frame.
#include <algorithm>
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
    class MouseStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            // XNA hides the system cursor over the game window unless asked otherwise.
            setIsMouseVisibleProperty(true);
            DemoGame::Initialize();
        }

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
            // Position is in back-buffer pixels from the window's top-left corner (and can lie
            // outside it). The wheel value is a running total: 120 per notch.
            mouse_ = Mouse::GetState();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);

            const int x = mouse_.getXProperty();
            const int y = mouse_.getYProperty();
            spriteBatch_->Draw(*pixel_, Rectangle(x - 20, y, 41, 1), Color::Yellow);
            spriteBatch_->Draw(*pixel_, Rectangle(x, y - 20, 1, 41), Color::Yellow);

            spriteBatch_->DrawString(*font_, "X " + std::to_string(x) + "  Y " + std::to_string(y),
                                     Vector2(40.0f, 30.0f), Color::White);
            const std::array<std::pair<std::string_view, ButtonState>, 5> buttons = {{
                {"Left", mouse_.getLeftButtonProperty()},
                {"Middle", mouse_.getMiddleButtonProperty()},
                {"Right", mouse_.getRightButtonProperty()},
                {"X1", mouse_.getXButton1Property()},
                {"X2", mouse_.getXButton2Property()},
            }};
            for (std::size_t i = 0; i < buttons.size(); ++i)
            {
                const int left = 40 + static_cast<int>(i) * 110;
                const bool pressed = buttons[i].second == ButtonState::Pressed;
                spriteBatch_->Draw(*pixel_, Rectangle(left, 70, 100, 40), pressed ? Color::LimeGreen : Color::DimGray);
                spriteBatch_->DrawString(*font_, std::string(buttons[i].first), Vector2(static_cast<float>(left + 8), 80.0f),
                                         Color::White);
            }

            const int wheel = mouse_.getScrollWheelValueProperty();
            spriteBatch_->DrawString(*font_, "ScrollWheelValue " + std::to_string(wheel) + " (" +
                                                 std::to_string(wheel / 120) + " notches)",
                                     Vector2(40.0f, 140.0f), Color::White);
            spriteBatch_->DrawString(*font_,
                                     "HorizontalScrollWheelValueEXT " +
                                         std::to_string(mouse_.getHorizontalScrollWheelValueEXTProperty()),
                                     Vector2(40.0f, 170.0f), Color::LightGray);
            // The wheel total moves a marker along a track, 20 pixels per notch.
            spriteBatch_->Draw(*pixel_, Rectangle(40, 220, 720, 4), Color::Gray);
            const int marker = 400 + std::clamp(wheel / 120 * 20, -360, 360);
            spriteBatch_->Draw(*pixel_, Rectangle(marker - 6, 210, 12, 24), Color::Orange);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        MouseState mouse_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(MouseStateDemo, {
    .id = "input.mouse.get_state",
    .title = "Mouse state",
    .description = "Polls Mouse::GetState every frame: a crosshair follows the cursor, five boxes light up with the "
                   "buttons and a marker moves with the scroll wheel total.",
    .apis = {"Mouse::GetState", "MouseState::X", "MouseState::Y", "MouseState::LeftButton", "MouseState::MiddleButton",
             "MouseState::RightButton", "MouseState::XButton1", "MouseState::XButton2", "MouseState::ScrollWheelValue",
             "MouseState::HorizontalScrollWheelValueEXT", "Game::IsMouseVisible", "ButtonState"},
    .tags = {"mouse", "cursor", "buttons", "wheel", "polling"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Mouse,
    .assets = {"fonts/pixel_font.cnb"},
})
