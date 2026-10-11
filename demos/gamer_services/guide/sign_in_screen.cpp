// SPDX-License-Identifier: MS-PL
// Guide::ShowSignIn: the Guide's profile picker, here for a second player joining.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Guide.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
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
    class SignInScreenDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            // Unattended, the picker is still up at the last frame (the screenshot shows it):
            // close it as Escape would.
            if (Context().Automated() && Guide::getIsVisibleProperty())
            {
                Guide::SimulateKeyboardInputCancelEXT();
                Context().Check(!Guide::getIsVisibleProperty(), "Escape closes the sign-in screen");
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (++updates_ == 5)
            {
                // Two panes: players one and two. Player one is signed in already, so the
                // Guide asks only for player two -- "press Start to join". onlineOnly = false
                // offers local profiles: the picker lists this machine's profiles that are not
                // signed in and can create a new one; whoever signs in appears in
                // Gamer::SignedInGamers and raises SignedInGamer::SignedIn.
                Guide::ShowSignIn(2, false);
                Context().Check(Guide::getIsVisibleProperty(), "ShowSignIn opens the Guide's sign-in screen");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 36, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Guide::ShowSignIn(2, false)", Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_,
                                     "Signed in: " + std::to_string(Gamer::getSignedInGamersProperty()->getCountProperty()),
                                     Vector2(40.0f, 70.0f), Color::LightGray);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        int updates_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SignInScreenDemo, {
    .id = "gamer_services.guide.sign_in_screen",
    .title = "Sign-in screen",
    .description = "Lets a second player join: Guide::ShowSignIn with two panes skips player one, who is signed in "
                   "already, and asks player two to pick a local profile (from the demos' own profile store) or create "
                   "one; Escape closes it.",
    .apis = {"Guide::ShowSignIn", "Guide::IsVisible", "Guide::SimulateKeyboardInputCancelEXT"},
    .tags = {"gamer-services", "guide", "sign-in", "profile", "multiplayer", "ui"},
    .frames = 40,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
