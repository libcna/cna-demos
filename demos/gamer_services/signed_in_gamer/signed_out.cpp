// SPDX-License-Identifier: MS-PL
// SignedInGamer::SignedOut: the player signs out in the Guide; the game pauses, saves, or returns
// to the title screen.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInEventArgs.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedOutEventArgs.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SignedOutDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void Initialize() override
        {
            // Both events are static: the game hears about every player.
            signedOut_ = SignedInGamer::SignedOut.Add([this](System::Object*, const SignedOutEventArgs& args) {
                const std::string gamertag = args.getGamerProperty()->getGamertagProperty();
                log_.push_back("SignedOut: " + gamertag + " -- back to the title screen");
                // By the time the event is raised, the gamer has left SignedInGamers.
                bool listed = false;
                for (const SignedInGamer* gamer : *Gamer::getSignedInGamersProperty())
                {
                    listed = listed || gamer->getGamertagProperty() == gamertag;
                }
                Context().Check(!listed, "the signed-out gamer is no longer in Gamer::SignedInGamers");
            });
            signedIn_ = SignedInGamer::SignedIn.Add([this](System::Object*, const SignedInEventArgs& args) {
                log_.push_back("SignedIn: " + args.getGamerProperty()->getGamertagProperty());
            });
            DemoGame::Initialize();
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            SignedInGamer::SignedOut.Remove(signedOut_);
            SignedInGamer::SignedIn.Remove(signedIn_);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(26, 30, 44));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Press Home (or a pad's Guide button) to open the Guide,",
                                     Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "choose Sign out -- then sign in again from the Guide.",
                                     Vector2(40.0f, 58.0f), Color::White);
            float y = 120.0f;
            for (const std::string& line : log_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::Gold);
                y += 28.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime); // the GamerServicesComponent draws the Guide on top
        }

    private:
        System::EventHandler<SignedOutEventArgs>::Token signedOut_{};
        System::EventHandler<SignedInEventArgs>::Token signedIn_{};
        std::vector<std::string> log_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SignedOutDemo, {
    .id = "gamer_services.signed_in_gamer.signed_out",
    .title = "A player signs out",
    .description = "Logs SignedInGamer::SignedOut (SignedOutEventArgs::Gamer) and SignedIn while the player signs out "
                   "and back in through the Guide (Home opens it); the signed-out gamer has left "
                   "Gamer::SignedInGamers when the event arrives.",
    .apis = {"SignedInGamer::SignedOut", "SignedOutEventArgs", "SignedOutEventArgs::Gamer", "SignedInGamer::SignedIn",
             "Gamer::SignedInGamers"},
    .tags = {"gamer-services", "profile", "sign-out", "guide"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: offline, only the Guide's own menu signs a profile out -- a person does it.",
    .ownProcess = true,
})
