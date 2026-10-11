// SPDX-License-Identifier: MS-PL
// The window's drawable area: GameWindow::ClientBounds.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class ClientBoundsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            const Rectangle bounds = getWindowProperty().getClientBoundsProperty();
            const auto& parameters = getGraphicsDeviceProperty().getPresentationParametersProperty();
            Context().Print("ClientBounds = {}, back buffer {}x{}", bounds.ToString(), parameters.getBackBufferWidthProperty(),
                            parameters.getBackBufferHeightProperty());
            // The client area is what the back buffer is presented into.
            Context().Check(bounds.Width == parameters.getBackBufferWidthProperty()
                                && bounds.Height == parameters.getBackBufferHeightProperty(),
                            "the client area matches the back-buffer size");
        }
    };
}

CNA_DEMOS_GAME(ClientBoundsDemo, {
    .id = "game.window.client_bounds",
    .title = "Window client bounds",
    .description = "Reads GameWindow::ClientBounds, the window's drawable area, and compares it with the back-buffer "
                   "size.",
    .apis = {"GameWindow::ClientBounds", "PresentationParameters::BackBufferWidth",
             "PresentationParameters::BackBufferHeight"},
    .tags = {"window", "size"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
})
