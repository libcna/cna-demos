// SPDX-License-Identifier: MS-PL
// GamerServicesComponent and Gamer::SignedInGamers: who is playing, announced by SignedIn.
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarDescription.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerServicesDispatcher.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInEventArgs.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class AutomaticSignInDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void Initialize() override
        {
            // SignedIn is static. Subscribe before the first Update: profiles that sign in when
            // the game starts appear -- and raise SignedIn -- in that Update, as on XNA.
            signedIn_ = SignedInGamer::SignedIn.Add([this](System::Object*, const SignedInEventArgs& args) {
                announced_.push_back(args.getGamerProperty()->getGamertagProperty());
            });
            DemoGame::Initialize(); // initializes the components: GamerServicesComponent starts the dispatcher

            Context().Check(GamerServicesDispatcher::getIsInitializedProperty(),
                            "GamerServicesComponent initialized GamerServicesDispatcher");
            Context().Check(Gamer::getSignedInGamersProperty()->getCountProperty() == 0,
                            "before the first Update nobody is signed in yet");
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override { SignedInGamer::SignedIn.Remove(signedIn_); }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime); // updates GamerServicesComponent, which runs the dispatcher

            const SignedInGamerCollection& gamers = *Gamer::getSignedInGamersProperty();
            if (!checked_ && gamers.getCountProperty() > 0)
            {
                checked_ = true;
                SignedInGamer* player = gamers[PlayerIndex::One];
                auto& ctx = Context();
                ctx.Print("signed in: {} (display name {}), PlayerIndex {}, IsSignedInToLive {}, IsGuest {}",
                          player->getGamertagProperty(), player->getDisplayNameProperty(),
                          static_cast<int>(player->getPlayerIndexProperty()), player->getIsSignedInToLiveProperty(),
                          player->getIsGuestProperty());
                ctx.Check(player == gamers[0], "SignedInGamers[PlayerIndex::One] is the first signed-in gamer");
                ctx.Check(player->getPlayerIndexProperty() == PlayerIndex::One, "the gamer plays as player one");
                ctx.Check(!player->getIsSignedInToLiveProperty() && !player->getIsGuestProperty(),
                          "a local offline profile: not signed in to a service, not a guest");
                ctx.Check(announced_.size() == 1 && announced_.front() == player->getGamertagProperty(),
                          "SignedIn announced the gamer once, in the first Update");

                // Offline profiles still use the public asynchronous pair. CNA completes this
                // local read synchronously, invokes the callback before Begin returns and carries
                // the caller's state through the token.
                int avatarCallbacks = 0;
                bool avatarStateMatched = false;
                std::unique_ptr<System::IAsyncResult> avatarRequest(AvatarDescription::BeginGetFromGamer(
                    player,
                    [&avatarCallbacks, &avatarStateMatched](System::IAsyncResult& result) {
                        ++avatarCallbacks;
                        avatarStateMatched =
                            std::any_cast<std::string>(result.getAsyncStateProperty()) == "Demo Player avatar";
                    },
                    std::string("Demo Player avatar")));
                ctx.Check(avatarCallbacks == 1 && avatarStateMatched && avatarRequest->getIsCompletedProperty() &&
                              avatarRequest->getCompletedSynchronouslyProperty(),
                          "BeginGetFromGamer completes locally, calls back once, and preserves caller state");
                const AvatarDescription avatar = AvatarDescription::EndGetFromGamer(avatarRequest.get());
                ctx.Check(avatar.getIsValidProperty() && avatar.getDescriptionProperty().size() == 1021 &&
                              avatar.getHeightProperty() > 0.0f,
                          "EndGetFromGamer returns the signed-in local profile's stored avatar");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(24, 32, 48));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Gamer::SignedInGamers", Vector2(40.0f, 30.0f), Color::White);
            const SignedInGamerCollection& gamers = *Gamer::getSignedInGamersProperty();
            float y = 80.0f;
            if (gamers.getCountProperty() == 0)
            {
                spriteBatch_->DrawString(*font_, "nobody is signed in", Vector2(40.0f, y), Color::Gray);
            }
            for (SignedInGamer* gamer : gamers)
            {
                const std::string line = "Player " + std::to_string(static_cast<int>(gamer->getPlayerIndexProperty()) + 1)
                    + ": " + gamer->getGamertagProperty()
                    + (gamer->getIsSignedInToLiveProperty() ? "  (online)" : "  (local profile)");
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::LightGreen);
                y += 30.0f;
            }
            for (const std::string& name : announced_)
            {
                spriteBatch_->DrawString(*font_, "SignedIn event: " + name, Vector2(40.0f, y + 20.0f), Color::Khaki);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        System::EventHandler<SignedInEventArgs>::Token signedIn_{};
        std::vector<std::string> announced_;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(AutomaticSignInDemo, {
    .id = "gamer_services.signed_in_gamer.automatic_sign_in",
    .title = "Signed-in gamers",
    .description = "Adds a GamerServicesComponent; the local profile that signs in when the game starts appears in "
                   "Gamer::SignedInGamers and raises SignedInGamer::SignedIn in the first Update, then its stored "
                   "avatar is read through AvatarDescription's Begin/End pair.",
    .apis = {"GamerServicesComponent", "GamerServicesDispatcher::IsInitialized", "Gamer::SignedInGamers",
             "SignedInGamer::SignedIn", "SignedInEventArgs::Gamer", "Gamer::Gamertag", "Gamer::DisplayName",
             "SignedInGamer::PlayerIndex", "SignedInGamer::IsSignedInToLive", "SignedInGamer::IsGuest",
             "SignedInGamerCollection::Item", "AvatarDescription::BeginGetFromGamer",
             "AvatarDescription::EndGetFromGamer"},
    .tags = {"gamer-services", "profile", "sign-in", "gamertag", "player"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
