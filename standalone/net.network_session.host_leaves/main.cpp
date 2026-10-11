// SPDX-License-Identifier: MS-PL
// When the host of a System Link session leaves: the session ends for everyone (SessionEnded), or,
// with AllowHostMigration, a remaining gamer becomes the host (HostChanged) and the game goes on.
// This is a complete program rather than a demo inside cna-demos: a process holds one
// NetworkSession, so the other player is a second process -- this program started again with
// --player. The host plays two rounds, leaving once a player has joined: first without host
// migration, then with it; a new player process joins each round.
//
// Arguments: [--automated] [--screenshot PATH] [--log-level LEVEL] [--player ROUND]
// (--frames is accepted and ignored: the run lasts as long as the two rounds take).
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
#include "Microsoft/Xna/Framework/Net/HostChangedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionEndReason.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionEndedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    constexpr const char* HostGamertag = "Demo Player";
    constexpr const char* PlayerGamertag = "Second Player";
    // Session property 0 carries the round, so each Find matches only that round's session.
    constexpr int RoundProperty = 0;
    using Clock = std::chrono::steady_clock;
    constexpr std::chrono::seconds Patience(20);

    struct Options : CnaDemos::Standalone::Arguments
    {
        int playerRound = 0; // > 0: this process is the player of that round
    };

    const char* EndReasonName(NetworkSessionEndReason reason)
    {
        switch (reason)
        {
            case NetworkSessionEndReason::ClientSignedOut: return "ClientSignedOut";
            case NetworkSessionEndReason::HostEndedSession: return "HostEndedSession";
            case NetworkSessionEndReason::RemovedByHost: return "RemovedByHost";
            case NetworkSessionEndReason::Disconnected: return "Disconnected";
        }
        return "?";
    }

    class HostLeavesGame final : public Game
    {
    public:
        explicit HostLeavesGame(const Options& options)
            : graphics_(this),
              options_(options),
              report_(options.playerRound > 0 ? "player" : "host")
        {
            getContentProperty().setRootDirectoryProperty("assets");
            // The screenshot reads the back buffer, which XNA allows under HiDef only.
            graphics_.setGraphicsProfileProperty(GraphicsProfile::HiDef);
            // CNA's offline gamer services and the demos' own profiles, as for every gamer services
            // demo; each process signs in its own local profile (a child inherits the parent's
            // environment, so it is set, not defaulted).
            CnaDemos::UseOfflineDemoProfiles();
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_AUTO_SIGN_IN",
                                     options.playerRound > 0 ? PlayerGamertag : HostGamertag);
            getComponentsProperty().Add(std::make_shared<GamerServicesComponent>(*this));
        }

        ~HostLeavesGame() override
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
            if (!options_.automated && options_.playerRound == 0 &&
                Input::Keyboard::GetState().IsKeyDown(Input::Keys::Escape))
            {
                Exit();
            }
            if (finished_)
            {
                return;
            }
            if (Gamer::getSignedInGamersProperty()->getCountProperty() == 0)
            {
                return; // the profile signs in during the first frames
            }
            if (options_.playerRound > 0)
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
            device.Clear(options_.playerRound > 0 ? Color(40, 30, 52) : Color(24, 40, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_,
                                     options_.playerRound > 0 ? "Player (second process)" : "Host: leaves each round",
                                     Vector2(30.0f, 20.0f), Color::White);
            float y = 64.0f;
            for (const std::string& line : report_.Lines())
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
            if (finished_ && !options_.screenshot.empty() && options_.playerRound == 0 && !captured_)
            {
                captured_ = true;
                CnaDemos::Standalone::SaveScreenshot(device, options_.screenshot);
            }
            if (finished_ && (options_.automated || options_.playerRound > 0))
            {
                Exit();
            }
        }

    private:
        // --- the host, which leaves (first process) ------------------------------------------
        void UpdateHost()
        {
            if (!playerRunning_)
            {
                if (round_ == 2)
                {
                    Finish();
                    return;
                }
                StartRound(++round_);
                return;
            }
            if (session_)
            {
                session_->Update();
                if (joinedAt_ && Clock::now() - *joinedAt_ > std::chrono::milliseconds(500))
                {
                    // The host quits the game: disposing its session is leaving it.
                    CloseSession();
                    report_.Log("round " + std::to_string(round_) + ": left the session");
                }
                else if (!joinedAt_ && Clock::now() - roundStarted_ > Patience)
                {
                    report_.Check(false, "round " + std::to_string(round_) + ": a player joined");
                    CloseSession();
                }
            }
            if (playerDone_)
            {
                player_.join();
                playerRunning_ = false;
                CloseSession();
                report_.Check(playerExit_ == 0, round_ == 1
                                                    ? "round 1: the player saw the session end (HostEndedSession)"
                                                    : "round 2: the player became the host (HostChanged)");
            }
        }

        void StartRound(int round)
        {
            NetworkSessionProperties properties;
            properties.setItem(RoundProperty, round);
            session_.reset(NetworkSession::Create(NetworkSessionType::SystemLink, 1, 2, 0, properties));
            // Host migration is the host's choice; joiners find it in the session.
            session_->setAllowHostMigrationProperty(round == 2);
            joinedAt_.reset();
            joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                if (!args.getGamerProperty()->getIsLocalProperty())
                {
                    report_.Log("round " + std::to_string(round_) + ": " +
                                args.getGamerProperty()->getGamertagProperty() + " joined");
                    joinedAt_ = Clock::now();
                }
            });
            report_.Log("round " + std::to_string(round) + ": hosting, AllowHostMigration " +
                        (round == 2 ? "true" : "false"));

            // The other machine: this program again, as a player. A new process each round:
            // CNA-DEMOS-ISSUE-026 -- on Linux a process that searched before this host's
            // discovery socket existed would not receive its replies.
            playerDone_ = false;
            playerRunning_ = true;
            roundStarted_ = Clock::now();
            std::vector<std::string> arguments = {"--player", std::to_string(round), "--log-level", options_.logLevel};
            if (options_.automated)
            {
                arguments.emplace_back("--automated");
            }
            player_ = std::thread([this, arguments] {
                playerExit_ = CnaDemos::RunProcess(
                    CnaDemos::Standalone::ThisProgram("cna-demo-net-network_session-host_leaves"), arguments);
                playerDone_ = true;
            });
        }

        // --- a player, which stays (second process) ------------------------------------------
        void UpdatePlayer()
        {
            if (!session_)
            {
                if (!joined_)
                {
                    JoinRound();
                }
                return;
            }
            // An ended session raises nothing more: only a live one is updated.
            if (!ended_)
            {
                session_->Update();
            }
            const bool settled = options_.playerRound == 1 ? ended_.has_value() : hostChanged_ && !left_.empty();
            if (settled || Clock::now() - playerJoinedAt_ > Patience)
            {
                CheckRound();
                CloseSession();
                Finish();
            }
        }

        void JoinRound()
        {
            NetworkSessionProperties search;
            search.setItem(RoundProperty, options_.playerRound);
            AvailableNetworkSessionCollection found = NetworkSession::Find(NetworkSessionType::SystemLink, 1, search);
            const auto& sessions = std::as_const(found);
            for (int i = 0; i < sessions.getCountProperty(); ++i)
            {
                if (sessions[i].getHostGamertagProperty() != HostGamertag)
                {
                    continue;
                }
                session_.reset(NetworkSession::Join(&sessions[i]));
                joined_ = true;
                playerJoinedAt_ = Clock::now();
                report_.Log("joined " + std::string(HostGamertag) + "'s session; AllowHostMigration " +
                    (session_->getAllowHostMigrationProperty() ? "true" : "false"));
                report_.Check(session_->getAllowHostMigrationProperty() == (options_.playerRound == 2),
                      "the joined session carries the host's AllowHostMigration");
                leftToken_ = session_->GamerLeft.Add([this](System::Object*, const GamerLeftEventArgs& args) {
                    left_ = args.getGamerProperty()->getGamertagProperty();
                    report_.Log("GamerLeft: " + left_);
                });
                hostChangedToken_ = session_->HostChanged.Add([this](System::Object*,
                                                                     const HostChangedEventArgs& args) {
                    hostChanged_ = true;
                    oldHost_ = args.getOldHostProperty()->getGamertagProperty();
                    newHostIsLocal_ = args.getNewHostProperty()->getIsLocalProperty();
                    report_.Log("HostChanged: " + oldHost_ + " -> " + args.getNewHostProperty()->getGamertagProperty());
                });
                endedToken_ = session_->SessionEnded.Add([this](System::Object*,
                                                                const NetworkSessionEndedEventArgs& args) {
                    ended_ = args.getEndReasonProperty();
                    report_.Log(std::string("SessionEnded: ") + EndReasonName(*ended_));
                });
                break;
            }
            found.Dispose();
            if (!session_ && Clock::now() - started_ > Patience)
            {
                report_.Check(false, "the host's session was found");
                Finish();
            }
        }

        void CheckRound()
        {
            if (options_.playerRound == 1)
            {
                // Without migration the session cannot outlive its host.
                report_.Check(ended_ == NetworkSessionEndReason::HostEndedSession,
                      "without migration, SessionEnded reports HostEndedSession");
                report_.Check(!hostChanged_, "without migration, no HostChanged");
                return;
            }
            // With migration the session goes on: a remaining gamer -- here this machine's --
            // becomes the host.
            report_.Check(!ended_.has_value(), "with migration, the session does not end");
            report_.Check(hostChanged_ && oldHost_ == HostGamertag && newHostIsLocal_,
                  "HostChanged names Demo Player as the old host and this machine's gamer as the new one");
            report_.Check(left_ == HostGamertag, "GamerLeft reports Demo Player");
            report_.Check(session_->getIsHostProperty() && session_->getAllGamersProperty().getCountProperty() == 1,
                  "Second Player now hosts the session, alone");
        }

        // --- helpers -----------------------------------------------------------------------
        void CloseSession()
        {
            if (session_)
            {
                session_->GamerJoined.Remove(joinedToken_);
                session_->GamerLeft.Remove(leftToken_);
                session_->HostChanged.Remove(hostChangedToken_);
                session_->SessionEnded.Remove(endedToken_);
                session_->Dispose(); // the caller owns the session: dispose, then delete
                session_.reset();
            }
        }

        void Finish()
        {
            finished_ = true;
            if (!options_.automated && options_.playerRound == 0)
            {
                report_.Log("press Escape or close the window");
            }
        }





        GraphicsDeviceManager graphics_;
        Options options_;
        CnaDemos::Standalone::Report report_;
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GamerJoinedEventArgs>::Token joinedToken_{};
        System::EventHandler<GamerLeftEventArgs>::Token leftToken_{};
        System::EventHandler<HostChangedEventArgs>::Token hostChangedToken_{};
        System::EventHandler<NetworkSessionEndedEventArgs>::Token endedToken_{};
        Clock::time_point started_{};
        // The host's rounds, each with its own player process.
        int round_ = 0;
        bool playerRunning_ = false;
        std::thread player_;
        std::atomic<bool> playerDone_{false};
        std::atomic<int> playerExit_{-1};
        Clock::time_point roundStarted_{};
        std::optional<Clock::time_point> joinedAt_;
        // The player's side.
        bool joined_ = false;
        Clock::time_point playerJoinedAt_{};
        std::optional<NetworkSessionEndReason> ended_;
        bool hostChanged_ = false;
        bool newHostIsLocal_ = false;
        std::string oldHost_;
        std::string left_;
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
    for (std::size_t i = 0; i + 1 < options.other.size(); ++i)
    {
        if (options.other[i] == "--player")
        {
            options.playerRound = std::stoi(options.other[i + 1]);
        }
    }

    try
    {
        HostLeavesGame game(options);
        game.Run();
        if (options.playerRound == 0)
        {
            std::cout << (game.Failures() == 0 ? "== PASSED\n" : "== FAILED\n");
        }
        return game.Failures() == 0 ? 0 : 1;
    }
    catch (const std::exception& exception)
    {
        std::cout << (options.playerRound > 0 ? "player" : "host") << ": exception: " << exception.what() << '\n';
        return 1;
    }
}
