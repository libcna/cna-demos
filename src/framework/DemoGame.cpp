// SPDX-License-Identifier: MS-PL
#include "CnaDemos/DemoGame.hpp"

#include <string>

#include "CNA/GraphicsCapability.hpp"
#include "CNA/Platform/NativeWindowSystem.hpp"
#include "CnaDemos/Metadata.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
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
            {Need::HalfFloatRenderTargets, CNA::GraphicsCapability::HalfFloatRenderTargets},
            {Need::AnisotropicFiltering, CNA::GraphicsCapability::AnisotropicFiltering},
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

#ifndef CNA_VIDEO_AVAILABLE
        // CNA decodes video only when built with its FFmpeg backend (CNA_ENABLE_VIDEO).
        if (HasNeed(needs, Need::Video))
        {
            context_.Skip("CNA was built without video decoding (CNA_ENABLE_VIDEO)");
        }
#endif
    }

    void DemoGame::EndDraw()
    {
        ++framesDrawn_;
        const bool lastFrame = frameLimit_ > 0 && framesDrawn_ >= frameLimit_;

        // Read the frame back before Game::EndDraw() presents it. Every frame is kept, so a demo
        // that ends itself early (Exit() once its checks are done) is captured as well.
        if (!context_.Options().screenshotPath.empty())
        {
            ReadBackFrame();
            if (lastFrame)
            {
                SaveScreenshot();
            }
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

    void DemoGame::EndRun()
    {
        // The demo ended before its frame limit: save the last frame it drew.
        if (!context_.Options().screenshotPath.empty())
        {
            SaveScreenshot();
        }
        Game::EndRun();
    }

    void DemoGame::ReadBackFrame()
    {
        GraphicsDevice& device = getGraphicsDeviceProperty();
        // XNA refuses back-buffer readback under Reach. A demo about Reach still runs; it just
        // cannot be photographed this way.
        if (device.getGraphicsProfileProperty() == GraphicsProfile::Reach)
        {
            return;
        }
        const auto& parameters = device.getPresentationParametersProperty();
        const std::size_t count = static_cast<std::size_t>(parameters.getBackBufferWidthProperty()) *
                                  static_cast<std::size_t>(parameters.getBackBufferHeightProperty());
        screenshot_.resize(count);
        device.GetBackBufferData(screenshot_.data(), static_cast<int>(count));
        screenshotFrame_ = framesDrawn_;
    }

    void DemoGame::SaveScreenshot()
    {
        if (screenshotSaved_)
        {
            return;
        }
        screenshotSaved_ = true;
        GraphicsDevice& device = getGraphicsDeviceProperty();
        if (screenshot_.empty())
        {
            context_.Out() << (device.getGraphicsProfileProperty() == GraphicsProfile::Reach
                                   ? "  screenshot: not available, the demo runs under the Reach profile "
                                     "(GetBackBufferData is HiDef only)\n"
                                   : "  screenshot: no frame was drawn\n");
            return;
        }
        const auto& parameters = device.getPresentationParametersProperty();
        const int width = parameters.getBackBufferWidthProperty();
        const int height = parameters.getBackBufferHeightProperty();
        if (screenshot_.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height))
        {
            context_.Out() << "  screenshot: not saved, the back buffer changed size after the last frame\n";
            return;
        }
        // A window shows only RGB. Make the copy opaque so the PNG shows the same: XNA's SaveAsPng
        // (and CNA's, which matches it) stores fully transparent pixels as (0, 0, 0, 0), which
        // would turn e.g. white-with-alpha-0 back-buffer pixels black in the screenshot.
        for (Color& pixel : screenshot_)
        {
            pixel.setAProperty(255);
        }

        Texture2D image(device, width, height);
        image.SetData(screenshot_.data(), static_cast<int>(screenshot_.size()));
        image.SaveAsPng(context_.Options().screenshotPath);
        context_.Out() << "  screenshot: " << context_.Options().screenshotPath << " (" << width << "x"
                       << height << ", frame " << screenshotFrame_ << ")\n";
    }
}
