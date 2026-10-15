// SPDX-License-Identifier: MS-PL
// GraphicsAdapter: the display adapters, the current display mode and the supported modes.
#include <algorithm>
#include <vector>

#include "CNA/GraphicsRendererSelection.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayModeCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/NoSuitableGraphicsDeviceException.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"

using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    void DisplayModes(CnaDemos::DemoContext& ctx)
    {
        // One adapter per display. No GraphicsDevice is needed to ask.
        const auto& adapters = GraphicsAdapter::getAdaptersProperty();
        ctx.Print("{} adapter(s)", adapters.size());
        ctx.Check(!adapters.empty(), "there is at least one adapter");

        GraphicsAdapter& adapter = GraphicsAdapter::getDefaultAdapterProperty();
        ctx.Print("DefaultAdapter: \"{}\" ({}), IsWideScreen {}", adapter.getDescriptionProperty(),
                  adapter.getDeviceNameProperty(), adapter.getIsWideScreenProperty());
        ctx.Check(adapter.getIsDefaultAdapterProperty(), "DefaultAdapter reports IsDefaultAdapter");
        ctx.Check(!adapters.empty() && adapters.front().get() == &adapter, "DefaultAdapter is Adapters[0]");

        const int vendorId = adapter.getVendorIdProperty();
        const int deviceId = adapter.getDeviceIdProperty();
        ctx.Print("PCI ids: vendor 0x{:04X}, device 0x{:04X}; revision {}, subsystem {}",
                  vendorId, deviceId, adapter.getRevisionProperty(), adapter.getSubSystemIdProperty());
        ctx.Check(vendorId >= 0 && vendorId <= 0xFFFF && deviceId >= 0 && deviceId <= 0xFFFF &&
                      ((vendorId == 0) == (deviceId == 0)),
                  "PCI vendor/device ids are a bounded pair or the documented unavailable zero pair");
        ctx.Check(adapter.getRevisionProperty() == 0 && adapter.getSubSystemIdProperty() == 0,
                  "unavailable revision and subsystem ids use their documented zero values");

        std::vector<GraphicsAdapter::IntPtr> monitorHandles;
        bool monitorIdentitiesAreCoherent = adapter.getMonitorHandleProperty() ==
            adapters.front()->getMonitorHandleProperty();
        for (const auto& candidate : adapters)
        {
            const auto handle = candidate->getMonitorHandleProperty();
            if (handle != 0 && std::ranges::find(monitorHandles, handle) != monitorHandles.end())
                monitorIdentitiesAreCoherent = false;
            monitorHandles.push_back(handle);
        }
        ctx.Print("default opaque monitor handle {}", adapter.getMonitorHandleProperty());
        ctx.Check(monitorIdentitiesAreCoherent,
                  "DefaultAdapter has Adapters[0]'s monitor identity and nonzero identities are unique");

        const bool originalNull = GraphicsAdapter::getUseNullDeviceProperty();
        const bool originalReference = GraphicsAdapter::getUseReferenceDeviceProperty();
        GraphicsAdapter::setUseNullDeviceProperty(false);
        GraphicsAdapter::setUseReferenceDeviceProperty(false);
        ctx.Check(!GraphicsAdapter::getUseNullDeviceProperty() &&
                      !GraphicsAdapter::getUseReferenceDeviceProperty() &&
                      GraphicsAdapter::GetCurrentDeviceTypeEXT() == GraphicsDeviceTypeEXT::Hardware &&
                      !GraphicsAdapter::GetRequiredRendererEXT().has_value(),
                  "cleared device-selection flags retain the active renderer");
        GraphicsAdapter::setUseReferenceDeviceProperty(true);
        ctx.Check(GraphicsAdapter::getUseReferenceDeviceProperty() &&
                      GraphicsAdapter::GetCurrentDeviceTypeEXT() == GraphicsDeviceTypeEXT::Reference &&
                      GraphicsAdapter::GetRequiredRendererEXT() == CNA::GraphicsRendererType::Software,
                  "UseReferenceDevice selects CNA's software renderer intent");
        GraphicsAdapter::setUseNullDeviceProperty(true);
        ctx.Check(GraphicsAdapter::getUseNullDeviceProperty() &&
                      GraphicsAdapter::getUseReferenceDeviceProperty() &&
                      GraphicsAdapter::GetCurrentDeviceTypeEXT() == GraphicsDeviceTypeEXT::Null &&
                      GraphicsAdapter::GetRequiredRendererEXT() == CNA::GraphicsRendererType::Headless,
                  "UseNullDevice wins over the simultaneously retained reference flag");
        GraphicsAdapter::setUseNullDeviceProperty(originalNull);
        GraphicsAdapter::setUseReferenceDeviceProperty(originalReference);
        ctx.Check(GraphicsAdapter::getUseNullDeviceProperty() == originalNull &&
                      GraphicsAdapter::getUseReferenceDeviceProperty() == originalReference,
                  "the process-wide device-selection flags are restored");

        // With SOFTWARE selected we can create a displayless device to latch that renderer, then
        // prove that changing the process-wide intent to HEADLESS is refused rather than ignored.
        // A direct GL-selected console run has no platform window, so its real mismatch remains a
        // renderer/platform case instead of manufacturing a surface solely for this control.
        if (CNA::GraphicsRendererSelection::GetSelected() == CNA::GraphicsRendererType::Software)
        {
            GraphicsAdapter::setUseNullDeviceProperty(false);
            GraphicsAdapter::setUseReferenceDeviceProperty(false);
            GraphicsDevice latchedDevice;
            GraphicsAdapter::setUseNullDeviceProperty(true);
            bool mismatchedDeviceRefused = false;
            try
            {
                GraphicsDevice mismatchedDevice;
            }
            catch (const NoSuitableGraphicsDeviceException&)
            {
                mismatchedDeviceRefused = true;
            }
            GraphicsAdapter::setUseNullDeviceProperty(originalNull);
            GraphicsAdapter::setUseReferenceDeviceProperty(originalReference);
            ctx.Check(mismatchedDeviceRefused,
                      "a latched SOFTWARE device refuses a later HEADLESS device-selection request");
        }

        // What the desktop runs at right now: a windowed game usually picks a smaller size,
        // a full-screen game often matches this.
        const DisplayMode current = adapter.getCurrentDisplayModeProperty();
        ctx.Print("CurrentDisplayMode: {} (format {}, aspect ratio {:.3f})", current.ToString(),
                  CnaDemos::Name(current.getFormatProperty()), current.getAspectRatioProperty());
        ctx.CheckNear(current.getAspectRatioProperty(),
                      static_cast<double>(current.getWidthProperty()) / current.getHeightProperty(), 1e-5,
                      "AspectRatio is Width / Height");

        const DisplayMode authored(1920, 1080, SurfaceFormat::Bgr565);
        ctx.Check(authored.getTitleSafeAreaProperty() == Microsoft::Xna::Framework::Rectangle(0, 0, 1920, 1080),
                  "TitleSafeArea covers the complete authored display mode");
        ctx.Check(authored.ToString() == "{Width:1920 Height:1080 Format:Bgr565 AspectRatio:1.777778}",
                  "ToString reports dimensions, named format and derived aspect ratio in XNA order");

        // Every mode the display offers; indexing by SurfaceFormat keeps one format only.
        const DisplayModeCollection& modes = adapter.getSupportedDisplayModesProperty();
        ctx.Print("SupportedDisplayModes: {} mode(s)", modes.getCountProperty());
        int shown = 0;
        for (const DisplayMode& mode : modes)
        {
            if (shown++ == 8)
            {
                ctx.Print("  ...");
                break;
            }
            ctx.Print("  {}", mode.ToString());
        }
        const auto colorModes = modes[SurfaceFormat::Color];
        ctx.Check(std::ranges::all_of(colorModes, [](const DisplayMode& mode) {
                      return mode.getFormatProperty() == SurfaceFormat::Color;
                  }),
                  "SupportedDisplayModes[SurfaceFormat::Color] holds only Color modes");
    }
}

CNA_DEMOS_CONSOLE(DisplayModes, {
    .id = "graphics.graphics_adapter.display_modes",
    .title = "Adapters and display modes",
    .description = "Lists GraphicsAdapter::Adapters, the default adapter's description, its CurrentDisplayMode and "
                   "SupportedDisplayModes, filtered by surface format.",
    .apis = {"GraphicsAdapter::Adapters", "GraphicsAdapter::DefaultAdapter", "GraphicsAdapter::Description",
             "GraphicsAdapter::DeviceName", "GraphicsAdapter::IsDefaultAdapter", "GraphicsAdapter::IsWideScreen",
             "GraphicsAdapter::DeviceId", "GraphicsAdapter::VendorId", "GraphicsAdapter::Revision",
             "GraphicsAdapter::SubSystemId", "GraphicsAdapter::MonitorHandle",
             "GraphicsAdapter::UseNullDevice", "GraphicsAdapter::UseReferenceDevice",
             "GraphicsAdapter::GetCurrentDeviceTypeEXT", "GraphicsAdapter::GetRequiredRendererEXT",
             "NoSuitableGraphicsDeviceException::NoSuitableGraphicsDeviceException",
             "GraphicsAdapter::CurrentDisplayMode", "GraphicsAdapter::SupportedDisplayModes", "DisplayMode::Width",
             "DisplayMode::Height", "DisplayMode::Format", "DisplayMode::AspectRatio",
             "DisplayMode::TitleSafeArea", "DisplayMode::ToString",
             "DisplayModeCollection::Item"},
    .tags = {"adapter", "display", "resolution", "monitor"},
    .restrictions = "Without a display (CI, SSH) CNA reports one fallback 800x480 mode.",
})
