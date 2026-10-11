// SPDX-License-Identifier: MS-PL
// SignedInGamer::Presence: telling friends what the player is doing.
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/GamerServicesNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPresence.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPresenceMode.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class PresenceDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            player_ = (*Gamer::getSignedInGamersProperty())[PlayerIndex::One];
            if (player_ == nullptr)
            {
                return;
            }
            auto& ctx = Context();
            GamerPresence& presence = player_->getPresenceProperty();
            ++updates_;
            // A game updates the presence as the player moves through it: menu, then stage 1,
            // then stage 2. Some modes carry a number (PresenceValue), shown as "Stage 2".
            if (updates_ == 1)
            {
                ctx.Check(presence.getPresenceModeProperty() == GamerPresenceMode::None,
                          "a freshly signed-in gamer has no presence (None)");
                presence.setPresenceModeProperty(GamerPresenceMode::AtMenu);
            }
            else if (updates_ == 20)
            {
                presence.setPresenceModeProperty(GamerPresenceMode::Stage);
                presence.setPresenceValueProperty(1);
            }
            else if (updates_ == 40)
            {
                presence.setPresenceValueProperty(2);
                ctx.Check(presence.getPresenceModeProperty() == GamerPresenceMode::Stage &&
                              presence.getPresenceValueProperty() == 2,
                          "PresenceMode and PresenceValue read back");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(44, 30, 50));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "SignedInGamer::Presence", Vector2(40.0f, 30.0f), Color::White);
            if (player_ != nullptr)
            {
                const GamerPresence& presence = std::as_const(*player_).getPresenceProperty();
                spriteBatch_->DrawString(*font_,
                                         "PresenceMode:  " + std::string(CnaDemos::Name(presence.getPresenceModeProperty())),
                                         Vector2(40.0f, 80.0f), Color::Plum);
                spriteBatch_->DrawString(*font_, "PresenceValue: " + std::to_string(presence.getPresenceValueProperty()),
                                         Vector2(40.0f, 110.0f), Color::Plum);
                spriteBatch_->DrawString(*font_, "(offline it stays on this machine; with a CNA service",
                                         Vector2(40.0f, 170.0f), Color::Gray);
                spriteBatch_->DrawString(*font_, " friends see it after the next dispatcher update)",
                                         Vector2(40.0f, 200.0f), Color::Gray);
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        SignedInGamer* player_ = nullptr;
        int updates_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PresenceDemo, {
    .id = "gamer_services.signed_in_gamer.presence",
    .title = "Rich presence",
    .description = "Sets the signed-in gamer's GamerPresence as play moves from the menu to stage 1 and stage 2 "
                   "(PresenceMode AtMenu, then Stage with PresenceValue); offline the presence is only kept locally.",
    .apis = {"SignedInGamer::Presence", "GamerPresence::PresenceMode", "GamerPresence::PresenceValue",
             "GamerPresenceMode"},
    .tags = {"gamer-services", "presence", "friends", "status"},
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
