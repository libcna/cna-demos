// SPDX-License-Identifier: MS-PL
// GamePadDeadZone: how GetState filters the small offsets of a stick at rest.
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
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadDeadZone.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    constexpr int PanelSize = 200;

    class DeadZonesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // The dead-zone sizes are XInput's (CNA exposes them, and the per-axis filter, as
            // CNAEXT): offsets inside the zone read as zero, the rest is rescaled to start at zero.
            const float zone = GamePad::LeftDeadZone;
            Context().Print("LeftDeadZone {:.4f}, RightDeadZone {:.4f}, TriggerThreshold {:.4f}", GamePad::LeftDeadZone,
                            GamePad::RightDeadZone, GamePad::TriggerThreshold);
            Context().Check(GamePad::ExcludeAxisDeadZone(zone * 0.9f, zone) == 0.0f,
                            "an offset inside the dead zone reads as 0");
            Context().Check(GamePad::ExcludeAxisDeadZone(1.0f, zone) == 1.0f &&
                                GamePad::ExcludeAxisDeadZone(-1.0f, zone) == -1.0f,
                            "full deflection still reads as -1 and 1");
            Context().CheckNear(GamePad::ExcludeAxisDeadZone(zone + 0.01f, zone), 0.01 / (1.0 - zone), 1e-5,
                                "just outside the zone the value starts near 0 -- no jump");
        }

        void Update(GameTime& gameTime) override
        {
            // The same controller read three ways. GetState(player) alone uses IndependentAxes.
            const std::array<GamePadDeadZone, 3> modes = {GamePadDeadZone::None, GamePadDeadZone::IndependentAxes,
                                                          GamePadDeadZone::Circular};
            for (std::size_t i = 0; i < modes.size(); ++i)
                sticks_[i] = GamePad::GetState(PlayerIndex::One, modes[i]).getThumbSticksProperty().getLeftProperty();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Player one's left stick, read with each GamePadDeadZone",
                                     Vector2(30.0f, 20.0f), Color::White);
            const std::array<const char*, 3> names = {"None", "IndependentAxes", "Circular"};
            for (std::size_t i = 0; i < 3; ++i)
            {
                const int left = 50 + static_cast<int>(i) * 250;
                const Rectangle panel(left, 120, PanelSize, PanelSize);
                spriteBatch_->Draw(*pixel_, panel, Color::DimGray);
                // The zone around the centre: a cross for per-axis filtering, a square standing in
                // for the circle of the circular mode; raw values (None) are not filtered at all.
                const int zone = static_cast<int>(GamePad::LeftDeadZone * PanelSize / 2);
                const int centreX = left + PanelSize / 2;
                const int centreY = 120 + PanelSize / 2;
                if (i == 1)
                {
                    spriteBatch_->Draw(*pixel_, Rectangle(centreX - zone, 120, zone * 2, PanelSize), Color::SlateGray);
                    spriteBatch_->Draw(*pixel_, Rectangle(left, centreY - zone, PanelSize, zone * 2), Color::SlateGray);
                }
                else if (i == 2)
                {
                    spriteBatch_->Draw(*pixel_, Rectangle(centreX - zone, centreY - zone, zone * 2, zone * 2),
                                       Color::SlateGray);
                }
                const Vector2 stick = sticks_[i];
                const int x = centreX + static_cast<int>(stick.X * (PanelSize / 2 - 6));
                const int y = centreY - static_cast<int>(stick.Y * (PanelSize / 2 - 6));
                spriteBatch_->Draw(*pixel_, Rectangle(x - 6, y - 6, 12, 12), Color::White);
                spriteBatch_->DrawString(*font_, names[i], Vector2(static_cast<float>(left), 90.0f), Color::White);
                spriteBatch_->DrawString(*font_, Format(stick.X) + ", " + Format(stick.Y),
                                         Vector2(static_cast<float>(left), 335.0f), Color::LightGray);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        static std::string Format(float value)
        {
            std::string text = std::to_string(value);
            return text.substr(0, text.find('.') + 4);
        }

        std::array<Vector2, 3> sticks_{};
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(DeadZonesDemo, {
    .id = "input.gamepad.dead_zones",
    .title = "Thumbstick dead zones",
    .description = "Reads player one's left stick with GamePad::GetState and each GamePadDeadZone (None, "
                   "IndependentAxes, Circular) side by side, and checks the per-axis filter "
                   "GamePad::ExcludeAxisDeadZone (CNAEXT) with XInput's dead-zone size.",
    .apis = {"GamePad::GetState", "GamePadDeadZone", "GamePadDeadZone::None",
             "GamePadDeadZone::IndependentAxes", "GamePadDeadZone::Circular",
             "GamePad::ExcludeAxisDeadZone", "GamePad::LeftDeadZone",
             "GamePad::RightDeadZone", "GamePad::TriggerThreshold", "GamePadThumbSticks::Left"},
    .tags = {"gamepad", "controller", "thumbstick", "dead-zone", "drift"},
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
