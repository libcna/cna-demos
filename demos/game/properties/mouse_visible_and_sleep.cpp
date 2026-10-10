// SPDX-License-Identifier: MS-PL
// Game properties for desktop behaviour: IsMouseVisible and InactiveSleepTime.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class GamePropertiesDemo final : public CnaDemos::DemoGame
    {
    public:
        GamePropertiesDemo()
        {
            // Show the system cursor over the window (hidden by default, as in XNA).
            Context().Check(!getIsMouseVisibleProperty(), "the mouse cursor is hidden by default");
            setIsMouseVisibleProperty(true);

            // How long the loop sleeps per frame while the window is in the background
            // (XNA's default is 20 ms), saving CPU when the player switches away.
            Context().Print("default InactiveSleepTime = {} ms",
                            getInactiveSleepTimeProperty().getTotalMillisecondsProperty());
            setInactiveSleepTimeProperty(System::TimeSpan::FromMilliseconds(100.0));
        }

    protected:
        void LoadContent() override
        {
            Context().Check(getIsMouseVisibleProperty(), "IsMouseVisible reads back true");
            Context().Check(getInactiveSleepTimeProperty().getTotalMillisecondsProperty() == 100.0,
                            "InactiveSleepTime reads back 100 ms");
            Context().Print("IsActive (window has focus) = {}", getIsActiveProperty());
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(getIsActiveProperty() ? Color::SeaGreen : Color::Gray);
            Game::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(GamePropertiesDemo, {
    .id = "game.properties.mouse_visible_and_sleep",
    .title = "Cursor visibility and background sleep",
    .description = "Shows the mouse cursor with Game::IsMouseVisible, sets Game::InactiveSleepTime, and colours the "
                   "screen by Game::IsActive (whether the window has focus).",
    .apis = {"Game::IsMouseVisible", "Game::InactiveSleepTime", "Game::IsActive"},
    .tags = {"cursor", "focus", "desktop"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Mouse,
    .frames = 5,
})
