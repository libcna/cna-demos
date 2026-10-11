// SPDX-License-Identifier: MS-PL
// NetworkSessionProperties: eight optional integers that describe a session (mode, map, ...).
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    // A game gives each slot a meaning of its own.
    enum Slot
    {
        GameMode = 0,
        Map = 1,
        Difficulty = 2,
    };

    class SessionPropertiesDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            if (session_)
            {
                session_->Dispose();
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (session_ || Gamer::getSignedInGamersProperty()->getCountProperty() == 0)
            {
                return;
            }
            auto& ctx = Context();

            // Described before creating: on System Link and online these are what
            // NetworkSession::Find matches and what joining players see.
            NetworkSessionProperties properties;
            properties.setItem(GameMode, 2);
            properties.setItem(Map, 7);
            ctx.Check(properties.getCountProperty() == 8, "there are eight slots");
            ctx.Check(!properties.getItem(Difficulty).has_value(), "a slot left alone is empty (no value)");

            session_.reset(NetworkSession::Create(NetworkSessionType::Local, 1, 4, 0, properties));
            NetworkSessionProperties& live = session_->getSessionPropertiesProperty();
            ctx.Check(live.getItem(GameMode) == 2 && live.getItem(Map) == 7, "the session carries the properties");

            // The host may change them later, e.g. when the players vote for another map.
            live.setItem(Map, 3);
            ctx.Check(session_->getSessionPropertiesProperty().getItem(Map) == 3, "the host changed the map");

            bool refused = false;
            try
            {
                (void)live.getItem(8);
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                refused = true;
            }
            ctx.Check(refused, "slot 8 does not exist: ArgumentOutOfRangeException");

            for (int i = 0; i < live.getCountProperty(); ++i)
            {
                const std::optional<int>& value = live.getItem(i);
                lines_.push_back("[" + std::to_string(i) + "] " + (value ? std::to_string(*value) : std::string("(empty)")));
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(40, 36, 30));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "NetworkSession::SessionProperties", Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "0 = game mode, 1 = map, 2 = difficulty", Vector2(40.0f, 60.0f), Color::Gray);
            float y = 100.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::Wheat);
                y += 26.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<NetworkSession> session_;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SessionPropertiesDemo, {
    .id = "net.network_session.session_properties",
    .title = "Session properties",
    .description = "Describes a session with NetworkSessionProperties -- eight optional integers a game gives "
                   "meanings (here game mode and map) -- creates it with them, and changes the map as the host.",
    .apis = {"NetworkSessionProperties::NetworkSessionProperties", "NetworkSessionProperties::Item", "NetworkSessionProperties::Count",
             "NetworkSessionProperties::Count", "NetworkSession::SessionProperties", "NetworkSession::Create"},
    .tags = {"net", "session", "properties", "matchmaking"},
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
