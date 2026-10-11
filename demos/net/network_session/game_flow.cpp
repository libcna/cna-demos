// SPDX-License-Identifier: MS-PL
// NetworkSession game flow: Lobby -> ready -> StartGame (Playing) -> EndGame (back to the Lobby).
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
#include "Microsoft/Xna/Framework/Net/GameEndedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/GameStartedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionState.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    const char* StateName(NetworkSessionState state)
    {
        switch (state)
        {
            case NetworkSessionState::Lobby: return "Lobby";
            case NetworkSessionState::Playing: return "Playing";
            case NetworkSessionState::Ended: return "Ended";
        }
        return "?";
    }

    class GameFlowDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        GameFlowDemo() : GamerServicesDemo("Demo Player,Second Player") {}

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
                session_->GameStarted.Remove(startedToken_);
                session_->GameEnded.Remove(endedToken_);
                session_->Dispose();
                session_.reset();
            }
        }

        LocalNetworkGamer* Player(int index) const { return session_->getLocalGamersProperty()[index]; }

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
                session_.reset(NetworkSession::Create(NetworkSessionType::Local, 2, 2));
                startedToken_ = session_->GameStarted.Add(
                    [this](System::Object*, const GameStartedEventArgs&) { log_.push_back("GameStarted"); });
                endedToken_ = session_->GameEnded.Add(
                    [this](System::Object*, const GameEndedEventArgs&) { log_.push_back("GameEnded"); });
            }
            session_->Update();
            ++updates_;

            // In the lobby each player says they are ready; the host starts when everyone is.
            if (updates_ == 10)
            {
                Player(0)->setIsReadyProperty(true);
                log_.push_back(Player(0)->getGamertagProperty() + " is ready");
                ctx.Check(!session_->getIsEveryoneReadyProperty(), "one ready player is not everyone");
            }
            else if (updates_ == 20)
            {
                Player(1)->setIsReadyProperty(true);
                log_.push_back(Player(1)->getGamertagProperty() + " is ready");
                ctx.Check(session_->getIsEveryoneReadyProperty(), "IsEveryoneReady once both are");
                session_->StartGame();
                ctx.Check(session_->getSessionStateProperty() == NetworkSessionState::Lobby,
                          "StartGame takes effect at the next NetworkSession::Update");
            }
            else if (updates_ == 21)
            {
                ctx.Check(session_->getSessionStateProperty() == NetworkSessionState::Playing && log_.back() == "GameStarted",
                          "the session is Playing and GameStarted was raised");
            }
            else if (updates_ == 50)
            {
                session_->EndGame();
            }
            else if (updates_ == 51)
            {
                ctx.Check(session_->getSessionStateProperty() == NetworkSessionState::Lobby && log_.back() == "GameEnded",
                          "EndGame returns to the Lobby and raises GameEnded");
                ctx.Check(!Player(0)->getIsReadyProperty() && !Player(1)->getIsReadyProperty(),
                          "ending the game clears every gamer's IsReady: the lobby starts over");
                for (const std::string& line : log_)
                {
                    ctx.Print("{}", line);
                }
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            const bool playing = session_ && session_->getSessionStateProperty() == NetworkSessionState::Playing;
            getGraphicsDeviceProperty().Clear(playing ? Color(30, 60, 40) : Color(36, 40, 56));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "NetworkSession game flow", Vector2(40.0f, 30.0f), Color::White);
            if (session_)
            {
                spriteBatch_->DrawString(*font_,
                                         std::string("SessionState: ") + StateName(session_->getSessionStateProperty()),
                                         Vector2(40.0f, 70.0f), Color::Khaki);
                float y = 120.0f;
                for (const std::string& line : log_)
                {
                    spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::LightGray);
                    y += 28.0f;
                }
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GameStartedEventArgs>::Token startedToken_{};
        System::EventHandler<GameEndedEventArgs>::Token endedToken_{};
        std::vector<std::string> log_;
        int waited_ = 0;
        int updates_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(GameFlowDemo, {
    .id = "net.network_session.game_flow",
    .title = "Lobby and game flow",
    .description = "Walks a Local session through a round: both players set IsReady, IsEveryoneReady turns true, "
                   "the host's StartGame makes the session Playing (GameStarted) at the next Update, and EndGame "
                   "returns it to the Lobby (GameEnded) with every IsReady cleared.",
    .apis = {"NetworkGamer::IsReady", "NetworkSession::IsEveryoneReady", "NetworkSession::StartGame",
             "NetworkSession::EndGame", "NetworkSession::GameStarted", "NetworkSession::GameEnded",
             "NetworkSession::SessionState", "NetworkSessionState", "NetworkSession::Update",
             "GameStartedEventArgs", "GameEndedEventArgs"},
    .tags = {"net", "session", "lobby", "ready", "multiplayer"},
    .frames = 70,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
