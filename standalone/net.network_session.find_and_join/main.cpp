// SPDX-License-Identifier: MS-PL
// NetworkSession::Find and Join: a System Link game found on the network by a second machine.
// This is a complete program rather than a demo inside cna-demos: a process holds one
// NetworkSession, so finding someone else's session needs a second process. The program plays
// both parts: started normally it hosts and launches a copy of itself with --joiner, which finds
// the session, joins it, and leaves again.
//
// Arguments: [--automated] [--screenshot PATH] [--log-level LEVEL] [--joiner] (--frames is accepted
// and ignored: the run lasts as long as the exchange takes).
// Exit code 0 = every check passed, 1 = a check failed.
#include <any>
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
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Net/QualityOfService.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IAsyncResult.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    constexpr const char* HostGamertag = "Demo Player";
    constexpr const char* JoinerGamertag = "Second Player";
    using Clock = std::chrono::steady_clock;
    // How long each side waits for the other before giving up.
    constexpr std::chrono::seconds Patience(20);

    struct Options : CnaDemos::Standalone::Arguments
    {
        bool joiner = false;
    };

    std::string MilliSeconds(const System::TimeSpan& time)
    {
        return std::to_string(static_cast<int>(time.getTotalMillisecondsProperty())) + " ms";
    }

    class FindAndJoinGame final : public Game
    {
    public:
        explicit FindAndJoinGame(const Options& options)
            : graphics_(this),
              options_(options),
              report_(options.joiner ? "joiner" : "host")
        {
            getContentProperty().setRootDirectoryProperty("assets");
            // The screenshot reads the back buffer, which XNA allows under HiDef only.
            graphics_.setGraphicsProfileProperty(GraphicsProfile::HiDef);
            // CNA's offline gamer services and the demos' own profiles, as for every gamer services
            // demo; each process signs in its own local profile (the joiner inherits the host's
            // environment, so it is set, not defaulted).
            CnaDemos::UseOfflineDemoProfiles();
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_AUTO_SIGN_IN", options.joiner ? JoinerGamertag : HostGamertag);
            getComponentsProperty().Add(std::make_shared<GamerServicesComponent>(*this));
        }

        ~FindAndJoinGame() override
        {
            if (joiner_.joinable())
            {
                joiner_.join();
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
            if (joiner_.joinable())
            {
                joiner_.join();
            }
            CloseSession();
        }

        void Update(GameTime& gameTime) override
        {
            Game::Update(gameTime);
            if (!options_.automated && !options_.joiner &&
                Input::Keyboard::GetState().IsKeyDown(Input::Keys::Escape))
            {
                Exit();
            }
            if (finished_)
            {
                return;
            }
            if (Clock::now() - started_ > Patience + Patience)
            {
                report_.Check(false, "the run finished in time");
                Finish();
                return;
            }
            if (Gamer::getSignedInGamersProperty()->getCountProperty() == 0)
            {
                return; // the profile signs in during the first frames
            }
            if (options_.joiner)
            {
                UpdateJoiner();
            }
            else
            {
                UpdateHost();
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(options_.joiner ? Color(40, 30, 52) : Color(24, 40, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, options_.joiner ? "Joiner (second process)" : "Host",
                                     Vector2(30.0f, 20.0f), Color::White);
            float y = 64.0f;
            for (const std::string& line : report_.Lines())
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
            if (finished_ && !options_.screenshot.empty() && !options_.joiner && !captured_)
            {
                captured_ = true;
                CnaDemos::Standalone::SaveScreenshot(device, options_.screenshot);
            }
            // Unattended, and always for the joiner, the run ends with the exchange; the host
            // window otherwise stays until Escape. (The run lasts as long as the exchange takes,
            // so --frames does not apply.)
            if (finished_ && (options_.automated || options_.joiner))
            {
                Exit();
            }
        }

    private:
        // --- host --------------------------------------------------------------------------
        void UpdateHost()
        {
            if (!session_)
            {
                // A System Link session is visible to every machine on the local network: up to
                // one local gamer here, three gamers in all. Reserve one private slot so the
                // public listing has one joinable slot, and advertise two sparse lobby values.
                NetworkSessionProperties advertised;
                advertised[0] = 2026;
                advertised[5] = -41;
                session_.reset(NetworkSession::Create(
                    NetworkSessionType::SystemLink, 1, 3, 1, std::move(advertised)));
                joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                    if (!args.getGamerProperty()->getIsLocalProperty())
                    {
                        NetworkGamer* gamer = args.getGamerProperty();
                        report_.Log("GamerJoined: " + gamer->getGamertagProperty() + " (remote)");
                        remoteJoined_ = gamer->getGamertagProperty();
                        remoteGamer_ = gamer;
                        report_.Check(!gamer->getHasLeftSessionProperty() && gamer->getSessionProperty() == session_.get(),
                                      "the joined remote gamer is live and belongs to the host session");
                    }
                });
                leftToken_ = session_->GamerLeft.Add([this](System::Object*, const GamerLeftEventArgs& args) {
                    NetworkGamer* gamer = args.getGamerProperty();
                    report_.Log("GamerLeft: " + gamer->getGamertagProperty());
                    remoteLeft_ = gamer->getGamertagProperty();
                    report_.Check(gamer == remoteGamer_ && gamer->getHasLeftSessionProperty(),
                                  "GamerLeft identifies the joined gamer and marks it as departed");
                    const auto& previous = session_->getPreviousGamersProperty();
                    report_.Check(previous.getCountProperty() == 1 && previous[0] == gamer &&
                                      gamer->getSessionProperty() == session_.get(),
                                  "PreviousGamers retains the departed gamer in its original session");
                    report_.Check(gamer->getRoundtripTimeProperty().getTotalMillisecondsProperty() >= 0.0,
                                  "the departed gamer retains a non-negative measured round trip");
                });
                report_.Log("hosting a System Link session as " + std::string(HostGamertag));

                // The second machine: this program again, as the joiner. It runs until it has
                // joined and left; the host keeps updating its session meanwhile.
                std::vector<std::string> arguments = {"--joiner", "--log-level", options_.logLevel};
                if (options_.automated)
                {
                    arguments.insert(arguments.begin(), "--automated");
                }
                joiner_ = std::thread([this, arguments] {
                    joinerExit_ = CnaDemos::RunProcess(
                        CnaDemos::Standalone::ThisProgram("cna-demo-net-network_session-find_and_join"), arguments);
                    joinerDone_ = true;
                });
                return;
            }
            // Joins and departures are raised here, in the session's Update.
            session_->Update();
            if (joinerDone_)
            {
                joiner_.join();
                report_.Log("the joiner exited with code " + std::to_string(joinerExit_.load()));
                report_.Check(joinerExit_ == 0, "the joiner found the session, joined it and left");
                report_.Check(remoteJoined_ == JoinerGamertag, "the host saw Second Player join (GamerJoined)");
                report_.Check(remoteLeft_ == JoinerGamertag, "the host saw Second Player leave (GamerLeft)");
                report_.Check(session_->getRemoteGamersProperty().getCountProperty() == 0,
                      "the session has no remote gamer left");
                Finish();
            }
        }

        // --- joiner ------------------------------------------------------------------------
        void UpdateJoiner()
        {
            if (!session_)
            {
                // Find lists the System Link sessions on the network with room for one more
                // local gamer. Any other System Link game on this network shows up too: pick the
                // one Demo Player hosts.
                std::string findCallbackState;
                std::unique_ptr<System::IAsyncResult> finding(NetworkSession::BeginFind(
                    NetworkSessionType::SystemLink, 1, NetworkSessionProperties(),
                    [&findCallbackState](System::IAsyncResult& result) {
                        findCallbackState = std::any_cast<std::string>(result.getAsyncStateProperty());
                    },
                    std::string("find-lobby")));
                if (!findAsyncChecked_)
                {
                    findAsyncChecked_ = true;
                    report_.Check(finding->getIsCompletedProperty() && finding->getCompletedSynchronouslyProperty() &&
                                      findCallbackState == "find-lobby",
                                  "BeginFind returns a completed System Link search token with its callback state");
                }
                AvailableNetworkSessionCollection found = NetworkSession::EndFind(finding.get());
                const auto& sessions = std::as_const(found);
                for (int i = 0; i < sessions.getCountProperty(); ++i)
                {
                    const AvailableNetworkSession& available = sessions[i];
                    const QualityOfService& quality = available.getQualityOfServiceProperty();
                    report_.Log("found " + available.getHostGamertagProperty() + "'s session: " +
                        std::to_string(available.getCurrentGamerCountProperty()) + " gamer(s), " +
                        std::to_string(available.getOpenPublicGamerSlotsProperty()) + " open slot(s)");
                    if (available.getHostGamertagProperty() != HostGamertag)
                    {
                        continue;
                    }
                    // QualityOfService: measured while searching -- a lobby shows it per session.
                    report_.Log("  round trip " + MilliSeconds(quality.getAverageRoundtripTimeProperty()) + " (min " +
                        MilliSeconds(quality.getMinimumRoundtripTimeProperty()) + "), measured: " +
                        (quality.getIsAvailableProperty() ? "yes" : "no"));
                    report_.Check(quality.getBytesPerSecondDownstreamProperty() > 0 &&
                                      quality.getBytesPerSecondUpstreamProperty() > 0,
                                  "discovery measured positive downstream and upstream bandwidth");
                    report_.Check(available.getCurrentGamerCountProperty() == 1 &&
                                      available.getOpenPublicGamerSlotsProperty() == 1,
                          "the session holds its host and has one open slot");
                    const NetworkSessionProperties& advertised = available.getSessionPropertiesProperty();
                    report_.Check(available.getOpenPrivateGamerSlotsProperty() == 1 &&
                                      available.getOpenPublicGamerSlotsProperty() == 1,
                          "the discovered session advertises one private and one public slot");
                    report_.Check(advertised.getCountProperty() == 8 && advertised.getItem(0) == 2026 &&
                                      !advertised.getItem(1).has_value() && advertised.getItem(5) == -41,
                          "the discovered session preserves both sparse advertised property values");
                    report_.Check(!found.getIsDisposedProperty(),
                                  "the search-result collection is live while its listing is joined");
                    const AvailableNetworkSession stale = available;
                    std::string joinCallbackState;
                    std::unique_ptr<System::IAsyncResult> joining(NetworkSession::BeginJoin(
                        &available,
                        [&joinCallbackState](System::IAsyncResult& result) {
                            joinCallbackState = std::any_cast<std::string>(result.getAsyncStateProperty());
                        },
                        std::string("join-lobby")));
                    report_.Check(joining->getIsCompletedProperty() && joining->getCompletedSynchronouslyProperty() &&
                                      joinCallbackState == "join-lobby",
                                  "BeginJoin returns its completed token and callback state before EndJoin connects");
                    session_.reset(NetworkSession::EndJoin(joining.get()));
                    found.Dispose();
                    found.Dispose();
                    bool staleRefused = false;
                    try
                    {
                        std::unique_ptr<NetworkSession> unused(NetworkSession::Join(&stale));
                    }
                    catch (const System::ObjectDisposedException&)
                    {
                        staleRefused = true;
                    }
                    report_.Check(found.getIsDisposedProperty() && found.getCountProperty() > 0 && staleRefused,
                                  "collection disposal is idempotent, retains listings and makes their copies unjoinable");
                    report_.Log("joined");
                    report_.Check(!session_->getIsHostProperty(), "the joiner is not the host");
                    report_.Check(session_->getHostProperty() != nullptr &&
                              session_->getHostProperty()->getGamertagProperty() == HostGamertag,
                          "Demo Player hosts the joined session");
                    joinedAt_ = Clock::now();
                    break;
                }
                found.Dispose(); // the search's results are released; a joined session lives on
                if (!session_ && Clock::now() - started_ > Patience)
                {
                    report_.Check(false, "Demo Player's session was found");
                    Finish();
                }
                return;
            }
            session_->Update();
            // Stay a moment, so the host sees this gamer in its session, then leave.
            if (Clock::now() - joinedAt_ > std::chrono::milliseconds(500))
            {
                report_.Check(session_->getAllGamersProperty().getCountProperty() == 2,
                              "both gamers are in the session");
                CloseSession();
                report_.Log("left the session");
                Finish();
            }
        }

        // --- helpers -----------------------------------------------------------------------
        void CloseSession()
        {
            if (session_)
            {
                session_->GamerJoined.Remove(joinedToken_);
                session_->GamerLeft.Remove(leftToken_);
                session_->Dispose(); // the caller owns the session: dispose, then delete
                session_.reset();
            }
        }

        void Finish()
        {
            finished_ = true;
            if (!options_.automated)
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
        std::thread joiner_;
        std::atomic<bool> joinerDone_{false};
        std::atomic<int> joinerExit_{-1};
        std::string remoteJoined_;
        std::string remoteLeft_;
        NetworkGamer* remoteGamer_ = nullptr;
        Clock::time_point started_{};
        Clock::time_point joinedAt_{};
        bool finished_ = false;
        bool captured_ = false;
        bool findAsyncChecked_ = false;
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
        options.joiner = options.joiner || argument == "--joiner";
    }

    try
    {
        FindAndJoinGame game(options);
        game.Run();
        if (!options.joiner)
        {
            std::cout << (game.Failures() == 0 ? "== PASSED\n" : "== FAILED\n");
        }
        return game.Failures() == 0 ? 0 : 1;
    }
    catch (const std::exception& exception)
    {
        std::cout << (options.joiner ? "joiner" : "host") << ": exception: " << exception.what() << '\n';
        return 1;
    }
}
