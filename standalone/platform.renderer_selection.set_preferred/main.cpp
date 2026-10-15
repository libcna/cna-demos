// SPDX-License-Identifier: MS-PL
// Runtime renderer selection. This is a complete program rather than a demo inside cna-demos:
// CNA latches the renderer when the first GraphicsDevice of a process is created, so a choice
// can only be made at the start of a fresh process.
//
// Arguments: [--automated] [--frames N]. Exit code 0 = success, 1 = failure.
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include "CNA/GraphicsRendererSelection.hpp"
#include "CNA/GraphicsRendererType.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/InvalidOperationException.hpp"

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
    GraphicsRendererType wanted = GraphicsRendererSelection::GetSelected();
    if (GraphicsRendererSelection::IsAvailable(GraphicsRendererType::Software))
    {
        wanted = GraphicsRendererType::Software;
    }

    constexpr std::array publicRenderers{
        GraphicsRendererType::OpenGLES3, GraphicsRendererType::OpenGL33,
        GraphicsRendererType::WebGL2, GraphicsRendererType::WebGPU,
        GraphicsRendererType::Headless, GraphicsRendererType::Software,
        GraphicsRendererType::Stub, GraphicsRendererType::DirectX9,
        GraphicsRendererType::SdlGpu, GraphicsRendererType::Fna3d,
    };
    auto absent = publicRenderers.end();
    for (auto candidate = publicRenderers.begin(); candidate != publicRenderers.end(); ++candidate)
    {
        if (!GraphicsRendererSelection::IsAvailable(*candidate))
        {
            absent = candidate;
            break;
        }
    }
    if (absent == publicRenderers.end())
    {
        std::cout << "SKIPPED: this build contains every public renderer, so no fallback can be demonstrated\n";
        return 77;
    }

    // Automatic fallback is independently configurable. Restore its default before installing an
    // explicit one-element chain so the actual history below is deterministic.
    const bool fallbackInitiallyDisabled = !GraphicsRendererSelection::IsFallbackEnabled();
    GraphicsRendererSelection::EnableAutomaticFallback(true);
    const bool automaticFallbackEnabled = GraphicsRendererSelection::IsFallbackEnabled();
    GraphicsRendererSelection::EnableAutomaticFallback(false);
    const bool automaticFallbackDisabled = !GraphicsRendererSelection::IsFallbackEnabled();

    const std::array fallbackChain{wanted};
    GraphicsRendererSelection::SetFallbackChain(fallbackChain);
    const bool explicitFallbackEnabled = GraphicsRendererSelection::IsFallbackEnabled();

    // The string overload is case-insensitive. Ask for an absent renderer so CNA records why it
    // was rejected, then falls back to the ordinary SOFTWARE renderer.
    std::string absentName(CNA::getGraphicsRendererName(*absent));
    std::transform(absentName.begin(), absentName.end(), absentName.begin(),
                   [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    GraphicsRendererSelection::SetPreferred(absentName);
    const bool selectedRequested = GraphicsRendererSelection::GetSelected() == *absent;
    std::cout << "Requested absent renderer: " << CNA::getGraphicsRendererName(*absent)
              << "; fallback: " << CNA::getGraphicsRendererName(wanted) << '\n';

    SelectedRendererGame game(frames);
    game.Run();

    const GraphicsRendererType active = GraphicsRendererSelection::GetActive();
    const auto history = GraphicsRendererSelection::GetFallbackHistory();
    const bool historyMatches = history.size() == 1 && history[0].type == *absent &&
                                history[0].reason == CNA::GraphicsRendererFallbackReason::NotCompiledIn &&
                                !history[0].message.empty();

    bool preferredRejectedAfterLatch = false;
    bool chainRejectedAfterLatch = false;
    bool automaticRejectedAfterLatch = false;
    try
    {
        GraphicsRendererSelection::SetPreferred(wanted);
    }
    catch (const System::InvalidOperationException&)
    {
        preferredRejectedAfterLatch = true;
    }
    try
    {
        GraphicsRendererSelection::SetFallbackChain(fallbackChain);
    }
    catch (const System::InvalidOperationException&)
    {
        chainRejectedAfterLatch = true;
    }
    try
    {
        GraphicsRendererSelection::EnableAutomaticFallback(true);
    }
    catch (const System::InvalidOperationException&)
    {
        automaticRejectedAfterLatch = true;
    }

    std::cout << "Active renderer: " << CNA::getGraphicsRendererName(active)
              << " (device reports " << game.rendererName << ")\n";
    if (historyMatches)
    {
        std::cout << "Fallback: " << CNA::getGraphicsRendererFallbackReasonName(history[0].reason)
                  << " (" << history[0].message << ")\n";
    }
    else
    {
        std::cout << "Fallback: UNEXPECTED HISTORY\n";
    }
    std::cout << "Selection latched: " << (GraphicsRendererSelection::IsLatched() ? "yes" : "no") << '\n';

    const bool ok = fallbackInitiallyDisabled && automaticFallbackEnabled && automaticFallbackDisabled &&
                    explicitFallbackEnabled && selectedRequested && active == wanted && historyMatches &&
                    GraphicsRendererSelection::IsLatched() &&
                    game.rendererName == CNA::getGraphicsRendererName(wanted) &&
                    preferredRejectedAfterLatch && chainRejectedAfterLatch && automaticRejectedAfterLatch;
    std::cout << (ok ? "OK" : "UNEXPECTED: renderer fallback or latch contract failed") << '\n';
    return ok ? 0 : 1;
}
