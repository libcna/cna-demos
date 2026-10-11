// SPDX-License-Identifier: MS-PL
// NetworkSession::AddLocalGamer: another player on the same machine joins a session already
// running -- "press Start to join" -- and AllowJoinInProgress decides whether that works mid-game.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Net/GamerJoinedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    constexpr const char* Players = "Demo Player,Second Player,Third Player";

    class AddLocalGamerDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        AddLocalGamerDemo() : GamerServicesDemo(Players) {}

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
                session_->Dispose();
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (done_)
            {
                session_->Update();
                return;
            }
            const SignedInGamerCollection& signedIn = *Gamer::getSignedInGamersProperty();
            if (signedIn.getCountProperty() < 3)
            {
                // Only when CNA_GAMER_SERVICES_AUTO_SIGN_IN names fewer players than this demo asks.
                if (++waited_ == 30)
                {
                    throw CnaDemos::SkipDemo("three signed-in players are needed (CNA_GAMER_SERVICES_AUTO_SIGN_IN)");
                }
                return;
            }
            done_ = true;
            auto& ctx = Context();
            SignedInGamer* first = signedIn[PlayerIndex::One];
            SignedInGamer* second = signedIn[PlayerIndex::Two];
            SignedInGamer* third = signedIn[PlayerIndex::Three];

            // Player one starts a split-screen session alone.
            session_.reset(NetworkSession::Create(NetworkSessionType::Local, std::vector<SignedInGamer*>{first}, 4, 0,
                                                  NetworkSessionProperties()));
            joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                log_.push_back("GamerJoined: " + args.getGamerProperty()->getGamertagProperty());
            });
            session_->Update();
            ctx.Check(session_->getLocalGamersProperty().getCountProperty() == 1, "the session starts with player one");

            // In the lobby, player two presses Start.
            session_->AddLocalGamer(second);
            session_->Update();
            ctx.Check(session_->getLocalGamersProperty().getCountProperty() == 2,
                      "AddLocalGamer adds player two as a second local gamer");
            ctx.Check(!log_.empty() && log_.back() == "GamerJoined: Second Player", "GamerJoined announces player two");
            const LocalNetworkGamer* one = session_->getLocalGamersProperty()[0];
            const LocalNetworkGamer* two = session_->getLocalGamersProperty()[1];
            ctx.Check(one->getMachineProperty().getGamersProperty().getCountProperty() == 2 &&
                          two->getSignedInGamerProperty() == second,
                      "both play on this machine; LocalNetworkGamer::SignedInGamer is player two's profile");

            bool refused = false;
            try
            {
                session_->AddLocalGamer(second);
            }
            catch (const System::ArgumentException&)
            {
                refused = true;
            }
            ctx.Check(refused, "adding the same player twice is refused (ArgumentException)");

            // The game starts. With AllowJoinInProgress off (the default), nobody joins mid-game.
            session_->StartGame();
            session_->Update();
            refused = false;
            try
            {
                session_->AddLocalGamer(third);
            }
            catch (const System::InvalidOperationException&)
            {
                refused = true;
            }
            ctx.Check(!session_->getAllowJoinInProgressProperty() && refused,
                      "during play, player three is refused while AllowJoinInProgress is false");

            session_->setAllowJoinInProgressProperty(true);
            session_->AddLocalGamer(third);
            session_->Update();
            ctx.Check(session_->getLocalGamersProperty().getCountProperty() == 3,
                      "with AllowJoinInProgress, player three joins the game in progress");
            for (const std::string& line : log_)
            {
                ctx.Print("{}", line);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 40, 30));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Local session: press Start to join", Vector2(40.0f, 30.0f), Color::White);
            float y = 80.0f;
            for (const std::string& line : log_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::LightGreen);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GamerJoinedEventArgs>::Token joinedToken_{};
        std::vector<std::string> log_;
        bool done_ = false;
        int waited_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(AddLocalGamerDemo, {
    .id = "net.network_session.add_local_gamer",
    .title = "Join a session already running",
    .description = "Player one starts a Local session alone; player two joins in the lobby with "
                   "NetworkSession::AddLocalGamer (GamerJoined); during play, player three is refused until "
                   "AllowJoinInProgress is set.",
    .apis = {"NetworkSession::AddLocalGamer", "NetworkSession::AllowJoinInProgress", "NetworkSession::Create",
             "NetworkSession::GamerJoined", "LocalNetworkGamer::SignedInGamer", "NetworkGamer::Machine",
             "NetworkMachine::Gamers", "NetworkSession::StartGame", "NetworkSession::LocalGamers"},
    .tags = {"net", "multiplayer", "split-screen", "join"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
