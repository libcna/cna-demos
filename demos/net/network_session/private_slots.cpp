// SPDX-License-Identifier: MS-PL
// NetworkSession::PrivateGamerSlots: slots held back for invited friends. Players who simply
// join -- here, local players pressing Start -- may use only the public ones.
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
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionProperties.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    constexpr const char* Players = "Demo Player,Second Player,Third Player";

    class PrivateSlotsDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        PrivateSlotsDemo() : GamerServicesDemo(Players) {}

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

            // A session cannot be all private: someone has to be able to join.
            bool refused = false;
            try
            {
                std::unique_ptr<NetworkSession> invalid(NetworkSession::Create(
                    NetworkSessionType::Local, std::vector<SignedInGamer*>{first}, 3, 3, NetworkSessionProperties()));
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                refused = true;
            }
            ctx.Check(refused, "Create refuses as many private slots as gamers (ArgumentOutOfRangeException)");

            // Three slots, one of them kept for a friend player one will invite.
            session_.reset(NetworkSession::Create(NetworkSessionType::Local, std::vector<SignedInGamer*>{first}, 3, 1,
                                                  NetworkSessionProperties()));
            session_->Update();
            Log("MaxGamers 3, PrivateGamerSlots " + std::to_string(session_->getPrivateGamerSlotsProperty()));
            ctx.Check(session_->getPrivateGamerSlotsProperty() == 1 && session_->getMaxGamersProperty() == 3,
                      "the session has three slots, one private");
            ctx.Check(!session_->getLocalGamersProperty()[0]->getIsPrivateSlotProperty(),
                      "player one, who created it, holds a public slot");

            session_->AddLocalGamer(second);
            session_->Update();
            Log("Second Player joins: " + std::to_string(session_->getAllGamersProperty().getCountProperty()) +
                " gamers");
            ctx.Check(session_->getAllGamersProperty().getCountProperty() == 2,
                      "player two takes the second public slot");

            // One slot is free, but it is the private one.
            refused = false;
            try
            {
                session_->AddLocalGamer(third);
            }
            catch (const System::InvalidOperationException&)
            {
                refused = true;
            }
            Log(std::string("Third Player: ") + (refused ? "refused, only the private slot is free" : "joined"));
            ctx.Check(refused, "player three is refused: the free slot is private (InvalidOperationException)");

            // The host cannot reserve slots that players already hold...
            refused = false;
            try
            {
                session_->setPrivateGamerSlotsProperty(2);
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                refused = true;
            }
            ctx.Check(refused && session_->getPrivateGamerSlotsProperty() == 1,
                      "PrivateGamerSlots 2 is refused while two players hold public slots");

            // ...but can open the reserved one to everybody.
            session_->setPrivateGamerSlotsProperty(0);
            session_->AddLocalGamer(third);
            session_->Update();
            Log("PrivateGamerSlots 0: Third Player joins, " +
                std::to_string(session_->getAllGamersProperty().getCountProperty()) + " gamers");
            ctx.Check(session_->getAllGamersProperty().getCountProperty() == 3,
                      "with PrivateGamerSlots 0, player three takes the last slot");
            for (const std::string& line : log_)
            {
                ctx.Print("{}", line);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 34, 46));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Private gamer slots", Vector2(40.0f, 30.0f), Color::White);
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
        void Log(const std::string& line) { log_.push_back(line); }

        std::unique_ptr<NetworkSession> session_;
        std::vector<std::string> log_;
        bool done_ = false;
        int waited_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PrivateSlotsDemo, {
    .id = "net.network_session.private_slots",
    .title = "Private gamer slots",
    .description = "A three-slot Local session with one private slot (kept for an invited friend): player two takes "
                   "the last public slot, player three is refused although a slot is free, PrivateGamerSlots cannot "
                   "reserve held slots, and setting it to 0 lets player three in.",
    .apis = {"NetworkSession::PrivateGamerSlots", "NetworkSession::MaxGamers", "NetworkGamer::IsPrivateSlot",
             "NetworkSession::AddLocalGamer", "NetworkSession::Create", "NetworkSession::AllGamers"},
    .tags = {"net", "multiplayer", "slots", "invite", "lobby"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
