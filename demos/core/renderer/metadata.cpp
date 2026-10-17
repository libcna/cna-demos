// SPDX-License-Identifier: MS-PL
// Renderer identities and their stable launcher-facing classification metadata.
#include "CNA/GraphicsRendererCategory.hpp"
#include "CNA/GraphicsRendererMaturity.hpp"
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
        CNA::GraphicsRendererCategory category;
        CNA::GraphicsRendererMaturity maturity;
    };

    struct NamedCategory
    {
        CNA::GraphicsRendererCategory value;
        std::string_view name;
    };

    struct NamedMaturity
    {
        CNA::GraphicsRendererMaturity value;
        std::string_view name;
    };

    constexpr std::array Renderers{
        RendererMetadata{CNA::GraphicsRendererType::OpenGLES3, "OPENGLES3",
                         CNA::GraphicsRendererCategory::Native, CNA::GraphicsRendererMaturity::Production},
        RendererMetadata{CNA::GraphicsRendererType::OpenGL33, "OPENGL33",
                         CNA::GraphicsRendererCategory::Native, CNA::GraphicsRendererMaturity::Production},
        RendererMetadata{CNA::GraphicsRendererType::WebGL2, "WEBGL2",
                         CNA::GraphicsRendererCategory::Web, CNA::GraphicsRendererMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::WebGPU, "WEBGPU",
                         CNA::GraphicsRendererCategory::TranslationLayer, CNA::GraphicsRendererMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::Headless, "HEADLESS",
                         CNA::GraphicsRendererCategory::Diagnostic, CNA::GraphicsRendererMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::Software, "SOFTWARE",
                         CNA::GraphicsRendererCategory::Software, CNA::GraphicsRendererMaturity::Experimental},
        RendererMetadata{CNA::GraphicsRendererType::Stub, "STUB",
                         CNA::GraphicsRendererCategory::Diagnostic, CNA::GraphicsRendererMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::DirectX9, "DIRECTX9",
                         CNA::GraphicsRendererCategory::Native, CNA::GraphicsRendererMaturity::Production},
        RendererMetadata{CNA::GraphicsRendererType::SdlGpu, "SDL_GPU",
                         CNA::GraphicsRendererCategory::TranslationLayer, CNA::GraphicsRendererMaturity::Supported},
        RendererMetadata{CNA::GraphicsRendererType::Fna3d, "FNA3D",
                         CNA::GraphicsRendererCategory::TranslationLayer, CNA::GraphicsRendererMaturity::Experimental},
    };

    constexpr std::array Categories{
        NamedCategory{CNA::GraphicsRendererCategory::Native, "Native"},
        NamedCategory{CNA::GraphicsRendererCategory::TranslationLayer, "TranslationLayer"},
        NamedCategory{CNA::GraphicsRendererCategory::Software, "Software"},
        NamedCategory{CNA::GraphicsRendererCategory::Web, "Web"},
        NamedCategory{CNA::GraphicsRendererCategory::Diagnostic, "Diagnostic"},
    };

    constexpr std::array Maturities{
        NamedMaturity{CNA::GraphicsRendererMaturity::Production, "Production"},
        NamedMaturity{CNA::GraphicsRendererMaturity::Supported, "Supported"},
        NamedMaturity{CNA::GraphicsRendererMaturity::Experimental, "Experimental"},
        NamedMaturity{CNA::GraphicsRendererMaturity::Historical, "Historical"},
        NamedMaturity{CNA::GraphicsRendererMaturity::Deprecated, "Deprecated"},
    };

    void Metadata(CnaDemos::DemoContext& ctx)
    {
        bool namesMatch = true;
        bool classificationsMatch = true;
        bool roundTripsMatch = true;
        for (const auto& renderer : Renderers)
        {
            namesMatch &= CNA::getGraphicsRendererName(renderer.type) == renderer.name;
            classificationsMatch &= CNA::getGraphicsRendererCategory(renderer.type) == renderer.category &&
                                    CNA::getGraphicsRendererMaturity(renderer.type) == renderer.maturity;

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
        ctx.Check(CNA::toStringView(static_cast<CNA::GraphicsRendererCategory>(999)) == "Unknown" &&
                      CNA::toStringView(static_cast<CNA::GraphicsRendererMaturity>(999)) == "Unknown",
                  "unknown category and maturity values have the documented fallback name");

        const auto current = CNA::getCurrentGraphicsRendererType();
        ctx.Check(CNA::getCurrentGraphicsRendererName() == CNA::getGraphicsRendererName(current) &&
                      CNA::getCurrentGraphicsRendererCategory() == CNA::getGraphicsRendererCategory(current) &&
                      CNA::getCurrentGraphicsRendererMaturity() == CNA::getGraphicsRendererMaturity(current),
                  "the current-build helpers agree with the general metadata queries");

        ctx.Print("Current renderer: {} ({}, {})", CNA::getCurrentGraphicsRendererName(),
                  CNA::toStringView(CNA::getCurrentGraphicsRendererCategory()),
                  CNA::toStringView(CNA::getCurrentGraphicsRendererMaturity()));
    }
}

CNA_DEMOS_CONSOLE(Metadata, {
    .id = "core.renderer.metadata",
    .title = "Renderer metadata",
    .description = "Enumerates every public CNA renderer identity, maps it to the canonical build-option name, "
                   "implementation category and maturity, parses names, and reports the current build.",
    .apis = {"CNA::GraphicsRendererType", "CNA::getGraphicsRendererName", "CNA::tryParseGraphicsRendererName",
             "CNA::getCurrentGraphicsRendererType", "CNA::getCurrentGraphicsRendererName",
             "CNA::GraphicsRendererCategory", "CNA::getGraphicsRendererCategory",
             "CNA::getCurrentGraphicsRendererCategory", "CNA::GraphicsRendererMaturity",
             "CNA::getGraphicsRendererMaturity", "CNA::getCurrentGraphicsRendererMaturity", "CNA::toStringView"},
    .tags = {"renderer", "metadata", "launcher"},
})
