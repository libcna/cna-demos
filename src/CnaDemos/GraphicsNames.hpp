// SPDX-License-Identifier: MS-PL
#pragma once

#include <string_view>

#include "CNA/GraphicsCapability.hpp"
#include "Microsoft/Xna/Framework/DisplayOrientation.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentInterval.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"

/**
 * Names of graphics enumerations for demo output. CNA's enums are plain C++ enums without a
 * ToString(), and printing "SurfaceFormat 0" teaches nothing. Output formatting only -- no demo
 * is about these functions.
 */
namespace CnaDemos
{
    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Graphics::SurfaceFormat format)
    {
        using enum Microsoft::Xna::Framework::Graphics::SurfaceFormat;
        switch (format)
        {
        case Color: return "Color";
        case Bgr565: return "Bgr565";
        case Bgra5551: return "Bgra5551";
        case Bgra4444: return "Bgra4444";
        case Dxt1: return "Dxt1";
        case Dxt3: return "Dxt3";
        case Dxt5: return "Dxt5";
        case NormalizedByte2: return "NormalizedByte2";
        case NormalizedByte4: return "NormalizedByte4";
        case Rgba1010102: return "Rgba1010102";
        case Rg32: return "Rg32";
        case Rgba64: return "Rgba64";
        case Alpha8: return "Alpha8";
        case Single: return "Single";
        case Vector2: return "Vector2";
        case Vector4: return "Vector4";
        case HalfSingle: return "HalfSingle";
        case HalfVector2: return "HalfVector2";
        case HalfVector4: return "HalfVector4";
        case HdrBlendable: return "HdrBlendable";
        case ColorBgraEXT: return "ColorBgraEXT";
        case ColorSrgbEXT: return "ColorSrgbEXT";
        case Dxt5SrgbEXT: return "Dxt5SrgbEXT";
        case Bc7EXT: return "Bc7EXT";
        case Bc7SrgbEXT: return "Bc7SrgbEXT";
        case ByteEXT: return "ByteEXT";
        case UShortEXT: return "UShortEXT";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Graphics::DepthFormat format)
    {
        using enum Microsoft::Xna::Framework::Graphics::DepthFormat;
        switch (format)
        {
        case None: return "None";
        case Depth16: return "Depth16";
        case Depth24: return "Depth24";
        case Depth24Stencil8: return "Depth24Stencil8";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Graphics::PresentInterval interval)
    {
        using enum Microsoft::Xna::Framework::Graphics::PresentInterval;
        switch (interval)
        {
        case Default: return "Default";
        case One: return "One";
        case Two: return "Two";
        case Immediate: return "Immediate";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Graphics::RenderTargetUsage usage)
    {
        using enum Microsoft::Xna::Framework::Graphics::RenderTargetUsage;
        switch (usage)
        {
        case DiscardContents: return "DiscardContents";
        case PreserveContents: return "PreserveContents";
        case PlatformContents: return "PlatformContents";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Graphics::GraphicsProfile profile)
    {
        return profile == Microsoft::Xna::Framework::Graphics::GraphicsProfile::Reach ? "Reach" : "HiDef";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::DisplayOrientation orientation)
    {
        using enum Microsoft::Xna::Framework::DisplayOrientation;
        switch (orientation)
        {
        case Default: return "Default";
        case LandscapeLeft: return "LandscapeLeft";
        case LandscapeRight: return "LandscapeRight";
        case Portrait: return "Portrait";
        }
        return "(combination)";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::GraphicsCapability capability)
    {
        using enum CNA::GraphicsCapability;
        switch (capability)
        {
        case ThreeD: return "ThreeD";
        case DepthStencilBuffer: return "DepthStencilBuffer";
        case MultiSampleAntiAliasing: return "MultiSampleAntiAliasing";
        case MultipleRenderTargets: return "MultipleRenderTargets";
        case AnisotropicFiltering: return "AnisotropicFiltering";
        case WireFrame: return "WireFrame";
        case OcclusionQuery: return "OcclusionQuery";
        case CustomEffects: return "CustomEffects";
        case Texture3D: return "Texture3D";
        case MultiStreamVertexInput: return "MultiStreamVertexInput";
        case Instancing: return "Instancing";
        case StencilBuffer: return "StencilBuffer";
        case AdditiveBlending: return "AdditiveBlending";
        case CompiledEffects: return "CompiledEffects";
        case FloatRenderTargets: return "FloatRenderTargets";
        case HalfFloatRenderTargets: return "HalfFloatRenderTargets";
        case HalfFloatTextureLinearFiltering: return "HalfFloatTextureLinearFiltering";
        }
        return "?";
    }
}
