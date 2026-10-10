// SPDX-License-Identifier: MS-PL
// Mouse::SetPosition: move the cursor -- and the classic XNA mouse look built on re-centring it.
#include <cmath>
#include <memory>
#include <optional>

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
    constexpr int CentreX = 400;
    constexpr int CentreY = 240;

    class SetPositionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // The new position is in window coordinates and GetState reports it at once.
            Mouse::SetPosition(123, 77);
            const MouseState moved = Mouse::GetState();
            Context().Print("after SetPosition(123, 77): X {}, Y {}", moved.getXProperty(), moved.getYProperty());
            Context().Check(moved.getXProperty() == 123 && moved.getYProperty() == 77,
                            "GetState reports the position SetPosition moved the cursor to");
            setIsMouseVisibleProperty(true);
        }

        void Update(GameTime& gameTime) override
        {
            // Mouse look, the XNA way: while the right button is held, read how far the cursor
            // moved from the window centre, then put it back there. The cursor never reaches the
            // edge of the window, so the view can turn forever.
            const MouseState mouse = Mouse::GetState();
            const bool looking = getIsActiveProperty() && mouse.getRightButtonProperty() == ButtonState::Pressed;
            if (looking && wasLooking_)
            {
                yaw_ += static_cast<float>(mouse.getXProperty() - CentreX);
                pitch_ += static_cast<float>(mouse.getYProperty() - CentreY);
            }
            if (looking)
                Mouse::SetPosition(CentreX, CentreY);
            setIsMouseVisibleProperty(!looking);
            wasLooking_ = looking;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // A grid that scrolls with the accumulated movement stands in for a 3D view.
            const int offsetX = static_cast<int>(std::fmod(-yaw_, 80.0f) + 80.0f) % 80;
            const int offsetY = static_cast<int>(std::fmod(-pitch_, 80.0f) + 80.0f) % 80;
            for (int x = offsetX - 80; x < 800; x += 80)
                spriteBatch_->Draw(*pixel_, Rectangle(x, 0, 2, 480), Color::DarkGreen);
            for (int y = offsetY - 80; y < 480; y += 80)
                spriteBatch_->Draw(*pixel_, Rectangle(0, y, 800, 2), Color::DarkGreen);
            spriteBatch_->Draw(*pixel_, Rectangle(CentreX - 10, CentreY, 21, 1), Color::White);
            spriteBatch_->Draw(*pixel_, Rectangle(CentreX, CentreY - 10, 1, 21), Color::White);
            spriteBatch_->DrawString(*font_, "Hold the right mouse button and move the mouse to look around",
                                     Vector2(20.0f, 20.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        float yaw_ = 0.0f;
        float pitch_ = 0.0f;
        bool wasLooking_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(SetPositionDemo, {
    .id = "input.mouse.set_position",
    .title = "Move the cursor (mouse look)",
    .description = "Mouse::SetPosition moves the cursor and GetState reports the new position at once; holding the "
                   "right button turns the view by re-centring the cursor every frame, XNA's usual mouse look.",
    .apis = {"Mouse::SetPosition", "Mouse::GetState", "Game::IsActive", "Game::IsMouseVisible"},
    .tags = {"mouse", "cursor", "warp", "mouse-look", "camera", "fps"},
    .needs = CnaDemos::Need::Mouse,
    .assets = {"fonts/pixel_font.cnb"},
})
