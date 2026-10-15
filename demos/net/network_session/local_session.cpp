// SPDX-License-Identifier: MS-PL
// NetworkSession of type Local: a split-screen session of the players on one machine.
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
#include "Microsoft/Xna/Framework/Net/GamerJoinedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkMachine.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionState.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    class LocalSessionDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        LocalSessionDemo() : GamerServicesDemo("Demo Player,Second Player") {}

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
                session_->GamerJoined.Remove(joinedToken_);
                session_->Dispose(); // the caller owns the session: dispose, then delete
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            if (!session_)
            {
                if (Gamer::getSignedInGamersProperty()->getCountProperty() < 2)
                {
                    if (++waited_ == 30)
                    {
                        throw CnaDemos::SkipDemo("two signed-in players are needed (CNA_GAMER_SERVICES_AUTO_SIGN_IN)");
                    }
                    return;
                }
                // Up to two players on this machine, four in all. A Local session never uses
                // the network: it is how a split-screen game uses the same code as an online one.
                bool aboveMaximumRefused = false;
                std::unique_ptr<NetworkSession> invalid;
                try
                {
                    invalid.reset(NetworkSession::Create(
                        NetworkSessionType::Local, 2, NetworkSession::MaxSupportedGamers + 1));
                }
                catch (const System::ArgumentOutOfRangeException&)
                {
                    aboveMaximumRefused = true;
                }
                if (invalid)
                {
                    invalid->Dispose();
                }
                ctx.Check(NetworkSession::MaxSupportedGamers == 31 && aboveMaximumRefused,
                          "MaxSupportedGamers is 31 and the next capacity is refused atomically");
                session_.reset(NetworkSession::Create(NetworkSessionType::Local, 2, 4));
                joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                    const NetworkGamer* gamer = args.getGamerProperty();
                    joined_.push_back(gamer->getGamertagProperty() + (gamer->getIsLocalProperty() ? " (local)" : " (remote)"));
                });
                ctx.Check(!session_->getIsDisposedProperty() &&
                              session_->getSessionTypeProperty() == NetworkSessionType::Local &&
                              session_->getSessionStateProperty() == NetworkSessionState::Lobby,
                          "a new session is live and is a Local session in the Lobby");
                ctx.Check(session_->getIsHostProperty() && session_->getMaxGamersProperty() == 4,
                          "this machine hosts it, for at most four gamers");
                ctx.Check(session_->getLocalGamersProperty().getCountProperty() == 2 &&
                              session_->getRemoteGamersProperty().getCountProperty() == 0,
                          "both signed-in players joined as local gamers; there are no remote ones");
                ctx.Check(NetworkSession::MaxPreviousGamers == 100 &&
                              session_->getPreviousGamersProperty().getCountProperty() <= NetworkSession::MaxPreviousGamers,
                          "the public history capacity is 100 and bounds the PreviousGamers view");
                // As on XNA, subscribing to GamerJoined reports every gamer already in the
                // session at once, so a handler added after Create misses nobody.
                ctx.Check(joined_.size() == 2, "subscribing raised GamerJoined for both gamers already there");
            }

            if (disposed_)
            {
                return;
            }
            // A game calls Update on its session every frame: later events are raised there.
            session_->Update();
            ++liveUpdates_;
            if (!checked_)
            {
                checked_ = true;
                ctx.Check(joined_.size() == 2, "Update raised no second GamerJoined for them");
                const NetworkGamer* host = session_->getHostProperty();
                ctx.Check(host != nullptr && host->getIsLocalProperty(), "the host is a local gamer");
                host_ = host->getGamertagProperty();
                const NetworkGamer* first = session_->getAllGamersProperty()[0];
                const NetworkGamer* second = session_->getAllGamersProperty()[1];
                ctx.Check(first->getIdProperty() != second->getIdProperty() &&
                              session_->FindGamerById(second->getIdProperty()) == second,
                          "every gamer has its own Id, and FindGamerById finds it");
                ctx.Check(first->getMachineProperty().getGamersProperty().getCountProperty() == 2,
                          "both gamers are on the same NetworkMachine");
                for (const std::string& line : joined_)
                {
                    ctx.Print("GamerJoined: {}", line);
                }
            }
            if (liveUpdates_ == 10)
            {
                session_->Dispose();
                session_->Dispose();
                ctx.Check(session_->getIsDisposedProperty() && session_->getHostProperty() == nullptr &&
                              session_->getAllGamersProperty().getCountProperty() == 0 &&
                              session_->getLocalGamersProperty().getCountProperty() == 0 &&
                              session_->getRemoteGamersProperty().getCountProperty() == 0 &&
                              session_->getPreviousGamersProperty().getCountProperty() == 0,
                          "Dispose is idempotent, marks the session and clears every gamer view");
                bool updateRefused = false;
                try
                {
                    session_->Update();
                }
                catch (const System::ObjectDisposedException&)
                {
                    updateRefused = true;
                }
                ctx.Check(updateRefused, "a disposed session refuses Update with ObjectDisposedException");
                disposed_ = true;
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(26, 42, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "NetworkSession (Local)", Vector2(40.0f, 30.0f), Color::White);
            if (!host_.empty())
            {
                spriteBatch_->DrawString(*font_, "Host: " + host_, Vector2(40.0f, 70.0f), Color::Khaki);
            }
            float y = 110.0f;
            for (const std::string& line : joined_)
            {
                spriteBatch_->DrawString(*font_, "GamerJoined: " + line, Vector2(40.0f, y), Color::LightGreen);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GamerJoinedEventArgs>::Token joinedToken_{};
        std::vector<std::string> joined_;
        std::string host_;
        int waited_ = 0;
        int liveUpdates_ = 0;
        bool checked_ = false;
        bool disposed_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(LocalSessionDemo, {
    .id = "net.network_session.local_session",
    .title = "Local network session",
    .description = "Creates a NetworkSession of type Local for two signed-in players: both join as local gamers of "
                   "the host machine, subscribing to GamerJoined reports both at once, each gamer has an Id that "
                   "FindGamerById resolves, and idempotent disposal marks the session and clears its gamer views. "
                   "No network is used.",
    .apis = {"NetworkSession::Create", "NetworkSession::Update", "NetworkSession::GamerJoined",
             "NetworkSession::SessionType", "NetworkSession::SessionState", "NetworkSession::IsHost",
             "NetworkSession::MaxGamers", "NetworkSession::MaxPreviousGamers", "NetworkSession::MaxSupportedGamers",
             "NetworkSession::LocalGamers", "NetworkSession::RemoteGamers",
             "NetworkSession::AllGamers", "NetworkSession::Host", "NetworkSession::FindGamerById",
             "NetworkGamer::Id", "NetworkGamer::IsHost", "NetworkGamer::IsLocal", "NetworkGamer::Machine",
             "NetworkMachine::Gamers", "GamerJoinedEventArgs::Gamer", "NetworkSession::Dispose",
             "NetworkSession::IsDisposed",
             "GamerCollection::Count", "GamerCollection::Item"},
    .tags = {"net", "session", "split-screen", "multiplayer", "local"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
