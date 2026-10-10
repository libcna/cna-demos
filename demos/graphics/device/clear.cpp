// SPDX-License-Identifier: MS-PL
// The smallest graphical CNA program: clear the back buffer every frame.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class ClearDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Draw(const GameTime& gameTime) override
        {
            // Fills the whole render target (here: the back buffer) with one colour.
            // Game presents the frame itself after Draw returns.
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            Game::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(ClearDemo, {
    .id = "graphics.device.clear",
    .title = "Clear the screen",
    .description = "Clears the back buffer to Color::CornflowerBlue every frame - the minimal Draw "
                   "override of a CNA game.",
    .apis = {"GraphicsDevice::Clear", "Game::Draw", "Color::CornflowerBlue"},
    .tags = {"clear", "hello"},
    .frames = 10,
    .deterministic = true,
})
