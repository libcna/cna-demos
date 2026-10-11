// SPDX-License-Identifier: MS-PL
// NetworkMachine::RemoveFromSession: the host removes a player's machine from a System Link
// session -- a kick. This is a complete program rather than a demo inside cna-demos: a process
// holds one NetworkSession, so the player is a second process -- this program started again with
// --player.
//
// Arguments: [--automated] [--screenshot PATH] [--log-level LEVEL] [--player] (--frames is
// accepted and ignored: the run lasts as long as the exchange takes).
// Exit code 0 = every check passed, 1 = a check failed.
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/Process.hpp"
#include "CnaDemos/StandaloneProgram.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerServicesComponent.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Net/AvailableNetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/AvailableNetworkSessionCollection.hpp"
#include "Microsoft/Xna/Framework/Net/GamerJoinedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/GamerLeftEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkMachine.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionEndReason.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionEndedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    constexpr const char* HostGamertag = "Demo Player";
    constexpr const char* PlayerGamertag = "Second Player";
    using Clock = std::chrono::steady_clock;
    constexpr std::chrono::seconds Patience(20);

    struct Options : CnaDemos::Standalone::Arguments
    {
        bool player = false;
    };

    class RemoveFromSessionGame final : public Game
    {
    public:
        explicit RemoveFromSessionGame(const Options& options)
            : graphics_(this),
              options_(options),
              report_(options.player ? "player" : "host")
        {
            getContentProperty().setRootDirectoryProperty("assets");
            // The screenshot reads the back buffer, which XNA allows under HiDef only.
            graphics_.setGraphicsProfileProperty(GraphicsProfile::HiDef);
            // CNA's offline gamer services and the demos' own profiles; each process signs in its
            // own local profile (the child inherits the parent's environment: set, not defaulted).
            CnaDemos::UseOfflineDemoProfiles();
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_AUTO_SIGN_IN", options.player ? PlayerGamertag : HostGamertag);
            getComponentsProperty().Add(std::make_shared<GamerServicesComponent>(*this));
        }

        ~RemoveFromSessionGame() override
        {
            if (player_.joinable())
            {
                player_.join();
            }
        }

        [[nodiscard]] int Failures() const { return report_.Failures(); }

    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            started_ = Clock::now();
        }

        void UnloadContent() override
        {
            if (player_.joinable())
            {
                player_.join();
            }
            CloseSession();
        }

        void Update(GameTime& gameTime) override
        {
            Game::Update(gameTime);
            if (!options_.automated && !options_.player && Input::Keyboard::GetState().IsKeyDown(Input::Keys::Escape))
            {
                Exit();
            }
            if (finished_ || Gamer::getSignedInGamersProperty()->getCountProperty() == 0)
            {
                return; // done, or the profile signs in during the first frames
            }
            if (options_.player)
            {
                UpdatePlayer();
            }
            else
            {
                UpdateHost();
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(options_.player ? Color(40, 30, 52) : Color(24, 40, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, options_.player ? "Player (second process)" : "Host: removes the player",
                                     Vector2(30.0f, 20.0f), Color::White);
            float y = 64.0f;
            for (const std::string& line : report_.Lines())
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
            if (finished_ && !options_.screenshot.empty() && !options_.player && !captured_)
            {
                captured_ = true;
                CnaDemos::Standalone::SaveScreenshot(device, options_.screenshot);
            }
            if (finished_ && (options_.automated || options_.player))
            {
                Exit();
            }
        }

    private:
        // --- the host (first process) ----------------------------------------------------------
        void UpdateHost()
        {
            if (!session_)
            {
                session_.reset(NetworkSession::Create(NetworkSessionType::SystemLink, 1, 2));
                joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                    if (!args.getGamerProperty()->getIsLocalProperty())
                    {
                        report_.Log(args.getGamerProperty()->getGamertagProperty() + " joined");
                        joinedAt_ = Clock::now();
                    }
                });
                leftToken_ = session_->GamerLeft.Add([this](System::Object*, const GamerLeftEventArgs& args) {
                    left_ = args.getGamerProperty()->getGamertagProperty();
                    report_.Log("GamerLeft: " + left_);
                });
                report_.Log("hosting a System Link session");
                // The player: this program again (it binds its discovery socket after this host's,
                // the order that works on Linux -- CNA-DEMOS-ISSUE-026).
                std::vector<std::string> arguments = {"--player", "--log-level", options_.logLevel};
                if (options_.automated)
                {
                    arguments.emplace_back("--automated");
                }
                player_ = std::thread([this, arguments] {
                    const auto program =
                        CnaDemos::Standalone::ThisProgram("cna-demo-net-network_machine-remove_from_session");
                    playerExit_ = CnaDemos::RunProcess(program, arguments);
                    playerDone_ = true;
                });
                return;
            }
            session_->Update();
            if (!removed_ && joinedAt_ && Clock::now() - *joinedAt_ > std::chrono::milliseconds(500))
            {
                removed_ = true;
                NetworkGamer* player = session_->getRemoteGamersProperty()[0];
                // A machine may hold several gamers (split screen): removing it removes them all.
                report_.Log("removing " + player->getGamertagProperty() + "'s machine");
                player->getMachineProperty().RemoveFromSession();

                // Nobody but the host may remove a machine, and the host cannot remove itself.
                bool refused = false;
                try
                {
                    session_->getLocalGamersProperty()[0]->getMachineProperty().RemoveFromSession();
                }
                catch (const System::InvalidOperationException&)
                {
                    refused = true;
                }
                report_.Check(refused, "the host cannot remove its own machine");
            }
            if (playerDone_)
            {
                player_.join();
                session_->Update();
                report_.Check(playerExit_ == 0, "the player saw its session end with RemovedByHost");
                report_.Check(left_ == PlayerGamertag, "the host saw the player leave (GamerLeft)");
                report_.Check(session_->getRemoteGamersProperty().getCountProperty() == 0,
                              "the session has no remote gamer left");
                CloseSession();
                finished_ = true;
            }
            else if (!joinedAt_ && Clock::now() - started_ > Patience + Patience)
            {
                report_.Check(false, "the player joined");
                finished_ = true;
            }
        }

        // --- the player (second process) -----------------------------------------------------
        void UpdatePlayer()
        {
            if (!session_)
            {
                AvailableNetworkSessionCollection found =
                    NetworkSession::Find(NetworkSessionType::SystemLink, 1, NetworkSessionProperties());
                const auto& sessions = std::as_const(found);
                for (int i = 0; i < sessions.getCountProperty() && !session_; ++i)
                {
                    if (sessions[i].getHostGamertagProperty() == HostGamertag)
                    {
                        session_.reset(NetworkSession::Join(&sessions[i]));
                        report_.Log("joined " + std::string(HostGamertag) + "'s session");
                        endedToken_ = session_->SessionEnded.Add([this](System::Object*,
                                                                        const NetworkSessionEndedEventArgs& args) {
                            ended_ = args.getEndReasonProperty();
                        });
                        joinedAt_ = Clock::now();
                    }
                }
                found.Dispose();
                if (!session_ && Clock::now() - started_ > Patience)
                {
                    report_.Check(false, "the host's session was found");
                    finished_ = true;
                }
                return;
            }
            if (!ended_)
            {
                session_->Update(); // the removal arrives here
            }
            if (ended_ || Clock::now() - *joinedAt_ > Patience)
            {
                report_.Check(ended_ == NetworkSessionEndReason::RemovedByHost,
                              "SessionEnded reports RemovedByHost");
                CloseSession();
                finished_ = true;
            }
        }

        // --- helpers -----------------------------------------------------------------------
        void CloseSession()
        {
            if (session_)
            {
                session_->GamerJoined.Remove(joinedToken_);
                session_->GamerLeft.Remove(leftToken_);
                session_->SessionEnded.Remove(endedToken_);
                session_->Dispose(); // the caller owns the session: dispose, then delete
                session_.reset();
            }
        }

        GraphicsDeviceManager graphics_;
        Options options_;
        CnaDemos::Standalone::Report report_;
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GamerJoinedEventArgs>::Token joinedToken_{};
        System::EventHandler<GamerLeftEventArgs>::Token leftToken_{};
        System::EventHandler<NetworkSessionEndedEventArgs>::Token endedToken_{};
        Clock::time_point started_{};
        std::optional<Clock::time_point> joinedAt_;
        // The host's side.
        std::thread player_;
        std::atomic<bool> playerDone_{false};
        std::atomic<int> playerExit_{-1};
        bool removed_ = false;
        std::string left_;
        // The player's side.
        std::optional<NetworkSessionEndReason> ended_;
        bool finished_ = false;
        bool captured_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

int main(int argc, char** argv)
{
    Options options;
    static_cast<CnaDemos::Standalone::Arguments&>(options) = CnaDemos::Standalone::ParseArguments(argc, argv);
    for (const std::string& argument : options.other)
    {
        options.player = options.player || argument == "--player";
    }

    try
    {
        RemoveFromSessionGame game(options);
        game.Run();
        if (!options.player)
        {
            std::cout << (game.Failures() == 0 ? "== PASSED\n" : "== FAILED\n");
        }
        return game.Failures() == 0 ? 0 : 1;
    }
    catch (const std::exception& exception)
    {
        std::cout << (options.player ? "player" : "host") << ": exception: " << exception.what() << '\n';
        return 1;
    }
}
