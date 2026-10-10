// SPDX-License-Identifier: MS-PL
// Letting the user resize the window: AllowUserResizing and the ClientSizeChanged event.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/EventArgs.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class UserResizingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            GameWindow& window = getWindowProperty();
            window.setAllowUserResizingProperty(true);
            Context().Check(window.getAllowUserResizingProperty(), "AllowUserResizing reads back true");

            // Drag the window border: the event reports each new size.
            window.ClientSizeChanged += [this](System::Object*, const System::EventArgs&) {
                const auto bounds = getWindowProperty().getClientBoundsProperty();
                Context().Print("ClientSizeChanged: now {}x{}", bounds.Width, bounds.Height);
            };
            DemoGame::Initialize();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::OliveDrab);
            Game::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(UserResizingDemo, {
    .id = "game.window.user_resizing",
    .title = "Resizable window",
    .description = "Enables GameWindow::AllowUserResizing and prints each new size from the ClientSizeChanged event "
                   "while you drag the window border.",
    .apis = {"GameWindow::AllowUserResizing", "GameWindow::ClientSizeChanged", "GameWindow::ClientBounds"},
    .tags = {"window", "resize", "event"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Mouse,
})
