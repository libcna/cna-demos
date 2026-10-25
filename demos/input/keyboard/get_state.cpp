// SPDX-License-Identifier: MS-PL
// Polling the keyboard once per frame with Keyboard::GetState.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class KeyboardStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            // A snapshot of every key, taken once per frame.
            const KeyboardState keyboard = Keyboard::GetState();

            // XNA also exposes four local-player overloads. A desktop has one physical
            // keyboard, so all four slots intentionally see the same published snapshot.
            const std::array playerStates = {
                Keyboard::GetState(PlayerIndex::One), Keyboard::GetState(PlayerIndex::Two),
                Keyboard::GetState(PlayerIndex::Three), Keyboard::GetState(PlayerIndex::Four)};
            if (!checkedPlayerSlots_)
            {
                Context().Check(playerStates[0] == keyboard && playerStates[1] == keyboard &&
                                    playerStates[2] == keyboard && playerStates[3] == keyboard,
                                "all four Keyboard::GetState(PlayerIndex) slots expose the physical keyboard");
                checkedPlayerSlots_ = true;
            }

            spaceDown_ = keyboard.IsKeyDown(Keys::Space);

            // Report changes in the set of pressed keys on the console.
            const auto pressed = keyboard.GetPressedKeys();
            if (pressed.size() != lastPressedCount_)
            {
                lastPressedCount_ = pressed.size();
                Context().Print("{} key(s) down", pressed.size());
            }

            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            // The screen turns green while Space is held.
            getGraphicsDeviceProperty().Clear(spaceDown_ ? Color::ForestGreen : Color::DimGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Hold Space", Vector2(40.0f, 40.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool spaceDown_ = false;
        bool checkedPlayerSlots_ = false;
        std::size_t lastPressedCount_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(KeyboardStateDemo, {
    .id = "input.keyboard.get_state",
    .title = "Keyboard state",
    .description = "Polls Keyboard::GetState every frame, including all four PlayerIndex overloads; the screen turns "
                   "green while Space is held and the number of pressed keys is printed when it changes.",
    .apis = {"Keyboard::GetState", "Keyboard::GetState(PlayerIndex)", "KeyboardState::IsKeyDown",
             "KeyboardState::GetPressedKeys"},
    .tags = {"keys", "polling", "player-index"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
})
