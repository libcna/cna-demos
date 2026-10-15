// SPDX-License-Identifier: MS-PL
// CNAEXT GraphicsDevice::SupportsCapability: ask the active renderer what it can do before using it.
#include <exception>
#include <string>
#include <string_view>
#include <vector>

#include "CNA/GraphicsCapability.hpp"
#include "CNA/LogCategory.hpp"
#include "CNA/LogLevel.hpp"
#include "CNA/Logger.hpp"
#include "CNA/Unsupported3DGraphicsCallBehavior.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture3D.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"

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

            CheckUnsupported3DPolicy();
        }

    private:
        void CheckUnsupported3DPolicy()
        {
            auto& device = getGraphicsDeviceProperty();
            auto& ctx = Context();
            using Behavior = CNA::Unsupported3DGraphicsCallBehavior;

            const Behavior original = device.GetUnsupported3DGraphicsCallBehavior();
            const bool supports3D = device.SupportsCapability(CNA::GraphicsCapability::ThreeD);
            ctx.Check(original == Behavior::Throw,
                      "unsupported 3D calls use the failure-preserving Throw policy by default");

            device.SetUnsupported3DGraphicsCallBehavior(Behavior::WarnAndStub);
            ctx.Check(device.GetUnsupported3DGraphicsCallBehavior() == Behavior::WarnAndStub &&
                          device.SupportsCapability(CNA::GraphicsCapability::ThreeD) == supports3D,
                      "WarnAndStub takes effect without changing the renderer's capability answer");

            bool invalidSizeRefused = false;
            try
            {
                Texture3D invalid(device, 0, 1, 1, false, SurfaceFormat::Color);
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                invalidSizeRefused = true;
            }
            ctx.Check(invalidSizeRefused,
                      "WarnAndStub does not suppress an invalid Texture3D dimension");

            if (!supports3D)
            {
                device.SetUnsupported3DGraphicsCallBehavior(Behavior::Throw);
                bool throwPolicyRefused = false;
                try
                {
                    Texture3D refused(device, 1, 1, 1, false, SurfaceFormat::Color);
                }
                catch (const System::NotSupportedException&)
                {
                    throwPolicyRefused = true;
                }
                ctx.Check(throwPolicyRefused,
                          "Throw refuses a volume resource on a renderer without 3D support");

                struct LogEntry
                {
                    CNA::LogLevel level;
                    CNA::LogCategory category;
                    std::string text;
                };
                std::vector<LogEntry> warnings;
                const CNA::LogLevel previousLevel = CNA::Logger::GetMinimumLevel();
                CNA::Logger::SetMinimumLevel(CNA::LogLevel::WARN);
                CNA::Logger::SetSink(
                    [&warnings](CNA::LogLevel level, CNA::LogCategory category,
                                std::string_view text) {
                        warnings.push_back({level, category, std::string(text)});
                    });

                device.SetUnsupported3DGraphicsCallBehavior(Behavior::WarnAndStub);
                bool safeNullObject = false;
                try
                {
                    Texture3D first(device, 1, 1, 1, false, SurfaceFormat::Color);
                    Texture3D second(device, 1, 1, 1, false, SurfaceFormat::Color);
                    const Color input = Color::Red;
                    Color output = Color::Magenta;
                    first.SetData(&input, 1);
                    first.GetData(&output, 1);
                    safeNullObject = output == Color::Transparent;
                }
                catch (const std::exception& exception)
                {
                    ctx.Print("WarnAndStub volume operation threw: {}", exception.what());
                }

                CNA::Logger::ResetSink();
                CNA::Logger::SetMinimumLevel(previousLevel);
                ctx.Check(safeNullObject,
                          "WarnAndStub supplies a usable zero-reading volume null object");

                int creationWarnings = 0;
                for (const LogEntry& warning : warnings)
                {
                    creationWarnings += warning.level == CNA::LogLevel::WARN &&
                                                warning.category == CNA::LogCategory::RENDER &&
                                                warning.text.find("CreateTexture3D") != std::string::npos
                                            ? 1
                                            : 0;
                }
                ctx.Check(creationWarnings == 1,
                          "repeated stubbed Texture3D creation logs one RENDER warning");
            }

            device.SetUnsupported3DGraphicsCallBehavior(original);
            ctx.Check(device.GetUnsupported3DGraphicsCallBehavior() == original,
                      "the unsupported-3D policy can be restored immediately");
        }
    };
}

CNA_DEMOS_GAME(SupportsCapabilityDemo, {
    .id = "graphics.device.supports_capability",
    .title = "Query renderer capabilities",
    .description = "Lists the active renderer's CNA capabilities and exercises the unsupported-3D policy: its default, "
                   "round trip, unchanged capability answer, argument validation and, on a 2D renderer, its warned "
                   "zero-reading Texture3D null object.",
    .apis = {"GraphicsDevice::SupportsCapability", "CNA::GraphicsCapability", "GraphicsDevice::GetGraphicsRendererName",
             "GraphicsDevice::SetUnsupported3DGraphicsCallBehavior",
             "GraphicsDevice::GetUnsupported3DGraphicsCallBehavior", "CNA::Unsupported3DGraphicsCallBehavior",
             "Texture3D::Texture3D", "Texture3D::SetData", "Texture3D::GetData"},
    .tags = {"capability", "renderer", "features", "fallback", "policy", "cnaext"},
    .frames = 1,
    .restrictions = "CNA-DEMOS-ISSUE-104: STUB advertises no 3D support but does not implement the WarnAndStub Texture3D null-object path.",
    .knownIssue = "CNA-DEMOS-ISSUE-104",
    .knownIssueRenderers = {"STUB"},
})
