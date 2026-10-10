// SPDX-License-Identifier: MS-PL
// Setting the window title: GameWindow::Title.
#include <format>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class WindowTitleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Update(GameTime& gameTime) override
        {
            // Titles are often used for quick status output (FPS, level name...).
            const std::string title = std::format("Frame {} - CNA Demos", ++frame_);
            getWindowProperty().setTitleProperty(title);
            Context().Check(getWindowProperty().getTitleProperty() == title, "Title reads back what was set");
            Game::Update(gameTime);
        }

    private:
        int frame_ = 0;
    };
}

CNA_DEMOS_GAME(WindowTitleDemo, {
    .id = "game.window.title",
    .title = "Window title",
    .description = "Updates the window caption every frame through Game.Window.Title.",
    .apis = {"Game::Window", "GameWindow::Title"},
    .tags = {"window", "caption"},
    .frames = 10,
})
