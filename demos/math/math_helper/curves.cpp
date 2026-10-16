// SPDX-License-Identifier: MS-PL
// MathHelper scalar interpolation endpoints, extrapolation and nonfinite behavior.
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

using Microsoft::Xna::Framework::MathHelper;

namespace
{
    float FromBits(std::uint32_t bits)
    {
        return std::bit_cast<float>(bits);
    }

    bool HasBits(float value, std::uint32_t bits)
    {
        return std::bit_cast<std::uint32_t>(value) == bits;
    }

    void MathHelperCurves(CnaDemos::DemoContext& ctx)
    {
        ctx.Print("t     SmoothStep(0, 1, t)");
        for (const float t : {0.0f, 0.2f, 0.5f, 0.8f, 1.0f})
        {
            ctx.Print("{:.1f}   {:.4f}", t, MathHelper::SmoothStep(0.0f, 1.0f, t));
        }

        const float nan = std::numeric_limits<float>::quiet_NaN();
        ctx.CheckNear(MathHelper::SmoothStep(0.0f, 1.0f, 0.2f), 0.104, 1e-4,
                      "SmoothStep at 0.2 is 3t^2 - 2t^3");
        ctx.Check(MathHelper::SmoothStep(2.0f, 7.0f, -0.5f) == 2.0f &&
                      MathHelper::SmoothStep(2.0f, 7.0f, 1.5f) == 7.0f &&
                      std::isnan(MathHelper::SmoothStep(2.0f, 7.0f, nan)),
                  "SmoothStep clamps finite amounts to its endpoints and propagates NaN");

        ctx.Check(MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, 0.0f) == 10.0f &&
                      MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, 1.0f) == 20.0f &&
                      MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, -0.5f) == 5.0f &&
                      MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, 1.5f) == 25.0f &&
                      std::isnan(MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, nan)),
                  "CatmullRom reaches its middle endpoints, extrapolates and propagates NaN");

        ctx.Check(MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, 0.0f) == 2.0f &&
                      MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, 1.0f) == 7.0f &&
                      MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, FromBits(0x33000000)) == 2.0f &&
                      HasBits(MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, FromBits(0x3f7fffff)),
                              0x40dfffff) &&
                      HasBits(MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, FromBits(0x3f800001)),
                              0x40e00003),
                  "Hermite matches XNA's exact and epsilon-adjacent endpoint behavior");
        ctx.Check(MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, -0.5f) == -0.5f &&
                      MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, 1.5f) == 15.5f &&
                      std::isnan(MathHelper::Hermite(2.0f, 3.0f, 7.0f, 11.0f, nan)),
                  "Hermite extrapolates on either side and propagates NaN");

        ctx.Check(MathHelper::Barycentric(1.0f, 3.0f, 5.0f, 0.0f, 0.0f) == 1.0f &&
                      MathHelper::Barycentric(1.0f, 3.0f, 5.0f, -1.0f, 2.0f) == 7.0f &&
                      std::isnan(MathHelper::Barycentric(1.0f, 3.0f, 5.0f, nan, 0.0f)),
                  "Barycentric covers its origin, extrapolated weights and NaN propagation");
    }
}

CNA_DEMOS_CONSOLE(MathHelperCurves, {
    .id = "math.math_helper.curves",
    .title = "Scalar interpolation curves",
    .description = "Evaluates SmoothStep, CatmullRom, Hermite and Barycentric at ordinary points, exact and near "
                   "endpoints, extrapolated amounts and NaN.",
    .apis = {"MathHelper::SmoothStep", "MathHelper::Hermite", "MathHelper::CatmullRom",
             "MathHelper::Barycentric"},
    .tags = {"interpolation", "easing", "spline", "boundaries", "ieee-754"},
})
