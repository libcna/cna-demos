// SPDX-License-Identifier: MS-PL
// NetworkSessionJoinException: a System Link game that is already being played refuses a new
// player until the host sets AllowJoinInProgress. Two processes, as every System Link demo needs:
// started normally, the program hosts and launches a copy of itself with --joiner. The joiner
// finds the session in its lobby; the host then starts the game; the joiner's Join fails with
// NetworkSessionJoinError::SessionNotJoinable. The host sets AllowJoinInProgress, and a second
// Join succeeds into the running game.
//
// The two processes tell each other when to take the next step through empty files in a
// temporary directory (--signals DIR), so each step waits for the other side's previous one.
//
// Arguments: [--automated] [--screenshot PATH] [--log-level LEVEL] [--joiner --signals DIR]
// (--frames is accepted and ignored: the run lasts as long as the exchange takes).
// Exit code 0 = every check passed, 1 = a check failed.
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
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
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionJoinError.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionJoinException.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionState.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Runtime/Serialization/SerializationInfo.hpp"
#include "System/Runtime/Serialization/StreamingContext.hpp"

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
        std::filesystem::path signals;
    };

    std::string JoinErrorName(NetworkSessionJoinError error)
    {
        switch (error)
        {
        case NetworkSessionJoinError::SessionNotFound: return "SessionNotFound";
        case NetworkSessionJoinError::SessionNotJoinable: return "SessionNotJoinable";
        case NetworkSessionJoinError::SessionFull: return "SessionFull";
        }
        return "?";
    }

    class JoinRefusedGame final : public Game
    {
    public:
        explicit JoinRefusedGame(const Options& options)
            : graphics_(this),
              options_(options),
              report_(options.joiner ? "joiner" : "host")
        {
            getContentProperty().setRootDirectoryProperty("assets");
            // The screenshot reads the back buffer, which XNA allows under HiDef only.
            graphics_.setGraphicsProfileProperty(GraphicsProfile::HiDef);
            CnaDemos::UseOfflineDemoProfiles();
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_AUTO_SIGN_IN", options.joiner ? JoinerGamertag : HostGamertag);
            getComponentsProperty().Add(std::make_shared<GamerServicesComponent>(*this));
        }

        ~JoinRefusedGame() override
        {
            if (joiner_.joinable())
            {
                joiner_.join();
            }
            if (!options_.joiner && !options_.signals.empty())
            {
                std::error_code ignored;
                std::filesystem::remove_all(options_.signals, ignored);
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
                session_.reset(NetworkSession::Create(NetworkSessionType::SystemLink, 1, 2));
                joinedToken_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                    if (!args.getGamerProperty()->getIsLocalProperty())
                    {
                        report_.Log("GamerJoined: " + args.getGamerProperty()->getGamertagProperty());
                        remoteJoined_ = args.getGamerProperty()->getGamertagProperty();
                    }
                });
                report_.Log("hosting a System Link session; AllowJoinInProgress " +
                            std::string(session_->getAllowJoinInProgressProperty() ? "true" : "false"));
                report_.Check(!session_->getAllowJoinInProgressProperty(), "AllowJoinInProgress is false by default");

                options_.signals = std::filesystem::temp_directory_path() /
                                   ("cna-demos-join-refused-" + std::to_string(Clock::now().time_since_epoch().count()));
                std::filesystem::create_directories(options_.signals);
                std::vector<std::string> arguments = {"--joiner", "--signals", options_.signals.string(), "--log-level",
                                                      options_.logLevel};
                if (options_.automated)
                {
                    arguments.insert(arguments.begin(), "--automated");
                }
                joiner_ = std::thread([this, arguments] {
                    joinerExit_ = CnaDemos::RunProcess(
                        CnaDemos::Standalone::ThisProgram("cna-demo-net-network_session-join_refused"), arguments);
                    joinerDone_ = true;
                });
                return;
            }
            session_->Update();
            if (step_ == 0 && Signalled("found"))
            {
                // The joiner has found the session in its lobby. Now the game begins.
                session_->StartGame();
                session_->Update();
                report_.Log("StartGame: the session is " +
                            std::string(session_->getSessionStateProperty() == NetworkSessionState::Playing
                                            ? "Playing"
                                            : "not Playing"));
                Signal("started");
                step_ = 1;
            }
            else if (step_ == 1 && Signalled("refused"))
            {
                report_.Check(remoteJoined_.empty() && session_->getAllGamersProperty().getCountProperty() == 1,
                              "the game in progress kept the joiner out");
                // Late joiners welcome: the same session now takes the joiner.
                session_->setAllowJoinInProgressProperty(true);
                session_->Update();
                report_.Log("AllowJoinInProgress = true");
                Signal("open");
                step_ = 2;
            }
            if (joinerDone_)
            {
                joiner_.join();
                report_.Log("the joiner exited with code " + std::to_string(joinerExit_.load()));
                report_.Check(joinerExit_ == 0, "the joiner was refused, then joined");
                report_.Check(remoteJoined_ == JoinerGamertag, "Second Player joined the game in progress (GamerJoined)");
                Finish();
            }
        }

        // --- joiner ------------------------------------------------------------------------
        void UpdateJoiner()
        {
            if (step_ == 0 || step_ == 2)
            {
                // Find the session (the lobby the first time, the game in progress the second).
                // The results stay alive until Join, which needs its search's collection.
                if (!Find())
                {
                    if (Clock::now() - stepStarted_ > Patience)
                    {
                        report_.Check(false, "Demo Player's session was found");
                        Finish();
                    }
                    return;
                }
                if (step_ == 0)
                {
                    Signal("found");
                    step_ = 1;
                    stepStarted_ = Clock::now();
                    return;
                }
                session_.reset(NetworkSession::Join(available_));
                report_.Log("joined");
                ReleaseSearch();
                joinedAt_ = Clock::now();
                step_ = 3;
                return;
            }
            if (step_ == 1)
            {
                if (!Signalled("started"))
                {
                    return;
                }
                // The session found in the lobby is being played now, without join-in-progress.
                try
                {
                    session_.reset(NetworkSession::Join(available_));
                    report_.Check(false, "Join refuses a game in progress");
                }
                catch (const NetworkSessionJoinException& exception)
                {
                    report_.Log("NetworkSessionJoinException: " + std::string(exception.what()));
                    report_.Log("JoinError: " + JoinErrorName(exception.getJoinErrorProperty()));
                    System::Runtime::Serialization::SerializationInfo serialized;
                    const System::Runtime::Serialization::StreamingContext context;
                    exception.GetObjectData(serialized, context);
                    const std::vector<std::string> names = serialized.GetNames();
                    report_.Check(names == std::vector<std::string>{"Message", "InnerException", "joinError"} &&
                                      serialized.GetString("Message") == exception.what() &&
                                      serialized.GetInt32("joinError") ==
                                          static_cast<SharpRuntime::intcs>(exception.getJoinErrorProperty()),
                                  "GetObjectData preserves the real refusal message and JoinError in base-first order");
                    // (CNA-DEMOS-ISSUE-032: CNA reports SessionNotFound -- the host closes the
                    // connection without saying why.)
                    report_.Check(exception.getJoinErrorProperty() == NetworkSessionJoinError::SessionNotJoinable,
                                  "the JoinError is SessionNotJoinable");
                }
                CloseSession();
                ReleaseSearch();
                Signal("refused");
                step_ = 2;
                stepStarted_ = Clock::now();
                return;
            }
            session_->Update();
            if (Clock::now() - joinedAt_ > std::chrono::milliseconds(500))
            {
                report_.Check(session_->getAllGamersProperty().getCountProperty() == 2, "both gamers are in the game");
                // (CNA-DEMOS-ISSUE-033: the late joiner's session stays in Lobby.)
                report_.Log(std::string("the joiner's session: ") +
                            (session_->getSessionStateProperty() == NetworkSessionState::Playing ? "Playing" : "Lobby"));
                report_.Check(session_->getSessionStateProperty() == NetworkSessionState::Playing,
                              "the joiner is in the game in progress (SessionState Playing)");
                CloseSession();
                report_.Log("left the session");
                Finish();
            }
        }

        // One Find; true when Demo Player's session is among the results (kept in available_).
        bool Find()
        {
            if (step_ == 2 && !Signalled("open"))
            {
                return false;
            }
            ReleaseSearch();
            search_.emplace(NetworkSession::Find(NetworkSessionType::SystemLink, 1, NetworkSessionProperties()));
            const auto& sessions = std::as_const(*search_);
            for (int i = 0; i < sessions.getCountProperty(); ++i)
            {
                if (sessions[i].getHostGamertagProperty() == HostGamertag)
                {
                    available_ = &sessions[i];
                    report_.Log("found Demo Player's session");
                    return true;
                }
            }
            return false;
        }

        // --- helpers -----------------------------------------------------------------------
        void Signal(const char* name)
        {
            std::ofstream(options_.signals / name).put('\n');
        }

        [[nodiscard]] bool Signalled(const char* name) const
        {
            return std::filesystem::exists(options_.signals / name);
        }

        void ReleaseSearch()
        {
            available_ = nullptr;
            if (search_)
            {
                search_->Dispose();
                search_.reset();
            }
        }

        void CloseSession()
        {
            if (session_)
            {
                if (!options_.joiner)
                {
                    session_->GamerJoined.Remove(joinedToken_);
                }
                session_->Dispose();
                session_.reset();
            }
        }

        void Finish()
        {
            finished_ = true;
            ReleaseSearch();
            if (!options_.automated)
            {
                report_.Log("press Escape or close the window");
            }
        }

        GraphicsDeviceManager graphics_;
        Options options_;
        CnaDemos::Standalone::Report report_;
        std::unique_ptr<NetworkSession> session_;
        std::optional<AvailableNetworkSessionCollection> search_;
        const AvailableNetworkSession* available_ = nullptr;
        System::EventHandler<GamerJoinedEventArgs>::Token joinedToken_{};
        std::thread joiner_;
        std::atomic<bool> joinerDone_{false};
        std::atomic<int> joinerExit_{-1};
        std::string remoteJoined_;
        int step_ = 0;
        Clock::time_point started_{};
        Clock::time_point stepStarted_ = Clock::now();
        Clock::time_point joinedAt_{};
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
    for (std::size_t i = 0; i < options.other.size(); ++i)
    {
        options.joiner = options.joiner || options.other[i] == "--joiner";
        if (options.other[i] == "--signals" && i + 1 < options.other.size())
        {
            options.signals = options.other[++i];
        }
    }

    try
    {
        JoinRefusedGame game(options);
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
