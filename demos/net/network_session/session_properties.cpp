// SPDX-License-Identifier: MS-PL
// NetworkSessionProperties: eight optional integers that describe a session (mode, map, ...).
#include <algorithm>
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
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"

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

            std::vector<std::optional<int>> ranged;
            for (const std::optional<int>& value : properties)
            {
                ranged.push_back(value);
            }
            ctx.Check(!properties.getIsReadOnlyProperty() && properties.Contains(2) &&
                          properties.Contains(std::nullopt) && properties.IndexOf(7) == Map &&
                          properties.IndexOf(99) == -1 && ranged.size() == 8 && ranged[GameMode] == 2 &&
                          ranged[Map] == 7 && !ranged[Difficulty].has_value(),
                      "lookup and C++ range iteration preserve the fixed eight-slot value shape");

            std::unique_ptr<System::Collections::Generic::IEnumerator<std::optional<int>>> cursor(
                properties.GetEnumerator());
            bool beforeStartRefused = false;
            try
            {
                static_cast<void>(cursor->Current());
            }
            catch (const System::InvalidOperationException&)
            {
                beforeStartRefused = true;
            }
            std::vector<std::optional<int>> enumerated;
            while (cursor->MoveNext())
            {
                enumerated.push_back(cursor->Current());
            }
            bool afterEndRefused = false;
            try
            {
                static_cast<void>(cursor->Current());
            }
            catch (const System::InvalidOperationException&)
            {
                afterEndRefused = true;
            }
            ctx.Check(beforeStartRefused && afterEndRefused && enumerated == ranged,
                      "the explicit enumerator visits the same eight slots and guards both invalid positions");

            cursor->Reset();
            properties.setItem(GameMode, 4);
            ctx.Check(cursor->MoveNext() && cursor->Current() == 4,
                      "Reset restarts the explicit enumerator and fixed-slot writes remain visible");
            properties.setItem(GameMode, 2);

            std::vector<std::optional<int>> shortOutput(9, -99);
            bool shortCopyRefused = false;
            try
            {
                properties.CopyTo(shortOutput, 2);
            }
            catch (const System::ArgumentException&)
            {
                shortCopyRefused = true;
            }
            std::vector<std::optional<int>> copied(10, -99);
            properties.CopyTo(copied, 1);
            ctx.Check(shortCopyRefused &&
                          std::all_of(shortOutput.begin(), shortOutput.end(),
                                      [](const std::optional<int>& value) { return value == -99; }) &&
                          copied.front() == -99 && copied.back() == -99 && copied[1] == 2 && copied[2] == 7 &&
                          !copied[3].has_value(),
                      "CopyTo is atomic when short and copies all eight slots at an offset");

            int structuralRefusals = 0;
            const auto expectStructuralRefusal = [&structuralRefusals](auto&& mutation) {
                try
                {
                    mutation();
                }
                catch (const System::NotSupportedException&)
                {
                    ++structuralRefusals;
                }
            };
            expectStructuralRefusal([&properties] { properties.Add(9); });
            expectStructuralRefusal([&properties] { properties.Insert(0, 9); });
            expectStructuralRefusal([&properties] { (void)properties.Remove(2); });
            expectStructuralRefusal([&properties] { properties.RemoveAt(0); });
            expectStructuralRefusal([&properties] { properties.Clear(); });
            ctx.Check(structuralRefusals == 5 && properties.getCountProperty() == 8 &&
                          properties.getItem(GameMode) == 2 && properties.getItem(Map) == 7 &&
                          !properties.getItem(Difficulty).has_value(),
                      "all structural mutations are refused without changing the fixed collection");

            session_.reset(NetworkSession::Create(NetworkSessionType::Local, 1, 4, 0, properties));
            NetworkSessionProperties& live = session_->getSessionPropertiesProperty();
            ctx.Check(live.getItem(GameMode) == 2 && live.getItem(Map) == 7, "the session carries the properties");

            // The host may change them later, e.g. when the players vote for another map.
            live.setItem(Map, 3);
            ctx.Check(session_->getSessionPropertiesProperty().getItem(Map) == 3, "the host changed the map");

            bool indexRefused = false;
            try
            {
                (void)live.getItem(8);
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                indexRefused = true;
            }
            ctx.Check(indexRefused, "slot 8 does not exist: ArgumentOutOfRangeException");

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
                   "meanings (here game mode and map) -- demonstrates fixed-shape lookup, range and copy behavior, "
                   "refuses structural mutations, creates a session with the values, and changes the live map.",
    .apis = {"NetworkSessionProperties::NetworkSessionProperties", "NetworkSessionProperties::Item",
             "NetworkSessionProperties::Count", "NetworkSessionProperties::IsReadOnly",
             "NetworkSessionProperties::Contains", "NetworkSessionProperties::IndexOf",
             "NetworkSessionProperties::CopyTo", "NetworkSessionProperties::Add",
             "NetworkSessionProperties::Insert", "NetworkSessionProperties::Remove",
             "NetworkSessionProperties::RemoveAt", "NetworkSessionProperties::Clear",
             "NetworkSessionProperties::GetEnumerator",
             "NetworkSession::SessionProperties", "NetworkSession::Create"},
    .tags = {"net", "session", "properties", "matchmaking"},
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
