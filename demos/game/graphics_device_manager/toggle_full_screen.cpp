// SPDX-License-Identifier: MS-PL
// Switching between windowed and full screen: ToggleFullScreen and IsFullScreen.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class ToggleFullScreenDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Update(GameTime& gameTime) override
        {
            // Press F to toggle. Compare with the previous frame so one key press toggles once.
            const bool fDown = Keyboard::GetState().IsKeyDown(Keys::F);
            if (fDown && !fWasDown_)
            {
                Graphics().ToggleFullScreen();
                Context().Print("IsFullScreen = {}", Graphics().getIsFullScreenProperty());
            }
            fWasDown_ = fDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Graphics().getIsFullScreenProperty() ? Color::DarkRed : Color::DarkBlue);
            Game::Draw(gameTime);
        }

    private:
        bool fWasDown_ = false;
    };
}

CNA_DEMOS_GAME(ToggleFullScreenDemo, {
    .id = "game.graphics_device_manager.toggle_full_screen",
    .title = "Toggle full screen",
    .description = "Press F to switch between windowed and full-screen mode with "
                   "GraphicsDeviceManager::ToggleFullScreen; the colour shows IsFullScreen.",
    .apis = {"GraphicsDeviceManager::ToggleFullScreen", "GraphicsDeviceManager::IsFullScreen"},
    .tags = {"fullscreen", "window", "display"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .restrictions = "Manual: changes the display mode of the user's screen, so it is never run unattended.",
})
