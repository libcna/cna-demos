// SPDX-License-Identifier: MS-PL
// GraphicsAdapter::IsProfileSupported and QueryBackBufferFormat/QueryRenderTargetFormat.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"

using namespace Microsoft::Xna::Framework::Graphics;
using CnaDemos::Name;

namespace
{
    void FormatQueries(CnaDemos::DemoContext& ctx)
    {
        const GraphicsAdapter& adapter = GraphicsAdapter::getDefaultAdapterProperty();

        const bool reach = adapter.IsProfileSupported(GraphicsProfile::Reach);
        const bool hiDef = adapter.IsProfileSupported(GraphicsProfile::HiDef);
        ctx.Print("IsProfileSupported: Reach {}, HiDef {}", reach, hiDef);
        ctx.Check(reach, "every adapter supports the Reach profile");

        // The Query methods never fail a request outright: they answer whether the exact request
        // is supported, and always say what the adapter would pick instead.
        const auto query = [&](const char* what, auto method, GraphicsProfile profile, SurfaceFormat format,
                               DepthFormat depth, int samples) {
            SurfaceFormat selectedFormat{};
            DepthFormat selectedDepth{};
            int selectedSamples = 0;
            const bool exact = (adapter.*method)(profile, format, depth, samples, selectedFormat, selectedDepth,
                                                 selectedSamples);
            ctx.Print("{} {} {} {}x {} -> {} {} {}x ({})", what, Name(profile), Name(format), samples, Name(depth),
                      Name(selectedFormat), Name(selectedDepth), selectedSamples, exact ? "exact" : "substituted");
            return exact;
        };

        const bool color = query("QueryBackBufferFormat  ", &GraphicsAdapter::QueryBackBufferFormat,
                                 GraphicsProfile::HiDef, SurfaceFormat::Color, DepthFormat::Depth24Stencil8, 0);
        ctx.Check(color, "a Color back buffer with Depth24Stencil8 is supported as requested");

        query("QueryRenderTargetFormat", &GraphicsAdapter::QueryRenderTargetFormat, GraphicsProfile::HiDef,
              SurfaceFormat::HdrBlendable, DepthFormat::None, 0);
        query("QueryRenderTargetFormat", &GraphicsAdapter::QueryRenderTargetFormat, GraphicsProfile::Reach,
              SurfaceFormat::Vector4, DepthFormat::Depth24, 0);
    }
}

CNA_DEMOS_CONSOLE(FormatQueries, {
    .id = "graphics.graphics_adapter.format_queries",
    .title = "Profile and format queries",
    .description = "Asks the default GraphicsAdapter which profiles it supports and which back-buffer and "
                   "render-target formats it would select for a request.",
    .apis = {"GraphicsAdapter::IsProfileSupported", "GraphicsAdapter::QueryBackBufferFormat",
             "GraphicsAdapter::QueryRenderTargetFormat"},
    .tags = {"adapter", "format", "render-target", "profile"},
})
