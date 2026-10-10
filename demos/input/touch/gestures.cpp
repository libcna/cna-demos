// SPDX-License-Identifier: MS-PL
// Touch gestures: enable the ones the game wants, then read what the recognizer found each frame.
#include <algorithm>
#include <deque>
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
#include "Microsoft/Xna/Framework/Input/Touch/GestureSample.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/GestureType.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input::Touch;

namespace
{
    class GesturesDemo final : public CnaDemos::DemoGame
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

            // Only enabled gestures are recognized; the default is GestureType::None.
            TouchPanel::setEnabledGesturesProperty(GestureType::Tap | GestureType::DoubleTap | GestureType::Hold |
                                                   GestureType::FreeDrag | GestureType::DragComplete |
                                                   GestureType::Flick | GestureType::Pinch |
                                                   GestureType::PinchComplete);
            // CNAEXT: the left mouse button acts as one finger (no pinch with a mouse).
            emulationWas_ = TouchPanel::getMouseTouchEmulationEnabledEXT();
            TouchPanel::setMouseTouchEmulationEnabledEXT(true);
        }

        void UnloadContent() override
        {
            TouchPanel::setEnabledGesturesProperty(GestureType::None);
            TouchPanel::setMouseTouchEmulationEnabledEXT(emulationWas_);
        }

        void Update(GameTime& gameTime) override
        {
            const float seconds = static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
            // Every gesture recognized since the last frame waits in a queue.
            while (TouchPanel::getIsGestureAvailableProperty())
            {
                const GestureSample gesture = TouchPanel::ReadGesture();
                switch (gesture.getGestureTypeProperty())
                {
                case GestureType::Tap:
                    tinted_ = !tinted_;
                    break;
                case GestureType::DoubleTap:
                    size_ = 80.0f;
                    break;
                case GestureType::Hold:
                    position_ = Vector2(400.0f, 280.0f);
                    velocity_ = Vector2::Zero;
                    break;
                case GestureType::FreeDrag:
                    position_ += gesture.getDeltaProperty();  // movement since the last drag sample
                    velocity_ = Vector2::Zero;
                    break;
                case GestureType::Flick:
                    velocity_ = gesture.getDeltaProperty();  // for a flick, Delta is pixels per second
                    break;
                case GestureType::Pinch:
                {
                    // Two fingers: compare their distance now and one sample ago.
                    const float now = Vector2::Distance(gesture.getPositionProperty(), gesture.getPosition2Property());
                    const float before = Vector2::Distance(gesture.getPositionProperty() - gesture.getDeltaProperty(),
                                                           gesture.getPosition2Property() - gesture.getDelta2Property());
                    if (before > 1.0f)
                        size_ = std::clamp(size_ * now / before, 20.0f, 400.0f);
                    break;
                }
                default:
                    break;
                }
                log_.push_front(std::string(CnaDemos::Name(gesture.getGestureTypeProperty())) + " at " +
                                std::to_string(static_cast<int>(gesture.getPositionProperty().X)) + ", " +
                                std::to_string(static_cast<int>(gesture.getPositionProperty().Y)));
                if (log_.size() > 8)
                    log_.pop_back();
            }
            position_ += velocity_ * seconds;
            velocity_ *= 0.95f;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Drag or flick the square. Tap: colour, double tap: size,",
                                     Vector2(20.0f, 15.0f), Color::White);
            spriteBatch_->DrawString(*font_, "hold: back to the middle, pinch: resize (two fingers).",
                                     Vector2(20.0f, 40.0f), Color::White);
            const int half = static_cast<int>(size_ / 2.0f);
            spriteBatch_->Draw(*pixel_,
                               Rectangle(static_cast<int>(position_.X) - half, static_cast<int>(position_.Y) - half,
                                         half * 2, half * 2),
                               tinted_ ? Color::Orange : Color::CornflowerBlue);
            float y = 75.0f;
            for (const std::string& entry : log_)
            {
                spriteBatch_->DrawString(*font_, entry, Vector2(20.0f, y), Color::LightGray);
                y += 22.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool emulationWas_ = false;
        Vector2 position_{400.0f, 280.0f};
        Vector2 velocity_{};
        float size_ = 80.0f;
        bool tinted_ = false;
        std::deque<std::string> log_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(GesturesDemo, {
    .id = "input.touch.gestures",
    .title = "Touch gestures",
    .description = "Enables tap, double tap, hold, free drag, flick and pinch with TouchPanel::EnabledGestures and "
                   "reads them with IsGestureAvailable/ReadGesture to move, fling, recolour and resize a square; "
                   "the mouse can emulate one finger.",
    .apis = {"TouchPanel::EnabledGestures", "TouchPanel::IsGestureAvailable", "TouchPanel::ReadGesture",
             "GestureSample::GestureType", "GestureSample::Position", "GestureSample::Position2",
             "GestureSample::Delta", "GestureSample::Delta2", "GestureType"},
    .tags = {"touch", "gesture", "drag", "flick", "pinch", "tap", "phone"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Touch,
    .assets = {"fonts/pixel_font.cnb"},
})
