// SPDX-License-Identifier: MS-PL
// Turning vertical sync off: SynchronizeWithVerticalRetrace and PresentationInterval.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentInterval.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class VerticalSyncDemo final : public CnaDemos::DemoGame
    {
    public:
        VerticalSyncDemo()
        {
            // Present as soon as a frame is ready instead of waiting for the display's refresh
            // (useful for benchmarks; may tear on a real display).
            Graphics().setSynchronizeWithVerticalRetraceProperty(false);
        }

    protected:
        void LoadContent() override
        {
            const PresentInterval interval =
                getGraphicsDeviceProperty().getPresentationParametersProperty().getPresentationIntervalProperty();
            Context().Print("SynchronizeWithVerticalRetrace = {}, PresentationInterval = {}",
                            Graphics().getSynchronizeWithVerticalRetraceProperty(),
                            interval == PresentInterval::Immediate ? "Immediate" : "One/Default");
            Context().Check(!Graphics().getSynchronizeWithVerticalRetraceProperty(), "the property reads back false");
            Context().Check(interval == PresentInterval::Immediate, "the device presents immediately");
        }
    };
}

CNA_DEMOS_GAME(VerticalSyncDemo, {
    .id = "game.graphics_device_manager.vertical_sync",
    .title = "Vertical sync",
    .description = "Disables vertical sync with GraphicsDeviceManager::SynchronizeWithVerticalRetrace and checks "
                   "the device's PresentationInterval.",
    .apis = {"GraphicsDeviceManager::SynchronizeWithVerticalRetrace", "PresentationParameters::PresentationInterval",
             "PresentInterval::Immediate"},
    .tags = {"vsync", "present", "device"},
    .frames = 3,
})
