// SPDX-License-Identifier: MS-PL
// NetworkSession::BeginCreate / EndCreate: creating a session without blocking the game loop.
#include <any>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IAsyncResult.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    class BeginCreateDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            delete pending_;
            pending_ = nullptr;
            if (session_)
            {
                session_->Dispose();
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            if (!started_ && Gamer::getSignedInGamersProperty()->getCountProperty() > 0)
            {
                started_ = true;
                // The game keeps drawing (a "creating session..." screen) while this runs; the
                // callback receives the result. The caller owns the IAsyncResult.
                pending_ = NetworkSession::BeginCreate(
                    NetworkSessionType::Local, 1, 4,
                    [this](System::IAsyncResult& result) {
                        callbackState_ = std::any_cast<std::string>(result.getAsyncStateProperty());
                    },
                    std::string("create-lobby"));
            }
            // Polling IsCompleted is the other way to wait.
            if (pending_ != nullptr && pending_->getIsCompletedProperty())
            {
                session_.reset(NetworkSession::EndCreate(pending_));
                delete pending_;
                pending_ = nullptr;
                ctx.Check(session_ != nullptr && session_->getLocalGamersProperty().getCountProperty() == 1,
                          "EndCreate returned the session with the signed-in player in it");
                ctx.Check(callbackState_ == "create-lobby", "the callback ran with the state passed to BeginCreate");
                status_ = "session created: " + std::to_string(session_->getAllGamersProperty().getCountProperty()) +
                    " gamer, " + std::to_string(session_->getMaxGamersProperty()) + " slots";
                ctx.Print("{}", status_);
            }
            if (session_)
            {
                session_->Update();
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(34, 38, 50));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "NetworkSession::BeginCreate", Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, status_.empty() ? std::string("creating session...") : status_,
                                     Vector2(40.0f, 80.0f), Color::LightGreen);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        bool started_ = false;
        System::IAsyncResult* pending_ = nullptr;
        std::unique_ptr<NetworkSession> session_;
        std::string callbackState_;
        std::string status_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(BeginCreateDemo, {
    .id = "net.network_session.begin_create",
    .title = "Creating a session asynchronously",
    .description = "Starts NetworkSession::BeginCreate while the game keeps running, sees the callback with its "
                   "state object, and takes the session from EndCreate once IsCompleted.",
    .apis = {"NetworkSession::BeginCreate", "NetworkSession::EndCreate", "IAsyncResult::IsCompleted",
             "IAsyncResult::AsyncState"},
    .tags = {"net", "session", "async", "lobby"},
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
