// SPDX-License-Identifier: MS-PL
#include "CnaDemos/DemoGame.hpp"

#include <string>

#include "CNA/GraphicsCapability.hpp"
#include "CNA/Platform/NativeWindowSystem.hpp"
#include "CnaDemos/Metadata.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace CnaDemos
{
    namespace
    {
        struct CapabilityNeed
        {
            Need need;
            CNA::GraphicsCapability capability;
        };

        // Needs that map one-to-one onto a renderer capability query.
        constexpr CapabilityNeed CapabilityNeeds[] = {
            {Need::ThreeD, CNA::GraphicsCapability::ThreeD},
            {Need::CustomEffects, CNA::GraphicsCapability::CustomEffects},
            {Need::Instancing, CNA::GraphicsCapability::Instancing},
            {Need::MultipleRenderTargets, CNA::GraphicsCapability::MultipleRenderTargets},
            {Need::Texture3D, CNA::GraphicsCapability::Texture3D},
            {Need::OcclusionQuery, CNA::GraphicsCapability::OcclusionQuery},
            {Need::StencilBuffer, CNA::GraphicsCapability::StencilBuffer},
            {Need::WireFrame, CNA::GraphicsCapability::WireFrame},
            {Need::MultiSampling, CNA::GraphicsCapability::MultiSampleAntiAliasing},
            {Need::CompiledEffects, CNA::GraphicsCapability::CompiledEffects},
            {Need::FloatRenderTargets, CNA::GraphicsCapability::FloatRenderTargets},
            {Need::AnisotropicFiltering, CNA::GraphicsCapability::AnisotropicFiltering},
            {Need::IndirectDraw, CNA::GraphicsCapability::IndirectDraw},
            {Need::ComputeShaders, CNA::GraphicsCapability::ComputeShaders},
        };

        std::string WindowTitle(const DemoEntry& entry)
        {
            return std::string(entry.info.title) + " - CNA Demos (" + std::string(entry.info.id) + ")";
        }
    }

    DemoGame::DemoGame()
        : context_(DemoContext::Current()),
          graphics_(this),
          frameLimit_(context_.FrameLimit())
    {
        getContentProperty().setRootDirectoryProperty("assets");
        getWindowProperty().setTitleProperty(WindowTitle(context_.Entry()));
    }

    DemoGame::~DemoGame() = default;

    void DemoGame::Initialize()
    {
        CheckNeeds();
        Game::Initialize();
    }

    void DemoGame::CheckNeeds()
    {
        const Need needs = context_.Entry().info.needs;
        GraphicsDevice& device = getGraphicsDeviceProperty();
        for (const auto& [need, capability] : CapabilityNeeds)
        {
            if (HasNeed(needs, need) && !device.SupportsCapability(capability))
            {
                std::string name;
                for (const auto& [candidate, text] : AllNeeds())
                {
                    if (candidate == need)
                    {
                        name = text;
                    }
                }
                context_.Skip("renderer " + std::string(device.GetGraphicsRendererName())
                              + " does not support '" + name + "'");
            }
        }

        if (HasNeed(needs, Need::Window))
        {
            // HEADLESS, SOFTWARE and STUB create no window; there is no capability for this, so
            // ask the window what it is attached to (the same probe cna-template uses).
            const auto system = getWindowProperty().GetNativeWindowHandleEXT().system;
            if (system == CNA::Platform::NativeWindowSystem::Unknown
                || system == CNA::Platform::NativeWindowSystem::Headless
                || system == CNA::Platform::NativeWindowSystem::Terminal)
            {
                context_.Skip("renderer " + std::string(device.GetGraphicsRendererName())
                              + " has no window");
            }
        }
    }

    void DemoGame::EndDraw()
    {
        ++framesDrawn_;
        const bool lastFrame = frameLimit_ > 0 && framesDrawn_ >= frameLimit_;

        // Read the frame back before Game::EndDraw() presents it.
        if (lastFrame && !context_.Options().screenshotPath.empty())
        {
            CaptureScreenshot();
        }

        Game::EndDraw();

        if (lastFrame)
        {
            Exit();
        }
        else if (!context_.Automated()
                 && (Keyboard::GetState().IsKeyDown(Keys::Escape)
                     || GamePad::GetState(PlayerIndex::One).IsButtonDown(Buttons::Back)))
        {
            Exit();
        }
    }

    void DemoGame::CaptureScreenshot()
    {
        GraphicsDevice& device = getGraphicsDeviceProperty();
        const auto& parameters = device.getPresentationParametersProperty();
        const int width = parameters.getBackBufferWidthProperty();
        const int height = parameters.getBackBufferHeightProperty();

        std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
        device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));

        Texture2D image(device, width, height);
        image.SetData(pixels.data(), static_cast<int>(pixels.size()));
        image.SaveAsPng(context_.Options().screenshotPath);
        context_.Out() << "  screenshot: " << context_.Options().screenshotPath << " (" << width << "x"
                       << height << ", frame " << framesDrawn_ << ")\n";
    }
}
