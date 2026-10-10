// SPDX-License-Identifier: MS-PL
#pragma once

#include "CnaDemos/DemoContext.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"

namespace CnaDemos
{
    /**
     * Base class of every in-process Game demo. It is an ordinary
     * Microsoft::Xna::Framework::Game: a demo overrides LoadContent/Update/Draw exactly as an XNA
     * game would, and everything the demo is about stays visible in the demo's own source.
     *
     * The base class only adds what every demo would otherwise repeat:
     *   - a GraphicsDeviceManager (reachable through Graphics());
     *   - Content.RootDirectory = "assets" (the shared demo assets next to the executable);
     *   - a window title naming the demo;
     *   - checking the demo's declared Needs against the device before LoadContent runs, and
     *     reporting SKIPPED with the reason when the renderer cannot meet them;
     *   - ending the demo after a fixed number of frames in automated runs (or with --frames);
     *   - Escape (keyboard) or Back (gamepad 1) to leave an interactive run;
     *   - optional capture of the last frame to a PNG (--screenshot).
     *
     * A demo that overrides Initialize() or EndDraw() must call DemoGame::Initialize() /
     * DemoGame::EndDraw() -- the same rule XNA has for Game::Initialize().
     */
    class DemoGame : public Microsoft::Xna::Framework::Game
    {
    public:
        DemoGame();
        ~DemoGame() override;

        /** The context of this run (options, output, checks). */
        [[nodiscard]] DemoContext& Context() { return context_; }

        /** Frames drawn so far (counted in EndDraw). */
        [[nodiscard]] int FramesDrawn() const { return framesDrawn_; }

    protected:
        [[nodiscard]] Microsoft::Xna::Framework::GraphicsDeviceManager& Graphics() { return graphics_; }

        void Initialize() override;
        void EndDraw() override;

    private:
        void CheckNeeds();
        void CaptureScreenshot();

        DemoContext& context_;
        Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
        int framesDrawn_ = 0;
        int frameLimit_ = 0;
    };
}
