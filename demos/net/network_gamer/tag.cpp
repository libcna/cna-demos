// SPDX-License-Identifier: MS-PL
// Gamer::Tag on network gamers: attaching the game's own player object to each gamer.
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
#include "Microsoft/Xna/Framework/Net/GamerJoinedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    // The game's own per-player state.
    struct Tank
    {
        Vector2 position;
        Color color;
        int lives = 3;
    };

    class TagDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        TagDemo() : GamerServicesDemo("Demo Player,Second Player") {}

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
                session_->GamerJoined.Remove(joined_);
                session_->Dispose();
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
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
                // The usual XNA pattern: give every gamer who joins a game object, kept in Tag.
                joined_ = session_->GamerJoined.Add([this](System::Object*, const GamerJoinedEventArgs& args) {
                    NetworkGamer* gamer = args.getGamerProperty();
                    const int index = static_cast<int>(session_->getAllGamersProperty().IndexOf(gamer));
                    gamer->setTagProperty(Tank{Vector2(80.0f + 360.0f * static_cast<float>(index), 220.0f),
                                               index == 0 ? Color::CornflowerBlue : Color::Orange, 3});
                });
            }
            session_->Update();

            // Every frame the game finds each player's object through the gamer.
            for (NetworkGamer* gamer : session_->getAllGamersProperty())
            {
                if (Tank* tank = std::any_cast<Tank>(&gamer->getTagProperty()))
                {
                    tank->position.X += gamer->getIsHostProperty() ? 0.5f : -0.5f;
                }
            }
            if (++updates_ == 10)
            {
                int tagged = 0;
                for (NetworkGamer* gamer : session_->getAllGamersProperty())
                {
                    tagged += std::any_cast<Tank>(&gamer->getTagProperty()) != nullptr ? 1 : 0;
                }
                Context().Check(tagged == 2, "each gamer carries its Tank in Tag");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 44, 30));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "NetworkGamer.Tag = the game's player object", Vector2(30.0f, 30.0f),
                                     Color::White);
            if (session_)
            {
                for (NetworkGamer* gamer : session_->getAllGamersProperty())
                {
                    if (const Tank* tank = std::any_cast<Tank>(&gamer->getTagProperty()))
                    {
                        spriteBatch_->DrawString(*font_, "[#] " + gamer->getGamertagProperty(), tank->position, tank->color);
                        spriteBatch_->DrawString(*font_, "lives " + std::to_string(tank->lives),
                                                 tank->position + Vector2(0.0f, 24.0f), tank->color);
                    }
                }
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<NetworkSession> session_;
        System::EventHandler<GamerJoinedEventArgs>::Token joined_{};
        int waited_ = 0;
        int updates_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(TagDemo, {
    .id = "net.network_gamer.tag",
    .title = "Game objects in Gamer.Tag",
    .description = "Gives every gamer who joins a session the game's own player object through Gamer::Tag in the "
                   "GamerJoined handler, then finds and moves each player's object through its gamer every frame.",
    .apis = {"Gamer::Tag", "NetworkSession::GamerJoined", "NetworkSession::AllGamers", "NetworkGamer::IsHost"},
    .tags = {"net", "gamer", "tag", "game-state", "multiplayer"},
    .frames = 40,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
