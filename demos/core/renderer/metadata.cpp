// SPDX-License-Identifier: MS-PL
// Renderer identities and their stable launcher-facing classification metadata.
#include "CNA/GraphicsBackendCategory.hpp"
#include "CNA/GraphicsBackendMaturity.hpp"
#include "CNA/GraphicsRendererType.hpp"
#include "CnaDemos/Demo.hpp"

#include <array>
#include <string_view>

namespace
{
    struct RendererMetadata
    {
        CNA::GraphicsRendererType type;
        std::string_view name;
        CNA::GraphicsBackendCategory category;
        CNA::GraphicsBackendMaturity maturity;
    };

    struct NamedCategory
    {
        CNA::GraphicsBackendCategory value;
        std::string_view name;
    };

    struct NamedMaturity
    {
        CNA::GraphicsBackendMaturity value;
        std::string_view name;
    };

    constexpr std::array Renderers{
        RendererMetadata{CNA::GraphicsRendererType::OpenGLES3, "OPENGLES3",
                         CNA::GraphicsBackendCategory::Native, CNA::GraphicsBackendMaturity::Production},
        RendererMetadata{CNA::GraphicsRendererType::OpenGL33, "OPENGL33",
                         CNA::GraphicsBackendCategory::Native, CNA::GraphicsBackendMaturity::Production},
        RendererMetadata{CNA::GraphicsRendererType::WebGL2, "WEBGL2",
                         CNA::GraphicsBackendCategory::Web, CNA::GraphicsBackendMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::WebGPU, "WEBGPU",
                         CNA::GraphicsBackendCategory::TranslationLayer, CNA::GraphicsBackendMaturity::Experimental},
        RendererMetadata{CNA::GraphicsRendererType::Headless, "HEADLESS",
                         CNA::GraphicsBackendCategory::Diagnostic, CNA::GraphicsBackendMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::Software, "SOFTWARE",
                         CNA::GraphicsBackendCategory::Software, CNA::GraphicsBackendMaturity::Experimental},
        RendererMetadata{CNA::GraphicsRendererType::Stub, "STUB",
                         CNA::GraphicsBackendCategory::Diagnostic, CNA::GraphicsBackendMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::DirectX9, "DIRECTX9",
                         CNA::GraphicsBackendCategory::Native, CNA::GraphicsBackendMaturity::Production},
        RendererMetadata{CNA::GraphicsRendererType::SdlGpu, "SDL_GPU",
                         CNA::GraphicsBackendCategory::TranslationLayer, CNA::GraphicsBackendMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::Fna3d, "FNA3D",
                         CNA::GraphicsBackendCategory::TranslationLayer, CNA::GraphicsBackendMaturity::Experimental},
    };

    constexpr std::array Categories{
        NamedCategory{CNA::GraphicsBackendCategory::Native, "Native"},
        NamedCategory{CNA::GraphicsBackendCategory::TranslationLayer, "TranslationLayer"},
        NamedCategory{CNA::GraphicsBackendCategory::Software, "Software"},
        NamedCategory{CNA::GraphicsBackendCategory::Web, "Web"},
        NamedCategory{CNA::GraphicsBackendCategory::Diagnostic, "Diagnostic"},
    };

    constexpr std::array Maturities{
        NamedMaturity{CNA::GraphicsBackendMaturity::Production, "Production"},
        NamedMaturity{CNA::GraphicsBackendMaturity::Supported, "Supported"},
        NamedMaturity{CNA::GraphicsBackendMaturity::Experimental, "Experimental"},
        NamedMaturity{CNA::GraphicsBackendMaturity::Historical, "Historical"},
        NamedMaturity{CNA::GraphicsBackendMaturity::Deprecated, "Deprecated"},
    };

    void Metadata(CnaDemos::DemoContext& ctx)
    {
        bool namesMatch = true;
        bool classificationsMatch = true;
        bool roundTripsMatch = true;
        for (const auto& renderer : Renderers)
        {
            namesMatch &= CNA::getGraphicsRendererName(renderer.type) == renderer.name;
            classificationsMatch &= CNA::getGraphicsBackendCategory(renderer.type) == renderer.category &&
                                    CNA::getGraphicsBackendMaturity(renderer.type) == renderer.maturity;

            CNA::GraphicsRendererType parsed = CNA::GraphicsRendererType::Stub;
            roundTripsMatch &= CNA::tryParseGraphicsRendererName(renderer.name, parsed) && parsed == renderer.type;
        }

        ctx.Check(namesMatch, "all public renderer identities have their canonical build-option names");
        ctx.Check(classificationsMatch,
                  "all renderer identities expose their implementation category and support maturity");
        ctx.Check(roundTripsMatch, "every canonical renderer name parses back to its identity");

        CNA::GraphicsRendererType mixedCase = CNA::GraphicsRendererType::Stub;
        ctx.Check(CNA::tryParseGraphicsRendererName("sDl_GpU", mixedCase) &&
                      mixedCase == CNA::GraphicsRendererType::SdlGpu,
                  "renderer names parse case-insensitively");

        CNA::GraphicsRendererType preserved = CNA::GraphicsRendererType::Headless;
        ctx.Check(!CNA::tryParseGraphicsRendererName("VULKAN", preserved) &&
                      preserved == CNA::GraphicsRendererType::Headless,
                  "an unknown renderer name leaves the output identity untouched");

        const auto unknownRenderer = static_cast<CNA::GraphicsRendererType>(999);
        ctx.Check(CNA::getGraphicsRendererName(unknownRenderer) == "UNKNOWN",
                  "an unknown renderer identity has the documented fallback name");

        bool valueNamesMatch = true;
        for (const auto& category : Categories)
            valueNamesMatch &= CNA::toStringView(category.value) == category.name;
        for (const auto& maturity : Maturities)
            valueNamesMatch &= CNA::toStringView(maturity.value) == maturity.name;
        ctx.Check(valueNamesMatch, "all category and maturity values have stable human-readable names");
        ctx.Check(CNA::toStringView(static_cast<CNA::GraphicsBackendCategory>(999)) == "Unknown" &&
                      CNA::toStringView(static_cast<CNA::GraphicsBackendMaturity>(999)) == "Unknown",
                  "unknown category and maturity values have the documented fallback name");

        const auto current = CNA::getCurrentGraphicsRendererType();
        ctx.Check(CNA::getCurrentGraphicsRendererName() == CNA::getGraphicsRendererName(current) &&
                      CNA::getCurrentGraphicsBackendCategory() == CNA::getGraphicsBackendCategory(current) &&
                      CNA::getCurrentGraphicsBackendMaturity() == CNA::getGraphicsBackendMaturity(current),
                  "the current-build helpers agree with the general metadata queries");

        ctx.Print("Current renderer: {} ({}, {})", CNA::getCurrentGraphicsRendererName(),
                  CNA::toStringView(CNA::getCurrentGraphicsBackendCategory()),
                  CNA::toStringView(CNA::getCurrentGraphicsBackendMaturity()));
    }
}

CNA_DEMOS_CONSOLE(Metadata, {
    .id = "core.renderer.metadata",
    .title = "Renderer metadata",
    .description = "Enumerates every public CNA renderer identity, maps it to the canonical build-option name, "
                   "implementation category and maturity, parses names, and reports the current build.",
    .apis = {"CNA::GraphicsRendererType", "CNA::getGraphicsRendererName", "CNA::tryParseGraphicsRendererName",
             "CNA::getCurrentGraphicsRendererType", "CNA::getCurrentGraphicsRendererName",
             "CNA::GraphicsBackendCategory", "CNA::getGraphicsBackendCategory",
             "CNA::getCurrentGraphicsBackendCategory", "CNA::GraphicsBackendMaturity",
             "CNA::getGraphicsBackendMaturity", "CNA::getCurrentGraphicsBackendMaturity", "CNA::toStringView"},
    .tags = {"renderer", "metadata", "launcher"},
})
