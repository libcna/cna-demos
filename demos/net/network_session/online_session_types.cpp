// SPDX-License-Identifier: MS-PL
// Session types and the online service: Local works offline; PlayerMatch and Ranked need the service.
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerServicesNotAvailableException.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPrivileges.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    class OnlineSessionTypesDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            if (done_)
            {
                return;
            }
            // The profile signs in during the first frames (GamerServicesComponent).
            if (Gamer::getSignedInGamersProperty()->getCountProperty() == 0)
            {
                if (++waited_ == 30)
                    throw CnaDemos::SkipDemo("no signed-in profile (CNA_GAMER_SERVICES_AUTO_SIGN_IN)");
                return;
            }
            done_ = true;
            SignedInGamer* gamer = (*Gamer::getSignedInGamersProperty())[0];
            // A local profile has no online account: the privilege that online play needs is off.
            const bool online = gamer->getPrivilegesProperty().getAllowOnlineSessionsProperty();
            lines_.push_back(gamer->getGamertagProperty() + ": AllowOnlineSessions " + (online ? "true" : "false"));
            ctx.Check(!online, "a local profile is not allowed online sessions");

            // Local: any profile can play on one machine.
            std::unique_ptr<NetworkSession> local(NetworkSession::Create(NetworkSessionType::Local, 1, 4));
            ctx.Check(local != nullptr, "a Local session is created for a local profile");
            lines_.push_back("Local: created");
            local->Dispose();

            // PlayerMatch and Ranked go through the CNA online service. The demos run CNA's gamer
            // services offline (no service configured), so both are refused before any account is
            // looked at; with a service, a profile without AllowOnlineSessions would get a
            // GamerPrivilegeException instead.
            for (const auto& [type, name] : {std::pair{NetworkSessionType::PlayerMatch, "PlayerMatch"},
                                             std::pair{NetworkSessionType::Ranked, "Ranked"}})
            {
                std::string outcome = "created (unexpected)";
                bool refused = false;
                try
                {
                    std::unique_ptr<NetworkSession> session(NetworkSession::Create(type, 1, 4));
                    session->Dispose();
                }
                catch (const GamerServicesNotAvailableException& error)
                {
                    refused = true;
                    outcome = std::string("GamerServicesNotAvailableException: ") + error.what();
                }
                catch (const std::exception& error)
                {
                    outcome = std::string("other exception: ") + error.what();
                }
                lines_.push_back(std::string(name) + ": " + (refused ? "GamerServicesNotAvailableException" : outcome));
                ctx.Print("{}: {}", name, outcome);
                ctx.Check(refused, std::string(name) + " needs the online service: GamerServicesNotAvailableException");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(34, 36, 50));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "NetworkSession::Create by session type", Vector2(30.0f, 30.0f), Color::White);
            float y = 90.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::LightGray);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        bool done_ = false;
        int waited_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(OnlineSessionTypesDemo, {
    .id = "net.network_session.online_session_types",
    .title = "Session types and the online service",
    .description = "With CNA's gamer services offline and a local profile signed in, creates a Local session "
                   "(no service needed) and tries PlayerMatch and Ranked, which need the online service and are "
                   "refused with GamerServicesNotAvailableException; the profile's "
                   "GamerPrivileges::AllowOnlineSessions is false as well.",
    .apis = {"NetworkSession::Create", "NetworkSessionType::Local", "NetworkSessionType::PlayerMatch",
             "NetworkSessionType::Ranked", "GamerServicesNotAvailableException",
             "GamerPrivileges::AllowOnlineSessions"},
    .tags = {"net", "session", "online", "privileges", "gamer-services"},
    .frames = 40,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
