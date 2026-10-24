// SPDX-License-Identifier: MS-PL
// Construct, reset, present and dispose a separately owned GraphicsDevice.
#include <array>
#include <cstdint>
#include <optional>
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
#include "System/ArgumentException.hpp"
#include "System/EventArgs.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    template<typename Exception, typename Action>
    bool Throws(Action&& action)
    {
        try
        {
            action();
            return false;
        }
        catch (const Exception&)
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
            initial.setIsFullScreenProperty(false);
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

            owned.Present(std::nullopt, std::nullopt, 0);
            Context().Check(owned.getGraphicsDeviceStatusProperty() == GraphicsDeviceStatus::Normal,
                            "null source, null destination and window zero use ordinary Present");

            const std::string renderer = std::string(owned.GetGraphicsRendererName());
            const bool acceptsSourceRegion = renderer == "SOFTWARE" || renderer == "HEADLESS";
            bool sourceRegionPresented = false;
            std::string sourceRefusal;
            try
            {
                // The supplied region extends beyond the top-left and is clipped to real pixels.
                owned.Present(Rectangle(-16, -8, 96, 48), std::nullopt, 0);
                sourceRegionPresented = true;
            }
            catch (const System::NotSupportedException& error)
            {
                sourceRefusal = error.what();
            }
            Context().Check(sourceRegionPresented == acceptsSourceRegion &&
                                (sourceRegionPresented ||
                                 (sourceRefusal.find(renderer) != std::string::npos &&
                                  sourceRefusal.find("source rectangle") != std::string::npos)),
                            "a clipped source region is honored only by the documented renderer contract");

            Context().Check(
                Throws<System::ArgumentException>([&owned] {
                    owned.Present(Rectangle(256, 0, 16, 16), std::nullopt, 0);
                }),
                "a source region entirely outside the back buffer is rejected before presentation");

            bool destinationAndWindowPresented = false;
            std::string destinationRefusal;
            try
            {
                owned.Present(std::nullopt, Rectangle(0, 0, 64, 36),
                              static_cast<std::uintptr_t>(0x1234));
                destinationAndWindowPresented = true;
            }
            catch (const System::NotSupportedException& error)
            {
                destinationRefusal = error.what();
            }
            Context().Check(destinationAndWindowPresented == (renderer == "HEADLESS") &&
                                (destinationAndWindowPresented ||
                                 (destinationRefusal.find("destination rectangle") != std::string::npos &&
                                  destinationRefusal.find("override window") != std::string::npos)),
                            "destination and override-window requests are honored only by HEADLESS");

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

            bool intervalCycle = true;
            for (const PresentInterval interval :
                 std::array{PresentInterval::Default, PresentInterval::One, PresentInterval::Two})
            {
                PresentationParameters changed = owned.getPresentationParametersProperty().Clone();
                changed.setPresentationIntervalProperty(interval);
                owned.Reset(changed);
                owned.Clear(Color::Black);
                owned.Present();
                intervalCycle = intervalCycle &&
                    owned.getPresentationParametersProperty().getPresentationIntervalProperty() == interval &&
                    owned.getGraphicsDeviceStatusProperty() == GraphicsDeviceStatus::Normal;
            }
            const std::vector<std::string> resetEvents{
                "resetting", "reset", "resetting", "reset",
                "resetting", "reset", "resetting", "reset"};
            Context().Check(intervalCycle && events == resetEvents && !resource.getIsDisposedProperty(),
                            "Default, One and Two survive real reset/present cycles with ordered events");

            owned.Dispose();
            auto disposedEvents = resetEvents;
            disposedEvents.emplace_back("disposing");
            Context().Check(owned.getIsDisposedProperty() && disposedDuringEvent &&
                                resource.getIsDisposedProperty() && resourceDisposingEvents == 1 &&
                                events == disposedEvents &&
                                deviceLostEvents == 0,
                            "Dispose marks the device before its event and disposes owned resources once");
            Context().Check(Throws<System::ObjectDisposedException>([&owned] { owned.Present(); }) &&
                                Throws<System::ObjectDisposedException>([&owned] { owned.Reset(); }),
                            "disposed devices reject Present and Reset");

            owned.Dispose();
            Context().Check(events == disposedEvents && resourceDisposingEvents == 1,
                            "disposing the device twice is idempotent");
            Context().Print("events: four resetting/reset pairs, disposing; DeviceLost {}", deviceLostEvents);
        }
    };
}

CNA_DEMOS_GAME(OwnedDeviceLifecycleDemo, {
    .id = "graphics.device.owned_lifecycle",
    .title = "Owned graphics-device lifecycle",
    .description = "Constructs a separate device, exercises region Present and every present interval through real resets, observes ordered lifecycle events and resource teardown, then checks disposed refusals.",
    .apis = {"GraphicsDevice::GraphicsDevice", "GraphicsDevice::Present", "GraphicsDevice::Reset",
             "GraphicsDevice::Dispose", "GraphicsDevice::Disposing", "GraphicsDevice::IsDisposed",
             "GraphicsDevice::DeviceResetting", "GraphicsDevice::DeviceReset",
             "GraphicsDevice::GraphicsDeviceStatus", "GraphicsDeviceStatus", "GraphicsDeviceStatus::Normal",
             "PresentInterval::Default", "PresentInterval::One", "PresentInterval::Two"},
    .tags = {"device", "lifecycle", "reset", "present", "region", "dispose"},
    .frames = 1,
    .restrictions = "SOFTWARE presents source regions; OPENGLES3 truthfully refuses region requests. No presented window pixels are inspected. DeviceLost is observed as absent and not claimed as exercised.",
})
