// SPDX-License-Identifier: MS-PL
// CNAEXT renderer capability profile: limits such as the largest texture, and the full report.
#include <string>
#include <string_view>

#include "CNA/RendererCapabilityProfile.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class CapabilityReportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            const GraphicsDevice& device = getGraphicsDeviceProperty();

            // One numeric limit. "known" separates a real answer from "this renderer did not say".
            const CNA::RendererLimitValue maxTexture =
                device.GetRendererLimitEXT(CNA::RendererLimit::MaxTextureDimension);
            Context().Print("MaxTextureDimension: {}", maxTexture.known ? std::to_string(maxTexture.value) : "unknown");

            // The shortcut for the limit every texture loader needs before creating a texture of a
            // size read from a file.
            const int maxDimension = device.GetMaxTextureDimension();
            Context().Print("GetMaxTextureDimension() = {}", maxDimension);
            Context().Check(maxDimension > 0, "the renderer reports a positive maximum texture size");
            if (maxTexture.known)
                Context().Check(maxTexture.value == static_cast<std::uint64_t>(maxDimension),
                                "the profile's limit and GetMaxTextureDimension agree");

            // Everything at once, as English text: features, limits, per-format support.
            const std::string_view report = device.GetRendererCapabilityReportEXT();
            Context().Check(!report.empty(), "the renderer produces a capability report");
            Context().Print("--- capability report of {} ---", device.GetGraphicsRendererName());
            Context().Print("{}", report);
        }
    };
}

CNA_DEMOS_GAME(CapabilityReportDemo, {
    .id = "graphics.device.capability_report",
    .title = "Renderer capability report",
    .description = "Reads a renderer limit with GetRendererLimitEXT and GetMaxTextureDimension, then prints the full "
                   "GetRendererCapabilityReportEXT text of the active renderer (CNA extensions).",
    .apis = {"GraphicsDevice::GetRendererLimitEXT", "GraphicsDevice::GetMaxTextureDimension",
             "GraphicsDevice::GetRendererCapabilityReportEXT", "CNA::RendererLimit"},
    .tags = {"capability", "renderer", "limits", "cnaext"},
    .frames = 1,
})
