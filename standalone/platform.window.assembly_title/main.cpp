// SPDX-License-Identifier: MS-PL
// A process-wide assembly title must be installed before Game creates its real platform window.
#include "CNA/AssemblyInfo.hpp"
#include "CNA/Platform/NativeWindowHandle.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

using namespace Microsoft::Xna::Framework;

namespace
{
    constexpr std::string_view AttributeTitle = "CNA Demos declarative title";
    constexpr std::string_view ExplicitTitle = "CNA Demos assembly title";
    const CNA::AssemblyTitleAttributeEXT AssemblyTitle{std::string(AttributeTitle)};

    class AssemblyTitleGame final : public Game
    {
    public:
        explicit AssemblyTitleGame(const int frames)
            : graphics_(this),
              frames_(frames)
        {
        }

        std::string windowTitle;
        bool hasWindow = false;

    protected:
        void LoadContent() override
        {
            windowTitle = getWindowProperty().getTitleProperty();
            const auto system = getWindowProperty().GetNativeWindowHandleEXT().system;
            hasWindow = system != CNA::Platform::NativeWindowSystem::Unknown &&
                        system != CNA::Platform::NativeWindowSystem::Headless &&
                        system != CNA::Platform::NativeWindowSystem::Terminal;
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);
            Game::Draw(gameTime);
            if (++drawn_ >= frames_)
                Exit();
        }

    private:
        GraphicsDeviceManager graphics_;
        int frames_;
        int drawn_ = 0;
    };
}

int main(int argc, char** argv)
{
    int frames = 3;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string_view(argv[i]) == "--frames" && i + 1 < argc)
            frames = std::atoi(argv[++i]);
    }
    if (frames < 1)
        frames = 1;

    const bool attributeRegistered = CNA::GetAssemblyTitleEXT() == AttributeTitle;
    CNA::SetAssemblyTitleEXT(std::string(ExplicitTitle));
    const bool explicitRegistered = CNA::GetAssemblyTitleEXT() == ExplicitTitle;

    AssemblyTitleGame game(frames);
    game.Run();

    if (!game.hasWindow)
    {
        std::cout << "SKIPPED: the active renderer created no real platform window\n";
        return 77;
    }

    std::cout << "Attribute title: " << AttributeTitle << '\n'
              << "Explicit title: " << CNA::GetAssemblyTitleEXT() << '\n'
              << "Initial window title: " << game.windowTitle << '\n';

    const bool ok = attributeRegistered && explicitRegistered && game.windowTitle == ExplicitTitle;
    std::cout << (ok ? "OK" : "UNEXPECTED: assembly and initial window titles differ") << '\n';
    return ok ? 0 : 1;
}
