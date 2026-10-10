// SPDX-License-Identifier: MS-PL
// GraphicsDevice::Clear with ClearOptions: choose which planes -- colour, depth, stencil -- to clear.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ClearOptionsDemo final : public CnaDemos::DemoGame
    {
    public:
        ClearOptionsDemo()
        {
            // A 24-bit depth buffer and no stencil buffer behind the back buffer.
            Graphics().setPreferredDepthStencilFormatProperty(DepthFormat::Depth24);
        }

    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // Asking to clear a plane the surface does not have is an error, not a silent no-op.
            bool threw = false;
            try
            {
                device.Clear(ClearOptions::Stencil, Color::Black, 1.0f, 0);
            }
            catch (const System::InvalidOperationException& e)
            {
                threw = true;
                Context().Print("Clear(ClearOptions::Stencil, ...) without a stencil buffer: {}", e.what());
            }
            Context().Check(threw, "clearing the stencil of a Depth24 back buffer throws InvalidOperationException");
        }

        void Draw(const GameTime& gameTime) override
        {
            // Colour to DarkSlateBlue and depth to 1.0 (the far plane) in one call -- what a 3D game
            // does at the start of every frame. The stencil value is ignored: Stencil is not requested.
            getGraphicsDeviceProperty().Clear(ClearOptions::Target | ClearOptions::DepthBuffer, Color::DarkSlateBlue,
                                              1.0f, 0);

            Game::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(ClearOptionsDemo, {
    .id = "graphics.device.clear_options",
    .title = "Clear selected buffers",
    .description = "Clears colour and depth with ClearOptions::Target | ClearOptions::DepthBuffer, and shows that "
                   "requesting ClearOptions::Stencil on a back buffer without a stencil buffer throws.",
    .apis = {"GraphicsDevice::Clear", "ClearOptions::Target", "ClearOptions::DepthBuffer", "ClearOptions::Stencil",
             "GraphicsDeviceManager::PreferredDepthStencilFormat"},
    .tags = {"clear", "depth", "stencil"},
    .frames = 5,
    .deterministic = true,
})
