// SPDX-License-Identifier: MS-PL
// Polling the keyboard once per frame with Keyboard::GetState.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class KeyboardStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Update(GameTime& gameTime) override
        {
            // A snapshot of every key, taken once per frame.
            const KeyboardState keyboard = Keyboard::GetState();

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
            Game::Draw(gameTime);
        }

    private:
        bool spaceDown_ = false;
        std::size_t lastPressedCount_ = 0;
    };
}

CNA_DEMOS_GAME(KeyboardStateDemo, {
    .id = "input.keyboard.get_state",
    .title = "Keyboard state",
    .description = "Polls Keyboard::GetState every frame; the screen turns green while Space is held and the "
                   "number of pressed keys is printed when it changes.",
    .apis = {"Keyboard::GetState", "KeyboardState::IsKeyDown", "KeyboardState::GetPressedKeys"},
    .tags = {"keys", "polling"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
})
