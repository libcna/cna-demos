// SPDX-License-Identifier: MS-PL
// CNAEXT Mouse::ClickedEXT: an event per button press, raised while input is processed.
#include <array>
#include <deque>
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
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/MulticastAction.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    // The handler receives the button: 0 left, 1 middle, 2 right, 3 and 4 the extra buttons.
    constexpr std::array<std::string_view, 5> ButtonNames = {"left", "middle", "right", "X1", "X2"};

    class ClickedEventDemo final : public CnaDemos::DemoGame
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

            // Every press is reported, even two within one frame that polling GetState would
            // see as a single press -- or miss when press and release fall between two frames.
            clickedToken_ = Mouse::ClickedEXT.Add([this](int button) {
                ++clicks_;
                log_.push_front(std::string(button >= 0 && button < static_cast<int>(ButtonNames.size())
                                                ? ButtonNames[static_cast<std::size_t>(button)]
                                                : "other") +
                                " button (" + std::to_string(button) + ")");
                if (log_.size() > 8)
                    log_.pop_back();
            });
        }

        void UnloadContent() override
        {
            // ClickedEXT is static: unsubscribe before this game, which the handler uses, is gone.
            Mouse::ClickedEXT.Remove(clickedToken_);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Click any mouse button -- " + std::to_string(clicks_) + " click(s)",
                                     Vector2(40.0f, 30.0f), Color::White);
            float y = 80.0f;
            for (const std::string& entry : log_)
            {
                spriteBatch_->DrawString(*font_, entry, Vector2(60.0f, y), Color::LightGray);
                y += 30.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        System::MulticastAction<int>::Token clickedToken_ = System::MulticastAction<int>::InvalidToken;
        int clicks_ = 0;
        std::deque<std::string> log_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ClickedEventDemo, {
    .id = "input.mouse.clicked_event",
    .title = "Click event (CNAEXT)",
    .description = "Subscribes to Mouse::ClickedEXT, which reports every button press with the button number, and "
                   "lists the latest clicks; the subscription is removed when the game unloads.",
    .apis = {"Mouse::ClickedEXT"},
    .tags = {"mouse", "click", "event", "callback", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Mouse,
    .assets = {"fonts/pixel_font.cnb"},
})
