// SPDX-License-Identifier: MS-PL
// Construct, reset, present and dispose a separately owned GraphicsDevice.
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceStatus.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentInterval.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/EventArgs.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    template<typename Action>
    bool ThrowsDisposed(Action&& action)
    {
        try
        {
            action();
            return false;
        }
        catch (const System::ObjectDisposedException&)
        {
            return true;
        }
    }

    class OwnedDeviceLifecycleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            PresentationParameters initial;
            initial.setBackBufferWidthProperty(256);
            initial.setBackBufferHeightProperty(144);
            initial.setDepthStencilFormatProperty(DepthFormat::None);
            initial.setPresentationIntervalProperty(PresentInterval::Immediate);

            GraphicsDevice owned(GraphicsAdapter::getDefaultAdapterProperty(), GraphicsProfile::HiDef, initial);
            Context().Check(!owned.getIsDisposedProperty() &&
                                owned.getGraphicsDeviceStatusProperty() == GraphicsDeviceStatus::Normal &&
                                owned.getPresentationParametersProperty().getBackBufferWidthProperty() == 256 &&
                                owned.getPresentationParametersProperty().getBackBufferHeightProperty() == 144,
                            "the separately constructed device is live with its requested presentation size");

            std::vector<std::string> events;
            int deviceLostEvents = 0;
            bool disposedDuringEvent = false;
            owned.DeviceResetting += [&events](System::Object*, const System::EventArgs&) {
                events.emplace_back("resetting");
            };
            owned.DeviceReset += [&events](System::Object*, const System::EventArgs&) {
                events.emplace_back("reset");
            };
            owned.Disposing += [&events, &owned, &disposedDuringEvent](System::Object* sender,
                                                                      const System::EventArgs&) {
                events.emplace_back("disposing");
                disposedDuringEvent = sender == &owned && owned.getIsDisposedProperty();
            };
            owned.DeviceLost += [&deviceLostEvents](System::Object*, const System::EventArgs&) {
                ++deviceLostEvents;
            };

            Texture2D resource(owned, 2, 2);
            int resourceDisposingEvents = 0;
            resource.Disposing += [&resourceDisposingEvents](System::Object*, const System::EventArgs&) {
                ++resourceDisposingEvents;
            };

            owned.Clear(Color::CornflowerBlue);
            owned.Present();
            Context().Check(owned.getGraphicsDeviceStatusProperty() == GraphicsDeviceStatus::Normal,
                            "Present completes while the owned device remains Normal");

            PresentationParameters resized = owned.getPresentationParametersProperty().Clone();
            resized.setBackBufferWidthProperty(320);
            resized.setBackBufferHeightProperty(180);
            owned.Reset(resized);
            Context().Check(events == std::vector<std::string>({"resetting", "reset"}) &&
                                owned.getPresentationParametersProperty().getBackBufferWidthProperty() == 320 &&
                                owned.getPresentationParametersProperty().getBackBufferHeightProperty() == 180 &&
                                owned.getViewportProperty().getWidthProperty() == 320 &&
                                owned.getViewportProperty().getHeightProperty() == 180 &&
                                !resource.getIsDisposedProperty(),
                            "Reset raises ordered events, applies 320x180 and retains the owned texture");

            owned.Clear(Color::DarkSlateBlue);
            owned.Present();
            Context().Check(owned.getGraphicsDeviceStatusProperty() == GraphicsDeviceStatus::Normal,
                            "the reset device presents again with Normal status");

            owned.Dispose();
            Context().Check(owned.getIsDisposedProperty() && disposedDuringEvent &&
                                resource.getIsDisposedProperty() && resourceDisposingEvents == 1 &&
                                events == std::vector<std::string>({"resetting", "reset", "disposing"}) &&
                                deviceLostEvents == 0,
                            "Dispose marks the device before its event and disposes owned resources once");
            Context().Check(ThrowsDisposed([&owned] { owned.Present(); }) &&
                                ThrowsDisposed([&owned] { owned.Reset(); }),
                            "disposed devices reject Present and Reset");

            owned.Dispose();
            Context().Check(events.size() == 3 && resourceDisposingEvents == 1,
                            "disposing the device twice is idempotent");
            Context().Print("events: resetting, reset, disposing; DeviceLost {}", deviceLostEvents);
        }
    };
}

CNA_DEMOS_GAME(OwnedDeviceLifecycleDemo, {
    .id = "graphics.device.owned_lifecycle",
    .title = "Owned graphics-device lifecycle",
    .description = "Constructs a separate device, presents before and after a reset, observes ordered reset/disposal events and owned-resource teardown, then checks disposed refusals.",
    .apis = {"GraphicsDevice::GraphicsDevice", "GraphicsDevice::Present", "GraphicsDevice::Reset",
             "GraphicsDevice::Dispose", "GraphicsDevice::Disposing", "GraphicsDevice::IsDisposed",
             "GraphicsDevice::DeviceResetting", "GraphicsDevice::DeviceReset",
             "GraphicsDevice::GraphicsDeviceStatus", "GraphicsDeviceStatus"},
    .tags = {"device", "lifecycle", "reset", "present", "dispose"},
    .frames = 1,
    .restrictions = "Desktop reset/present lifecycle; DeviceLost is observed as absent and not claimed as exercised.",
})
