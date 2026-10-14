// SPDX-License-Identifier: MS-PL
// GraphicsAdapter: the display adapters, the current display mode and the supported modes.
#include <algorithm>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayModeCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
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
             "GraphicsAdapter::CurrentDisplayMode", "GraphicsAdapter::SupportedDisplayModes", "DisplayMode::Width",
             "DisplayMode::Height", "DisplayMode::Format", "DisplayMode::AspectRatio",
             "DisplayMode::TitleSafeArea", "DisplayMode::ToString",
             "DisplayModeCollection::Item"},
    .tags = {"adapter", "display", "resolution", "monitor"},
    .restrictions = "Without a display (CI, SSH) CNA reports one fallback 800x480 mode.",
})
