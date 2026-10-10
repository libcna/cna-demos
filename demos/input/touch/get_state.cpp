// SPDX-License-Identifier: MS-PL
// TouchPanel::GetState: every finger on the screen, tracked by Id from press to release.
#include <deque>
#include <map>
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
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocationState.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input::Touch;

namespace
{
    class TouchStateDemo final : public CnaDemos::DemoGame
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

            // CNAEXT: without a touch screen the left mouse button can stand in for one finger;
            // it then arrives exactly like a real touch. A process-wide switch, restored on unload.
            emulationWas_ = TouchPanel::getMouseTouchEmulationEnabledEXT();
            TouchPanel::setMouseTouchEmulationEnabledEXT(true);
        }

        void UnloadContent() override { TouchPanel::setMouseTouchEmulationEnabledEXT(emulationWas_); }

        void Update(GameTime& gameTime) override
        {
            // Each location keeps its Id while the finger stays down: Pressed in the first frame,
            // Moved while it is held (even without moving), Released in the last.
            touches_ = TouchPanel::GetState();
            for (const TouchLocation& touch : touches_)
            {
                std::deque<Vector2>& trail = trails_[touch.getIdProperty()];
                trail.push_back(touch.getPositionProperty());
                if (trail.size() > 40)
                    trail.pop_front();
                if (touch.getStateProperty() == TouchLocationState::Released)
                    trails_.erase(touch.getIdProperty());
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Touch the screen (or hold the left mouse button)", Vector2(20.0f, 15.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_,
                                     std::to_string(touches_.getCountProperty()) + " touch location(s), touch device " +
                                         (touches_.getIsConnectedProperty() ? "present" : "not reported"),
                                     Vector2(20.0f, 40.0f), Color::LightGray);

            for (const auto& [id, trail] : trails_)
            {
                for (const Vector2& point : trail)
                    spriteBatch_->Draw(*pixel_, Rectangle(static_cast<int>(point.X) - 2, static_cast<int>(point.Y) - 2, 4, 4),
                                       Color::SlateGray);
            }
            float y = 70.0f;
            for (const TouchLocation& touch : touches_)
            {
                const Vector2 at = touch.getPositionProperty();
                const Color colour = touch.getStateProperty() == TouchLocationState::Pressed    ? Color::LimeGreen
                                     : touch.getStateProperty() == TouchLocationState::Released ? Color::Red
                                                                                                : Color::Yellow;
                spriteBatch_->Draw(*pixel_, Rectangle(static_cast<int>(at.X) - 15, static_cast<int>(at.Y) - 15, 30, 30),
                                   colour);
                // The previous frame's location of the same finger, when there was one.
                TouchLocation previous;
                std::string moved;
                if (touch.TryGetPreviousLocation(previous))
                    moved = ", moved " + std::to_string(static_cast<int>((at - previous.getPositionProperty()).Length())) +
                            " px";
                spriteBatch_->DrawString(*font_,
                                         "Id " + std::to_string(touch.getIdProperty()) + ": " +
                                             std::string(CnaDemos::Name(touch.getStateProperty())) + " at " +
                                             std::to_string(static_cast<int>(at.X)) + ", " +
                                             std::to_string(static_cast<int>(at.Y)) + moved,
                                         Vector2(20.0f, y), colour);
                y += 24.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool emulationWas_ = false;
        TouchCollection touches_;
        std::map<int, std::deque<Vector2>> trails_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(TouchStateDemo, {
    .id = "input.touch.get_state",
    .title = "Touch state",
    .description = "Polls TouchPanel::GetState every frame and draws each TouchLocation by Id and state with its "
                   "trail and the distance from TryGetPreviousLocation; the mouse can emulate a finger "
                   "(MouseTouchEmulationEnabledEXT).",
    .apis = {"TouchPanel::GetState", "TouchCollection::Count", "TouchCollection::IsConnected",
             "TouchCollection::GetEnumerator", "TouchLocation::Id", "TouchLocation::State", "TouchLocation::Position",
             "TouchLocation::TryGetPreviousLocation", "TouchLocationState",
             "TouchPanel::MouseTouchEmulationEnabledEXT"},
    .tags = {"touch", "multitouch", "finger", "phone", "tablet"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Touch,
    .assets = {"fonts/pixel_font.cnb"},
})
