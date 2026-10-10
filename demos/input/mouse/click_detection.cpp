// SPDX-License-Identifier: MS-PL
// Clicks from polling: a button was clicked when it is pressed now and was released last frame.
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class ClickDetectionDemo final : public CnaDemos::DemoGame
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
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
        }

        void Update(GameTime& gameTime) override
        {
            const MouseState current = Mouse::GetState();
            const Point at(current.getXProperty(), current.getYProperty());

            // Left click: drop a marker where the button went down.
            if (current.getLeftButtonProperty() == ButtonState::Pressed &&
                previous_.getLeftButtonProperty() == ButtonState::Released)
            {
                markers_.push_back(at);
                dragging_ = true;
            }
            // Holding the button down and moving drags the newest marker...
            if (dragging_ && current.getLeftButtonProperty() == ButtonState::Pressed)
                markers_.back() = at;
            // ...and the release ends the drag.
            if (current.getLeftButtonProperty() == ButtonState::Released &&
                previous_.getLeftButtonProperty() == ButtonState::Pressed)
                dragging_ = false;

            // Right click removes every marker.
            if (current.getRightButtonProperty() == ButtonState::Pressed &&
                previous_.getRightButtonProperty() == ButtonState::Released)
                markers_.clear();

            previous_ = current;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Left click: drop a marker (hold to drag it)", Vector2(40.0f, 30.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "Right click: remove all", Vector2(40.0f, 60.0f), Color::White);
            spriteBatch_->DrawString(*font_, std::to_string(markers_.size()) + " marker(s)", Vector2(40.0f, 90.0f),
                                     Color::LightGray);
            for (std::size_t i = 0; i < markers_.size(); ++i)
            {
                const bool held = dragging_ && i + 1 == markers_.size();
                spriteBatch_->Draw(*pixel_, Rectangle(markers_[i].X - 8, markers_[i].Y - 8, 16, 16),
                                   held ? Color::Yellow : Color::Orange);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        MouseState previous_;
        std::vector<Point> markers_;
        bool dragging_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(ClickDetectionDemo, {
    .id = "input.mouse.click_detection",
    .title = "Mouse clicks and dragging",
    .description = "Finds clicks, drags and releases by comparing Mouse::GetState with the previous frame's "
                   "MouseState: left click drops a marker that follows the cursor while held, right click clears.",
    .apis = {"Mouse::GetState", "MouseState::LeftButton", "MouseState::RightButton", "MouseState::X",
             "MouseState::Y", "ButtonState"},
    .tags = {"mouse", "click", "drag", "pressed", "released", "edge"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Mouse,
    .assets = {"fonts/pixel_font.cnb"},
})
