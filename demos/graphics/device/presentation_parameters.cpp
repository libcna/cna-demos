// SPDX-License-Identifier: MS-PL
// GraphicsDevice::PresentationParameters: the settings the back buffer was created with.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CnaDemos::Name;

namespace
{
    class PresentationParametersDemo final : public CnaDemos::DemoGame
    {
    public:
        PresentationParametersDemo()
        {
            // GraphicsDeviceManager's preferences become the device's PresentationParameters.
            Graphics().setPreferredBackBufferWidthProperty(640);
            Graphics().setPreferredBackBufferHeightProperty(360);
            Graphics().setPreferredDepthStencilFormatProperty(DepthFormat::Depth24Stencil8);
        }

    protected:
        void LoadContent() override
        {
            const PresentationParameters& pp = getGraphicsDeviceProperty().getPresentationParametersProperty();
            const Rectangle bounds = pp.getBoundsProperty();

            Context().Print("BackBufferWidth      {}", pp.getBackBufferWidthProperty());
            Context().Print("BackBufferHeight     {}", pp.getBackBufferHeightProperty());
            Context().Print("BackBufferFormat     {}", Name(pp.getBackBufferFormatProperty()));
            Context().Print("DepthStencilFormat   {}", Name(pp.getDepthStencilFormatProperty()));
            Context().Print("MultiSampleCount     {}", pp.getMultiSampleCountProperty());
            Context().Print("PresentationInterval {}", Name(pp.getPresentationIntervalProperty()));
            Context().Print("IsFullScreen         {}", pp.getIsFullScreenProperty());
            Context().Print("DisplayOrientation   {}", Name(pp.getDisplayOrientationProperty()));
            Context().Print("RenderTargetUsage    {}", Name(pp.getRenderTargetUsageProperty()));
            Context().Print("Bounds               {{X:{} Y:{} Width:{} Height:{}}}", bounds.X, bounds.Y, bounds.Width,
                            bounds.Height);

            Context().Check(pp.getBackBufferWidthProperty() == 640 && pp.getBackBufferHeightProperty() == 360,
                            "the back buffer has the preferred size");
            Context().Check(pp.getDepthStencilFormatProperty() == DepthFormat::Depth24Stencil8,
                            "the preferred depth-stencil format was used");
            Context().Check(bounds == Rectangle(0, 0, 640, 360), "Bounds is the back buffer at the origin");

            // Clone gives an independent copy -- the starting point for GraphicsDevice::Reset with
            // changed settings. Editing it does not touch the device's own parameters.
            PresentationParameters copy = pp.Clone();
            copy.setBackBufferWidthProperty(320);
            Context().Check(copy.getBackBufferHeightProperty() == 360, "the clone carries every setting over");
            Context().Check(pp.getBackBufferWidthProperty() == 640, "changing the clone leaves the device alone");
        }
    };
}

CNA_DEMOS_GAME(PresentationParametersDemo, {
    .id = "graphics.device.presentation_parameters",
    .title = "Presentation parameters",
    .description = "Reads the device's PresentationParameters (back-buffer size and formats, multisampling, "
                   "present interval, orientation, usage) and clones them.",
    .apis = {"GraphicsDevice::PresentationParameters", "PresentationParameters::BackBufferWidth",
             "PresentationParameters::BackBufferHeight", "PresentationParameters::BackBufferFormat",
             "PresentationParameters::DepthStencilFormat", "PresentationParameters::MultiSampleCount",
             "PresentationParameters::PresentationInterval", "PresentationParameters::IsFullScreen",
             "PresentationParameters::DisplayOrientation", "PresentationParameters::RenderTargetUsage",
             "PresentationParameters::Bounds", "PresentationParameters::Clone"},
    .tags = {"device", "back-buffer", "settings"},
    .frames = 3,
})
