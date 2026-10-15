// SPDX-License-Identifier: MS-PL
// CNAEXT renderer capability profile: structured features, limits, formats and display output.
#include <array>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>

#include "CNA/DisplayColorSpace.hpp"
#include "CNA/RendererCapabilityProfile.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr std::array SurfaceFormats{
        SurfaceFormat::Color, SurfaceFormat::Bgr565, SurfaceFormat::Bgra5551,
        SurfaceFormat::Bgra4444, SurfaceFormat::Dxt1, SurfaceFormat::Dxt3,
        SurfaceFormat::Dxt5, SurfaceFormat::NormalizedByte2, SurfaceFormat::NormalizedByte4,
        SurfaceFormat::Rgba1010102, SurfaceFormat::Rg32, SurfaceFormat::Rgba64,
        SurfaceFormat::Alpha8, SurfaceFormat::Single, SurfaceFormat::Vector2,
        SurfaceFormat::Vector4, SurfaceFormat::HalfSingle, SurfaceFormat::HalfVector2,
        SurfaceFormat::HalfVector4, SurfaceFormat::HdrBlendable, SurfaceFormat::ColorBgraEXT,
        SurfaceFormat::ColorSrgbEXT, SurfaceFormat::Dxt5SrgbEXT, SurfaceFormat::Bc7EXT,
        SurfaceFormat::Bc7SrgbEXT, SurfaceFormat::ByteEXT, SurfaceFormat::UShortEXT,
    };

    constexpr std::array FormatUsages{
        CNA::RendererFormatUsage::TextureStorage,
        CNA::RendererFormatUsage::Sampled,
        CNA::RendererFormatUsage::Filterable,
        CNA::RendererFormatUsage::RenderTarget,
        CNA::RendererFormatUsage::Blendable,
        CNA::RendererFormatUsage::TransferSource,
        CNA::RendererFormatUsage::TransferDestination,
        CNA::RendererFormatUsage::Mipmapped,
        CNA::RendererFormatUsage::Multisample,
        CNA::RendererFormatUsage::ColorTransfer,
    };

    constexpr std::uint32_t FormatUsageMask()
    {
        std::uint32_t mask = 0;
        for (const CNA::RendererFormatUsage usage : FormatUsages)
            mask |= static_cast<std::uint32_t>(usage);
        return mask;
    }

    class CapabilityReportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const CNA::RendererCapabilityProfile& profile = device.GetRendererCapabilityProfileEXT();

            Context().Check(&profile == &device.GetRendererCapabilityProfileEXT() &&
                                profile.GetRendererName() == device.GetGraphicsRendererName(),
                            "the device caches one detailed profile bearing the active renderer name");

            std::set<std::string_view> featureNames;
            bool featuresCoherent = !CNA::AllRendererFeatures().empty();
            for (const CNA::RendererFeature feature : CNA::AllRendererFeatures())
            {
                const CNA::RendererFeatureInfo& info = profile.GetFeature(feature);
                const bool complete = info.support == CNA::RendererFeatureSupport::Supported;
                const std::string_view name = CNA::GetRendererFeatureName(feature);
                featuresCoherent &= info.support != CNA::RendererFeatureSupport::Unknown &&
                                    device.GetRendererFeatureSupportEXT(feature) == info.support &&
                                    profile.Supports(feature) == complete &&
                                    device.SupportsRendererFeatureEXT(feature) == complete &&
                                    name != "UnknownRendererFeature" &&
                                    !CNA::GetRendererFeatureDescription(feature).empty();
                featureNames.insert(name);
            }
            Context().Check(featuresCoherent && featureNames.size() == CNA::AllRendererFeatures().size(),
                            "every defined detailed feature is uniquely named, described and classified consistently");

            std::set<std::string_view> limitNames;
            bool limitsCoherent = CNA::AllRendererLimits().size() == 8;
            for (const CNA::RendererLimit limit : CNA::AllRendererLimits())
            {
                const CNA::RendererLimitValue fromProfile = profile.GetLimit(limit);
                const CNA::RendererLimitValue fromDevice = device.GetRendererLimitEXT(limit);
                limitsCoherent &= fromProfile.known && fromDevice.known &&
                                  fromProfile.value == fromDevice.value;
                limitNames.insert(CNA::GetRendererLimitName(limit));
            }
            Context().Check(limitsCoherent && limitNames.size() == CNA::AllRendererLimits().size(),
                            "all eight live renderer limits are uniquely named, known and identical through both APIs");

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

            bool formatsCoherent = true;
            for (const SurfaceFormat format : SurfaceFormats)
            {
                const CNA::RendererFormatSupport fromProfile =
                    profile.GetSurfaceFormatSupport(static_cast<std::uint32_t>(format));
                const CNA::RendererFormatSupport fromDevice =
                    device.GetRendererSurfaceFormatSupportEXT(format);
                formatsCoherent &= fromProfile.knownUsages == fromDevice.knownUsages &&
                                   fromProfile.supportedUsages == fromDevice.supportedUsages &&
                                   (fromProfile.knownUsages & ~FormatUsageMask()) == 0 &&
                                   (fromProfile.supportedUsages & ~fromProfile.knownUsages) == 0 &&
                                   device.SupportsSurfaceFormatAsRenderTargetEXT(format) ==
                                       fromProfile.Supports(CNA::RendererFormatUsage::RenderTarget);
            }
            const CNA::RendererFormatSupport color =
                profile.GetSurfaceFormatSupport(static_cast<std::uint32_t>(SurfaceFormat::Color));
            Context().Check(formatsCoherent && color.IsKnown(CNA::RendererFormatUsage::TextureStorage) &&
                                color.IsKnown(CNA::RendererFormatUsage::RenderTarget) &&
                                color.IsKnown(CNA::RendererFormatUsage::ColorTransfer),
                            "all 27 surface formats expose bounded usage masks and agree with direct render-target queries");

            const CNA::RendererCapabilityProfile emptyProfile;
            const auto invalidFeature = static_cast<CNA::RendererFeature>(18);
            const auto invalidLimit = static_cast<CNA::RendererLimit>(2);
            Context().Check(emptyProfile.GetRendererName().empty() && emptyProfile.GetEnglishReport().empty() &&
                                emptyProfile.GetAdditionalLimitationsText().empty() &&
                                emptyProfile.GetFeature(invalidFeature).support ==
                                    CNA::RendererFeatureSupport::Unknown &&
                                !emptyProfile.Supports(invalidFeature) &&
                                !emptyProfile.GetLimit(invalidLimit).known &&
                                emptyProfile.GetSurfaceFormatSupport(999).knownUsages == 0,
                            "an empty profile and retired or invalid identities return explicit unknown results");

            const CNA::DisplayColorSpace originalSpace = device.GetDisplayColorSpaceEXT();
            bool colorSpacesCoherent = device.SupportsDisplayColorSpaceEXT(originalSpace);
            for (const CNA::DisplayColorSpace space : {CNA::DisplayColorSpace::Srgb,
                                                       CNA::DisplayColorSpace::Scrgb,
                                                       CNA::DisplayColorSpace::Hdr10})
            {
                const bool supported = device.SupportsDisplayColorSpaceEXT(space);
                colorSpacesCoherent &= device.GetDisplayColorSpaceEXT() == originalSpace;
                const bool accepted = device.SetDisplayColorSpaceEXT(space);
                colorSpacesCoherent &= accepted == supported &&
                                       device.GetDisplayColorSpaceEXT() == (accepted ? space : originalSpace);
                if (accepted && space != originalSpace)
                    colorSpacesCoherent &= device.SetDisplayColorSpaceEXT(originalSpace);
            }
            colorSpacesCoherent &= device.GetDisplayColorSpaceEXT() == originalSpace;
            Context().Check(colorSpacesCoherent,
                            "display-color-space probes preserve state and setters follow the advertised support");

            // Everything at once, as English text: features, limits, per-format support.
            const std::string_view report = device.GetRendererCapabilityReportEXT();
            Context().Check(report == profile.GetEnglishReport() &&
                                report.find("Detailed features") != std::string_view::npos &&
                                report.find("Numeric limits") != std::string_view::npos &&
                                report.find("Surface-format support") != std::string_view::npos &&
                                report.find(profile.GetRendererName()) != std::string_view::npos &&
                                !profile.GetAdditionalLimitationsText().empty(),
                            "the complete English report reflects the same cached structured profile");
            Context().Print("--- capability report of {} ---", device.GetGraphicsRendererName());
            Context().Print("{}", report);
        }
    };
}

CNA_DEMOS_GAME(CapabilityReportDemo, {
    .id = "graphics.device.capability_report",
    .title = "Renderer capability report",
    .description = "Audits the active renderer's cached structured feature, limit, surface-format and display-color "
                   "space answers, then prints the matching complete English report.",
    .apis = {"GraphicsDevice::GetRendererCapabilityProfileEXT", "GraphicsDevice::GetRendererFeatureSupportEXT",
             "GraphicsDevice::SupportsRendererFeatureEXT", "GraphicsDevice::GetRendererLimitEXT",
             "GraphicsDevice::GetRendererSurfaceFormatSupportEXT",
             "GraphicsDevice::SupportsSurfaceFormatAsRenderTargetEXT",
             "GraphicsDevice::GetRendererCapabilityReportEXT", "GraphicsDevice::GetMaxTextureDimension",
             "GraphicsDevice::GetDisplayColorSpaceEXT", "GraphicsDevice::SetDisplayColorSpaceEXT",
             "GraphicsDevice::SupportsDisplayColorSpaceEXT", "CNA::RendererCapabilityProfile",
             "CNA::RendererCapabilityProfile::GetRendererName", "CNA::RendererCapabilityProfile::GetFeature",
             "CNA::RendererCapabilityProfile::Supports", "CNA::RendererCapabilityProfile::GetLimit",
             "CNA::RendererCapabilityProfile::GetSurfaceFormatSupport",
             "CNA::RendererCapabilityProfile::GetAdditionalLimitationsText",
             "CNA::RendererCapabilityProfile::GetEnglishReport",
             "CNA::RendererFeature", "CNA::RendererFeatureSupport", "CNA::RendererFeatureInfo",
             "CNA::RendererLimit", "CNA::RendererLimitValue", "CNA::RendererFormatUsage",
             "CNA::RendererFormatSupport", "CNA::RendererFormatSupport::IsKnown",
             "CNA::RendererFormatSupport::Supports", "CNA::DisplayColorSpace", "CNA::AllRendererFeatures",
             "CNA::GetRendererFeatureName", "CNA::GetRendererFeatureDescription",
             "CNA::AllRendererLimits", "CNA::GetRendererLimitName"},
    .tags = {"capability", "renderer", "limits", "cnaext"},
    .frames = 1,
})
