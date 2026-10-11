// SPDX-License-Identifier: MS-PL
// NetworkSession::SimulatedLatency and SimulatedPacketLoss: test a game on a bad connection from
// a good one. This is a complete program rather than a demo inside cna-demos: a process holds one
// NetworkSession, so the sender is a second process -- this program started again with --sender.
// The sender sends finite bursts: baseline, 200 ms latency, then complete simulated loss.
// The last burst gets a recovery interval with loss disabled before checking reliable delivery.
// Empty files coordinate phases independently of the network being deliberately impaired.
//
// Arguments: [--automated] [--screenshot PATH] [--log-level LEVEL] [--sender --signals DIR] (--frames is
// accepted and ignored: the run lasts as long as the exchange takes).
// Exit code 0 = every check passed, 1 = a check failed.
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
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
    constexpr int BurstPackets = 24;
    constexpr std::chrono::milliseconds SendInterval(20);
    constexpr std::chrono::milliseconds SettleTime(600);
    constexpr std::chrono::milliseconds RecoveryTime(1000);
    constexpr int Latency = 200; // ms

    struct Options : CnaDemos::Standalone::Arguments
    {
        bool sender = false;
        std::filesystem::path signals;
    };

    // Both processes run on one machine: steady_clock is the same clock in each, so the send time
    // a packet carries can be compared with its arrival.
    long long NowNanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
    }

    struct Phase
    {
        std::array<std::set<int>, 2> received; // kind 0: unreliable, 1: reliable
        std::vector<double> delaysMs;

        [[nodiscard]] double MeanDelay() const
        {
            double sum = 0.0;
            for (double d : delaysMs)
                sum += d;
            return delaysMs.empty() ? 0.0 : sum / static_cast<double>(delaysMs.size());
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
            if (!options_.sender && !options_.signals.empty())
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
            if (Clock::now() - started_ > Patience)
            {
                report_.Check(false, "the coordinated packet exchange completes before its deadline");
                finished_ = true;
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
                options_.signals = std::filesystem::temp_directory_path() /
                    ("cna-demos-simulated-" + std::to_string(Clock::now().time_since_epoch().count()));
                std::filesystem::create_directories(options_.signals);
                report_.Log("hosting; starting the sender (24 packet pairs per phase)");
                std::vector<std::string> arguments = {"--sender", "--signals", options_.signals.string(),
                                                      "--log-level", options_.logLevel};
                if (options_.automated) arguments.emplace_back("--automated");
                sender_ = std::thread([this, arguments] {
                    const auto program =
                        CnaDemos::Standalone::ThisProgram("cna-demo-net-network_session-simulated_conditions");
                    senderExit_ = CnaDemos::RunProcess(program, arguments);
                    senderDone_ = true;
                });
                return;
            }
            session_->Update();
            Receive();
            if (phase_ < 0 && session_->getRemoteGamersProperty().getCountProperty() > 0)
                BeginPhase(0);
            if (phase_ >= 0 && Signalled("sent-" + std::to_string(phase_)))
            {
                if (!settling_) settling_ = Clock::now();
                if (Clock::now() - *settling_ >= SettleTime)
                {
                    if (phase_ < 2)
                        BeginPhase(phase_ + 1);
                    else if (!recovering_)
                    {
                        // Keep the sender connected: reliable packets lost during the outage
                        // must have a chance to be retransmitted after the connection recovers.
                        session_->setSimulatedPacketLossProperty(0.0f);
                        recovering_ = Clock::now();
                        report_.Log("outage ended; waiting for reliable recovery");
                    }
                    else if (Clock::now() - *recovering_ >= RecoveryTime)
                        Signal("complete");
                }
            }
            if (senderDone_)
            {
                sender_.join();
                session_->Update();
                Receive();
                Evaluate();
                finished_ = true;
            }
        }

        void BeginPhase(int phase)
        {
            phase_ = phase;
            settling_.reset();
            session_->setSimulatedLatencyProperty(System::TimeSpan::FromMilliseconds(phase == 1 ? Latency : 0));
            session_->setSimulatedPacketLossProperty(phase == 2 ? 1.0f : 0.0f);
            report_.Log(phase == 0 ? "baseline: no simulated delay or loss"
                       : phase == 1 ? "latency: 200 ms, no simulated loss" : "outage: 100% simulated loss");
            Signal("go-" + std::to_string(phase));
        }

        void Receive()
        {
            LocalNetworkGamer* me = session_->getLocalGamersProperty()[0];
            while (me->getIsDataAvailableProperty())
            {
                PacketReader reader;
                NetworkGamer* from = nullptr;
                me->ReceiveData(reader, from);
                const int phase = reader.ReadInt32();
                const int kind = reader.ReadInt32();
                const int sequence = reader.ReadInt32();
                const long long sent = reader.ReadInt64();
                if (phase < 0 || phase >= 3 || kind < 0 || kind >= 2 || sequence < 0 || sequence >= BurstPackets)
                    throw std::runtime_error("Received an invalid packet from the demo sender");
                auto& result = phases_[static_cast<std::size_t>(phase)];
                result.received[static_cast<std::size_t>(kind)].insert(sequence);
                result.delaysMs.push_back(static_cast<double>(NowNanoseconds() - sent) / 1.0e6);
            }
        }

        void Evaluate()
        {
            for (std::size_t phase = 0; phase < phases_.size(); ++phase)
            {
                const auto& result = phases_[phase];
                char line[160];
                if (result.delaysMs.empty())
                    std::snprintf(line, sizeof(line), "phase %zu: no packets arrived (sent %d of each kind)",
                                  phase, BurstPackets);
                else
                    std::snprintf(line, sizeof(line), "phase %zu: mean %.0f ms; unreliable %zu/%d, reliable %zu/%d",
                                  phase, result.MeanDelay(), result.received[0].size(), BurstPackets,
                                  result.received[1].size(), BurstPackets);
                report_.Log(line);
            }
            report_.Check(senderExit_ == 0, "the sender completed all bursts and the recovery interval");
            // None has no delivery guarantee, even on loopback. Its measured count remains
            // visible; the exact authored denominator exposes missing first/last packets too.
            report_.Check(!phases_[0].received[0].empty() && phases_[0].received[1].size() == BurstPackets,
                          "baseline receives both streams and every reliable packet");
            const auto& delayed = phases_[1];
            report_.Check(delayed.received[1].size() == BurstPackets && !delayed.delaysMs.empty() &&
                          std::all_of(delayed.delaysMs.begin(), delayed.delaysMs.end(),
                                      [](double ms) { return ms >= Latency - 1.0; }),
                          "every delayed packet waits at least the configured 200 ms");
            report_.Check(phases_[2].received[0].empty(), "the complete outage drops all unreliable packets");
            report_.Check(phases_[2].received[1].size() == BurstPackets,
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
                        report_.Log("joined; waiting for the host's phases");
                    }
                }
                found.Dispose();
                return;
            }
            session_->Update();
            if (Signalled("complete"))
            {
                session_->Dispose();
                session_.reset();
                finished_ = true;
                return;
            }
            if (phase_ < 2 && Signalled("go-" + std::to_string(phase_ + 1)))
            {
                ++phase_;
                sequence_ = 0;
                burstFinished_.reset();
                nextSend_ = Clock::now();
            }
            if (phase_ < 0) return;
            if (sequence_ < BurstPackets && Clock::now() >= nextSend_ &&
                session_->getRemoteGamersProperty().getCountProperty() > 0)
            {
                LocalNetworkGamer* me = session_->getLocalGamersProperty()[0];
                NetworkGamer* host = session_->getRemoteGamersProperty()[0];
                for (int kind = 0; kind < 2; ++kind)
                {
                    writer_.Write(phase_);
                    writer_.Write(kind);
                    writer_.Write(sequence_);
                    writer_.Write(static_cast<SharpRuntime::longcs>(NowNanoseconds()));
                    me->SendData(writer_, kind == 0 ? SendDataOptions::None : SendDataOptions::Reliable, host);
                }
                ++sequence_;
                // Wall-clock pacing avoids a burst of sends during fixed-step catch-up.
                nextSend_ = Clock::now() + SendInterval;
                if (sequence_ == BurstPackets) burstFinished_ = Clock::now();
            }
            if (burstFinished_ && Clock::now() - *burstFinished_ >= SettleTime)
                Signal("sent-" + std::to_string(phase_));
        }

        void Signal(const std::string& name)
        {
            if (Signalled(name)) return;
            std::ofstream marker(options_.signals / name);
            marker.put('\n');
            if (!marker) throw std::runtime_error("Cannot write phase marker: " + name);
        }

        [[nodiscard]] bool Signalled(const std::string& name) const
        {
            return std::filesystem::exists(options_.signals / name);
        }

        GraphicsDeviceManager graphics_;
        Options options_;
        CnaDemos::Standalone::Report report_;
        std::unique_ptr<NetworkSession> session_;
        Clock::time_point started_{};
        int phase_ = -1;
        // The host's side.
        std::thread sender_;
        std::atomic<bool> senderDone_{false};
        std::atomic<int> senderExit_{-1};
        std::optional<Clock::time_point> settling_;
        std::optional<Clock::time_point> recovering_;
        std::array<Phase, 3> phases_;
        // The sender's side.
        PacketWriter writer_;
        int sequence_ = 0;
        Clock::time_point nextSend_{};
        std::optional<Clock::time_point> burstFinished_;
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
        if (options.other[i] == "--sender") options.sender = true;
        else if (options.other[i] == "--signals" && i + 1 < options.other.size()) options.signals = options.other[++i];
    }
    if (options.sender && options.signals.empty())
    {
        std::cerr << "The sender requires --signals DIR.\n";
        return 1;
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
