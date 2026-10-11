// SPDX-License-Identifier: MS-PL
// NetworkSession::SimulatedLatency and SimulatedPacketLoss: test a game on a bad connection from
// a good one. This is a complete program rather than a demo inside cna-demos: a process holds one
// NetworkSession, so the sender is a second process -- this program started again with --sender.
// The sender streams packets (one unreliable, one reliable per frame); the host receives them,
// first as they are, then with 200 ms of latency and 30 % loss simulated.
//
// Arguments: [--automated] [--screenshot PATH] [--log-level LEVEL] [--sender] (--frames is
// accepted and ignored: the run lasts as long as the exchange takes).
// Exit code 0 = every check passed, 1 = a check failed.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
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
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Net/PacketReader.hpp"
#include "Microsoft/Xna/Framework/Net/PacketWriter.hpp"
#include "Microsoft/Xna/Framework/Net/SendDataOptions.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    constexpr const char* HostGamertag = "Demo Player";
    constexpr const char* SenderGamertag = "Second Player";
    using Clock = std::chrono::steady_clock;
    constexpr std::chrono::seconds Patience(20);
    // The sender streams this long; the host switches the simulation on halfway.
    constexpr std::chrono::milliseconds StreamTime(3000);
    constexpr std::chrono::milliseconds SwitchAt(1500);
    constexpr std::chrono::milliseconds Margin(150);
    constexpr int Latency = 200;     // ms
    constexpr float Loss = 0.3f;

    struct Options : CnaDemos::Standalone::Arguments
    {
        bool sender = false;
    };

    // Both processes run on one machine: steady_clock is the same clock in each, so the send time
    // a packet carries can be compared with its arrival.
    long long NowNanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
    }

    struct Phase
    {
        std::map<int, std::set<int>> received; // kind (0 unreliable, 1 reliable) -> sequence numbers
        std::vector<double> delaysMs;

        [[nodiscard]] double MeanDelay() const
        {
            double sum = 0.0;
            for (double d : delaysMs)
                sum += d;
            return delaysMs.empty() ? 0.0 : sum / static_cast<double>(delaysMs.size());
        }

        // Received / sent, the sent ones being every sequence number between the first and last
        // received of that kind.
        [[nodiscard]] double Delivered(int kind) const
        {
            const auto found = received.find(kind);
            if (found == received.end() || found->second.empty())
                return 0.0;
            const int span = *found->second.rbegin() - *found->second.begin() + 1;
            return static_cast<double>(found->second.size()) / span;
        }
    };

    class SimulatedConditionsGame final : public Game
    {
    public:
        explicit SimulatedConditionsGame(const Options& options)
            : graphics_(this),
              options_(options),
              report_(options.sender ? "sender" : "host")
        {
            getContentProperty().setRootDirectoryProperty("assets");
            graphics_.setGraphicsProfileProperty(GraphicsProfile::HiDef); // the screenshot reads the back buffer
            CnaDemos::UseOfflineDemoProfiles();
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_AUTO_SIGN_IN", options.sender ? SenderGamertag : HostGamertag);
            getComponentsProperty().Add(std::make_shared<GamerServicesComponent>(*this));
        }

        ~SimulatedConditionsGame() override
        {
            if (sender_.joinable())
            {
                sender_.join();
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
            if (sender_.joinable())
            {
                sender_.join();
            }
            if (session_)
            {
                session_->Dispose();
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            Game::Update(gameTime);
            if (!options_.automated && !options_.sender && Input::Keyboard::GetState().IsKeyDown(Input::Keys::Escape))
            {
                Exit();
            }
            if (finished_ || Gamer::getSignedInGamersProperty()->getCountProperty() == 0)
            {
                return;
            }
            if (options_.sender)
            {
                UpdateSender();
            }
            else
            {
                UpdateHost();
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(options_.sender ? Color(40, 30, 52) : Color(24, 40, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, options_.sender ? "Sender (second process)" : "Host: simulated conditions",
                                     Vector2(30.0f, 20.0f), Color::White);
            float y = 64.0f;
            for (const std::string& line : report_.Lines())
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
            if (finished_ && !options_.screenshot.empty() && !options_.sender && !captured_)
            {
                captured_ = true;
                CnaDemos::Standalone::SaveScreenshot(device, options_.screenshot);
            }
            if (finished_ && (options_.automated || options_.sender))
            {
                Exit();
            }
        }

    private:
        // --- the host, which receives (first process) -------------------------------------------
        void UpdateHost()
        {
            if (!session_)
            {
                session_.reset(NetworkSession::Create(NetworkSessionType::SystemLink, 1, 2));
                report_.Log("hosting; starting the sender");
                std::vector<std::string> arguments = {"--sender", "--log-level", options_.logLevel};
                if (options_.automated)
                {
                    arguments.emplace_back("--automated");
                }
                sender_ = std::thread([this, arguments] {
                    const auto program =
                        CnaDemos::Standalone::ThisProgram("cna-demo-net-network_session-simulated_conditions");
                    senderExit_ = CnaDemos::RunProcess(program, arguments);
                    senderDone_ = true;
                });
                return;
            }
            session_->Update();
            if (!streamStart_ && session_->getRemoteGamersProperty().getCountProperty() > 0)
            {
                streamStart_ = Clock::now();
            }
            if (streamStart_ && !switched_ && Clock::now() - *streamStart_ > SwitchAt)
            {
                // A bad connection, simulated for what arrives at this machine.
                switched_ = Clock::now();
                session_->setSimulatedLatencyProperty(System::TimeSpan::FromMilliseconds(Latency));
                session_->setSimulatedPacketLossProperty(Loss);
                report_.Log("SimulatedLatency 200 ms, SimulatedPacketLoss 0.3");
            }
            Receive();
            if (senderDone_)
            {
                sender_.join();
                session_->Update();
                Receive();
                Evaluate();
                finished_ = true;
            }
        }

        void Receive()
        {
            LocalNetworkGamer* me = session_->getLocalGamersProperty()[0];
            while (me->getIsDataAvailableProperty())
            {
                PacketReader reader;
                NetworkGamer* from = nullptr;
                me->ReceiveData(reader, from);
                const int kind = reader.ReadInt32();
                const int sequence = reader.ReadInt32();
                const long long sent = reader.ReadInt64();
                const double delayMs = static_cast<double>(NowNanoseconds() - sent) / 1.0e6;
                // Sorted by when the packet was sent, with a margin around the switch.
                const Clock::time_point sentAt{std::chrono::nanoseconds(sent)};
                Phase* phase = nullptr;
                if (switched_ && sentAt > *switched_ + Margin)
                    phase = &bad_;
                else if (!switched_ || sentAt < *switched_ - Margin)
                    phase = &good_;
                if (phase != nullptr)
                {
                    phase->received[kind].insert(sequence);
                    phase->delaysMs.push_back(delayMs);
                }
            }
        }

        void Evaluate()
        {
            char line[160];
            std::snprintf(line, sizeof(line), "as is:     delay %.0f ms, unreliable %.0f %%, reliable %.0f %%",
                          good_.MeanDelay(), good_.Delivered(0) * 100.0, good_.Delivered(1) * 100.0);
            report_.Log(line);
            std::snprintf(line, sizeof(line), "simulated: delay %.0f ms, unreliable %.0f %%, reliable %.0f %%",
                          bad_.MeanDelay(), bad_.Delivered(0) * 100.0, bad_.Delivered(1) * 100.0);
            report_.Log(line);
            report_.Check(senderExit_ == 0, "the sender joined, streamed and left");
            // As is, a packet waits for the next frame of each side (both draw at 60 Hz).
            report_.Check(good_.MeanDelay() < 150.0 && good_.Delivered(0) > 0.9 && good_.Delivered(1) == 1.0,
                          "as is, packets arrive within a few frames and (nearly) all of them");
            const double added = bad_.MeanDelay() - good_.MeanDelay();
            report_.Check(added > Latency - 50.0 && added < Latency + 100.0,
                          "SimulatedLatency adds about 200 ms to every packet");
            report_.Check(bad_.Delivered(0) > 0.5 && bad_.Delivered(0) < 0.9,
                          "with SimulatedPacketLoss 0.3, about 70 % of the unreliable packets arrive");
            // SendDataOptions::Reliable guarantees delivery: a lost reliable packet is sent again.
            report_.Check(bad_.Delivered(1) == 1.0,
                          "reliable packets are never lost (CNA-DEMOS-ISSUE-030: CNA drops them too)");
        }

        // --- the sender (second process) ------------------------------------------------------
        void UpdateSender()
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
                        report_.Log("joined; streaming for 3 s");
                        streamStart_ = Clock::now();
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
            session_->Update();
            const auto elapsed = Clock::now() - *streamStart_;
            if (elapsed < StreamTime && session_->getRemoteGamersProperty().getCountProperty() > 0)
            {
                LocalNetworkGamer* me = session_->getLocalGamersProperty()[0];
                NetworkGamer* host = session_->getRemoteGamersProperty()[0];
                for (int kind = 0; kind < 2; ++kind)
                {
                    writer_.Write(kind);
                    writer_.Write(sequence_);
                    writer_.Write(static_cast<SharpRuntime::longcs>(NowNanoseconds()));
                    me->SendData(writer_, kind == 0 ? SendDataOptions::None : SendDataOptions::Reliable, host);
                }
                ++sequence_;
            }
            // Linger so the last reliable packets get through, then leave.
            if (elapsed > StreamTime + std::chrono::milliseconds(1500))
            {
                session_->Dispose();
                session_.reset();
                finished_ = true;
            }
        }

        GraphicsDeviceManager graphics_;
        Options options_;
        CnaDemos::Standalone::Report report_;
        std::unique_ptr<NetworkSession> session_;
        Clock::time_point started_{};
        std::optional<Clock::time_point> streamStart_;
        // The host's side.
        std::thread sender_;
        std::atomic<bool> senderDone_{false};
        std::atomic<int> senderExit_{-1};
        std::optional<Clock::time_point> switched_;
        Phase good_;
        Phase bad_;
        // The sender's side.
        PacketWriter writer_;
        int sequence_ = 0;
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
        options.sender = options.sender || argument == "--sender";
    }

    try
    {
        SimulatedConditionsGame game(options);
        game.Run();
        if (!options.sender)
        {
            std::cout << (game.Failures() == 0 ? "== PASSED\n" : "== FAILED\n");
        }
        return game.Failures() == 0 ? 0 : 1;
    }
    catch (const std::exception& exception)
    {
        std::cout << (options.sender ? "sender" : "host") << ": exception: " << exception.what() << '\n';
        return 1;
    }
}
