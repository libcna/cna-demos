// SPDX-License-Identifier: MS-PL
// CNAEXT GraphicsDevice::SupportsCapability: ask the active renderer what it can do before using it.
#include "CNA/GraphicsCapability.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr CNA::GraphicsCapability AllCapabilities[] = {
        CNA::GraphicsCapability::ThreeD,
        CNA::GraphicsCapability::DepthStencilBuffer,
        CNA::GraphicsCapability::StencilBuffer,
        CNA::GraphicsCapability::MultiSampleAntiAliasing,
        CNA::GraphicsCapability::MultipleRenderTargets,
        CNA::GraphicsCapability::AnisotropicFiltering,
        CNA::GraphicsCapability::WireFrame,
        CNA::GraphicsCapability::OcclusionQuery,
        CNA::GraphicsCapability::CustomEffects,
        CNA::GraphicsCapability::CompiledEffects,
        CNA::GraphicsCapability::Texture3D,
        CNA::GraphicsCapability::MultiStreamVertexInput,
        CNA::GraphicsCapability::Instancing,
        CNA::GraphicsCapability::AdditiveBlending,
        CNA::GraphicsCapability::FloatRenderTargets,
        CNA::GraphicsCapability::HalfFloatRenderTargets,
        CNA::GraphicsCapability::HalfFloatTextureLinearFiltering,
        CNA::GraphicsCapability::ComputeShaders,
        CNA::GraphicsCapability::IndirectDraw,
    };

    class SupportsCapabilityDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            const GraphicsDevice& device = getGraphicsDeviceProperty();
            Context().Print("Renderer {} ({} profile):", device.GetGraphicsRendererName(),
                            CnaDemos::Name(device.getGraphicsProfileProperty()));

            // XNA has one fixed feature set per profile. CNA runs on renderers that differ (a
            // 2D-only renderer has no 3D at all), so a CNA program asks instead of catching the
            // exception a missing feature throws.
            int supported = 0;
            for (const CNA::GraphicsCapability capability : AllCapabilities)
            {
                const bool yes = device.SupportsCapability(capability);
                supported += yes ? 1 : 0;
                Context().Print("  {:<32} {}", CnaDemos::Name(capability), yes ? "yes" : "no");
            }
            Context().Print("{} of {} capabilities supported", supported, std::size(AllCapabilities));

            // A stencil buffer is a kind of depth-stencil buffer.
            if (device.SupportsCapability(CNA::GraphicsCapability::StencilBuffer))
                Context().Check(device.SupportsCapability(CNA::GraphicsCapability::DepthStencilBuffer),
                                "a renderer with stencil buffers also reports depth-stencil buffers");
        }
    };
}

CNA_DEMOS_GAME(SupportsCapabilityDemo, {
    .id = "graphics.device.supports_capability",
    .title = "Query renderer capabilities",
    .description = "Lists which CNA::GraphicsCapability values the active renderer supports, using the CNA extension "
                   "GraphicsDevice::SupportsCapability.",
    .apis = {"GraphicsDevice::SupportsCapability", "CNA::GraphicsCapability", "GraphicsDevice::GetGraphicsRendererName"},
    .tags = {"capability", "renderer", "features", "cnaext"},
    .frames = 1,
})
