// SPDX-License-Identifier: MS-PL
// CNAEXT hot-plug events: joysticks, mice and keyboards connecting and disconnecting.
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CNA/Input/InputDevices.hpp"
#include "CNA/Input/Joysticks.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/MulticastAction.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace CNA::Input;

namespace
{
    using Event = System::MulticastAction<std::uint32_t>;

    class HotPlugDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // XNA games poll GamePadState::IsConnected; these events say when it changes, with
            // the device id. They fire on the game thread while input is processed -- and at
            // start-up the devices already present arrive as connections too.
            Subscribe(Joysticks::ConnectedEXT, "joystick connected");
            Subscribe(Joysticks::DisconnectedEXT, "joystick disconnected");
            Subscribe(InputDevices::MouseConnectedEXT, "mouse connected");
            Subscribe(InputDevices::MouseDisconnectedEXT, "mouse disconnected");
            Subscribe(InputDevices::KeyboardConnectedEXT, "keyboard connected");
            Subscribe(InputDevices::KeyboardDisconnectedEXT, "keyboard disconnected");
        }

        void UnloadContent() override
        {
            // The events are static: remove every handler before the game they use is gone.
            for (const auto& [event, token] : subscriptions_)
                event->Remove(token);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Plug in or unplug a controller, mouse or keyboard", Vector2(30.0f, 20.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "(devices present at start-up are reported as connected)",
                                     Vector2(30.0f, 45.0f), Color::Gray);
            float y = 85.0f;
            if (log_.empty())
                spriteBatch_->DrawString(*font_, "(no events yet)", Vector2(30.0f, y), Color::Gray);
            for (const std::string& entry : log_)
            {
                spriteBatch_->DrawString(*font_, entry, Vector2(30.0f, y), Color::LightGray);
                y += 26.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void Subscribe(Event& event, const std::string& what)
        {
            subscriptions_.emplace_back(&event, event.Add([this, what](std::uint32_t id) {
                const std::string entry = what + ", id " + std::to_string(id);
                Context().Print("{}", entry);
                log_.push_front(entry);
                if (log_.size() > 12)
                    log_.pop_back();
            }));
        }

        std::vector<std::pair<Event*, Event::Token>> subscriptions_;
        std::deque<std::string> log_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(HotPlugDemo, {
    .id = "input.devices.hot_plug",
    .title = "Device hot-plug events (CNAEXT)",
    .description = "Subscribes to Joysticks::ConnectedEXT/DisconnectedEXT and the mouse and keyboard events of "
                   "InputDevices and logs each with its device id; every handler is removed on unload.",
    .apis = {"CNA::Input::Joysticks::ConnectedEXT", "CNA::Input::Joysticks::DisconnectedEXT",
             "CNA::Input::InputDevices::MouseConnectedEXT", "CNA::Input::InputDevices::MouseDisconnectedEXT",
             "CNA::Input::InputDevices::KeyboardConnectedEXT", "CNA::Input::InputDevices::KeyboardDisconnectedEXT"},
    .tags = {"hot-plug", "connect", "disconnect", "joystick", "mouse", "keyboard", "event", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
