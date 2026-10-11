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
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "CNA/Logger.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/Process.hpp"
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
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
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

    struct Options
    {
        bool automated = false;
        bool joiner = false;
        std::string screenshot;
        std::string logLevel = "warn";
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
              name_(options.joiner ? "joiner" : "host")
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

        [[nodiscard]] int Failures() const { return failures_; }

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
                Check(false, "the run finished in time");
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
            spriteBatch_->DrawString(*font_, options_.joiner ? "Joiner (second process)" : "Host", Vector2(30.0f, 20.0f),
                                     Color::White);
            float y = 64.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
            if (finished_ && !options_.screenshot.empty() && !options_.joiner && !captured_)
            {
                captured_ = true;
                SaveScreenshot(device);
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
                // one local gamer here, two gamers in all.
                session_.reset(NetworkSession::Create(NetworkSessionType::SystemLink, 1, 2));
                joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                    if (!args.getGamerProperty()->getIsLocalProperty())
                    {
                        Log("GamerJoined: " + args.getGamerProperty()->getGamertagProperty() + " (remote)");
                        remoteJoined_ = args.getGamerProperty()->getGamertagProperty();
                    }
                });
                leftToken_ = session_->GamerLeft.Add([this](System::Object*, const GamerLeftEventArgs& args) {
                    Log("GamerLeft: " + args.getGamerProperty()->getGamertagProperty());
                    remoteLeft_ = args.getGamerProperty()->getGamertagProperty();
                });
                Log("hosting a System Link session as " + std::string(HostGamertag));

                // The second machine: this program again, as the joiner. It runs until it has
                // joined and left; the host keeps updating its session meanwhile.
                std::vector<std::string> arguments = {"--joiner", "--log-level", options_.logLevel};
                if (options_.automated)
                {
                    arguments.insert(arguments.begin(), "--automated");
                }
                joiner_ = std::thread([this, arguments] {
                    joinerExit_ = CnaDemos::RunProcess(CnaDemos::ExecutableDirectory() / ProgramName(), arguments);
                    joinerDone_ = true;
                });
                return;
            }
            // Joins and departures are raised here, in the session's Update.
            session_->Update();
            if (joinerDone_)
            {
                joiner_.join();
                Log("the joiner exited with code " + std::to_string(joinerExit_.load()));
                Check(joinerExit_ == 0, "the joiner found the session, joined it and left");
                Check(remoteJoined_ == JoinerGamertag, "the host saw Second Player join (GamerJoined)");
                Check(remoteLeft_ == JoinerGamertag, "the host saw Second Player leave (GamerLeft)");
                Check(session_->getRemoteGamersProperty().getCountProperty() == 0,
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
                AvailableNetworkSessionCollection found =
                    NetworkSession::Find(NetworkSessionType::SystemLink, 1, NetworkSessionProperties());
                const auto& sessions = std::as_const(found);
                for (int i = 0; i < sessions.getCountProperty(); ++i)
                {
                    const AvailableNetworkSession& available = sessions[i];
                    const QualityOfService& quality = available.getQualityOfServiceProperty();
                    Log("found " + available.getHostGamertagProperty() + "'s session: " +
                        std::to_string(available.getCurrentGamerCountProperty()) + " gamer(s), " +
                        std::to_string(available.getOpenPublicGamerSlotsProperty()) + " open slot(s)");
                    if (available.getHostGamertagProperty() != HostGamertag)
                    {
                        continue;
                    }
                    // QualityOfService: measured while searching -- a lobby shows it per session.
                    Log("  round trip " + MilliSeconds(quality.getAverageRoundtripTimeProperty()) + " (min " +
                        MilliSeconds(quality.getMinimumRoundtripTimeProperty()) + "), measured: " +
                        (quality.getIsAvailableProperty() ? "yes" : "no"));
                    Check(available.getCurrentGamerCountProperty() == 1 && available.getOpenPublicGamerSlotsProperty() == 1,
                          "the session holds its host and has one open slot");
                    session_.reset(NetworkSession::Join(&available));
                    Log("joined");
                    Check(!session_->getIsHostProperty(), "the joiner is not the host");
                    Check(session_->getHostProperty() != nullptr &&
                              session_->getHostProperty()->getGamertagProperty() == HostGamertag,
                          "Demo Player hosts the joined session");
                    joinedAt_ = Clock::now();
                    break;
                }
                found.Dispose(); // the search's results are released; a joined session lives on
                if (!session_ && Clock::now() - started_ > Patience)
                {
                    Check(false, "Demo Player's session was found");
                    Finish();
                }
                return;
            }
            session_->Update();
            // Stay a moment, so the host sees this gamer in its session, then leave.
            if (Clock::now() - joinedAt_ > std::chrono::milliseconds(500))
            {
                Check(session_->getAllGamersProperty().getCountProperty() == 2, "both gamers are in the session");
                CloseSession();
                Log("left the session");
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
                Log("press Escape or close the window");
            }
        }

        void Log(const std::string& line)
        {
            std::cout << name_ << ": " << line << '\n' << std::flush;
            lines_.push_back(line);
        }

        void Check(bool passed, const std::string& what)
        {
            std::cout << name_ << (passed ? ":   check: " : ":   CHECK FAILED: ") << what << '\n' << std::flush;
            failures_ += passed ? 0 : 1;
        }

        static std::string ProgramName()
        {
#ifdef _WIN32
            return "cna-demo-net-network_session-find_and_join.exe";
#else
            return "cna-demo-net-network_session-find_and_join";
#endif
        }

        void SaveScreenshot(GraphicsDevice& device)
        {
            const auto& parameters = device.getPresentationParametersProperty();
            const int width = parameters.getBackBufferWidthProperty();
            const int height = parameters.getBackBufferHeightProperty();
            std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
            device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            for (Color& pixel : pixels)
            {
                pixel.setAProperty(255);
            }
            Texture2D image(device, width, height);
            image.SetData(pixels.data(), static_cast<int>(pixels.size()));
            image.SaveAsPng(options_.screenshot);
            std::cout << "  screenshot: " << options_.screenshot << '\n';
        }

        GraphicsDeviceManager graphics_;
        Options options_;
        std::string name_;
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GamerJoinedEventArgs>::Token joinedToken_{};
        System::EventHandler<GamerLeftEventArgs>::Token leftToken_{};
        std::thread joiner_;
        std::atomic<bool> joinerDone_{false};
        std::atomic<int> joinerExit_{-1};
        std::string remoteJoined_;
        std::string remoteLeft_;
        Clock::time_point started_{};
        Clock::time_point joinedAt_{};
        bool finished_ = false;
        bool captured_ = false;
        int failures_ = 0;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

int main(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument(argv[i]);
        if (argument == "--automated")
            options.automated = true;
        else if (argument == "--joiner")
            options.joiner = true;
        else if (argument == "--frames" && i + 1 < argc)
            ++i; // see the arguments above
        else if (argument == "--screenshot" && i + 1 < argc)
            options.screenshot = argv[++i];
        else if (argument == "--log-level" && i + 1 < argc)
            options.logLevel = argv[++i];
    }
    if (options.logLevel == "fatal")
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::FATAL);
    else if (options.logLevel == "error")
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::ERROR);
    else if (options.logLevel == "info")
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::INFO);
    else if (options.logLevel == "debug" || options.logLevel == "trace")
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::DEBUG);
    else
        CNA::Logger::SetMinimumLevel(CNA::LogLevel::WARN);

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
