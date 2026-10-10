// SPDX-License-Identifier: MS-PL
// Runtime renderer selection. This is a complete program rather than a demo inside cna-demos:
// CNA latches the renderer when the first GraphicsDevice of a process is created, so a choice
// can only be made at the start of a fresh process.
//
// Arguments: [--automated] [--frames N]. Exit code 0 = success, 1 = failure.
#include <cstdlib>
#include <iostream>
#include <string_view>

#include "CNA/GraphicsRendererSelection.hpp"
#include "CNA/GraphicsRendererType.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class SelectedRendererGame final : public Game
    {
    public:
        explicit SelectedRendererGame(int frames)
            : graphics_(this),
              frames_(frames)
        {
        }

        std::string rendererName;

    protected:
        void LoadContent() override
        {
            // The device reports the renderer that was actually created.
            rendererName = std::string(getGraphicsDeviceProperty().GetGraphicsRendererName());
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::MediumPurple);
            Game::Draw(gameTime);
            if (frames_ > 0 && ++drawn_ >= frames_)
            {
                Exit();
            }
        }

    private:
        GraphicsDeviceManager graphics_;
        int frames_;
        int drawn_ = 0;
    };
}

int main(int argc, char** argv)
{
    int frames = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string_view(argv[i]) == "--frames" && i + 1 < argc)
        {
            frames = std::atoi(argv[++i]);
        }
    }

    using CNA::GraphicsRendererSelection;
    using CNA::GraphicsRendererType;

    // Which renderers were compiled into this program (a multi-renderer build has several).
    std::cout << "Compiled-in renderers:";
    for (const GraphicsRendererType type : GraphicsRendererSelection::GetAvailable())
    {
        std::cout << ' ' << CNA::getGraphicsRendererName(type);
    }
    std::cout << "\nDefault/selected before any choice: "
              << CNA::getGraphicsRendererName(GraphicsRendererSelection::GetSelected()) << '\n';

    // Pick SOFTWARE when it is compiled in (it needs no display), otherwise keep the default.
    // SetPreferred must run before the first GraphicsDevice is constructed.
    GraphicsRendererType wanted = GraphicsRendererSelection::GetSelected();
    if (GraphicsRendererSelection::IsAvailable(GraphicsRendererType::Software))
    {
        wanted = GraphicsRendererType::Software;
    }
    GraphicsRendererSelection::SetPreferred(wanted);
    std::cout << "SetPreferred(" << CNA::getGraphicsRendererName(wanted) << ")\n";

    SelectedRendererGame game(frames);
    game.Run();

    const GraphicsRendererType active = GraphicsRendererSelection::GetActive();
    std::cout << "Active renderer: " << CNA::getGraphicsRendererName(active)
              << " (device reports " << game.rendererName << ")\n"
              << "Selection latched: " << (GraphicsRendererSelection::IsLatched() ? "yes" : "no") << '\n';

    const bool ok = active == wanted && GraphicsRendererSelection::IsLatched()
                    && game.rendererName == CNA::getGraphicsRendererName(wanted);
    std::cout << (ok ? "OK" : "UNEXPECTED: the active renderer differs from the preferred one") << '\n';
    return ok ? 0 : 1;
}
