// SPDX-License-Identifier: MS-PL
// MathHelper scalar bounds, distance, interpolation and CNA-exposed internal helpers.
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

    void MathHelperScalars(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(MathHelper::Clamp(-5.0f, 0.0f, 10.0f) == 0.0f &&
                      MathHelper::Clamp(5.0f, 0.0f, 10.0f) == 5.0f &&
                      MathHelper::Clamp(15.0f, 0.0f, 10.0f) == 10.0f &&
                      MathHelper::Clamp(5.0f, 10.0f, 0.0f) == 10.0f,
                  "float Clamp handles both sides, the interior and XNA's reversed-range ordering");

        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float negativeZero = FromBits(0x80000000);
        ctx.Check(std::isnan(MathHelper::Clamp(nan, 0.0f, 10.0f)) &&
                      MathHelper::Clamp(5.0f, nan, 10.0f) == 5.0f &&
                      MathHelper::Clamp(5.0f, 0.0f, nan) == 5.0f &&
                      std::signbit(MathHelper::Clamp(negativeZero, 0.0f, 0.0f)),
                  "float Clamp preserves a NaN value, ignores NaN bounds and retains signed zero");

        ctx.Check(MathHelper::Min(3.0f, -2.0f) == -2.0f && MathHelper::Max(3.0f, -2.0f) == 3.0f &&
                      !std::signbit(MathHelper::Min(negativeZero, 0.0f)) &&
                      std::signbit(MathHelper::Min(0.0f, negativeZero)) &&
                      !std::signbit(MathHelper::Max(negativeZero, 0.0f)) &&
                      std::signbit(MathHelper::Max(0.0f, negativeZero)),
                  "Min and Max choose XNA's second operand for equal signed-zero values");
        ctx.Check(std::isnan(MathHelper::Min(nan, 7.0f)) && std::isnan(MathHelper::Min(7.0f, nan)) &&
                      std::isnan(MathHelper::Max(nan, 7.0f)) && std::isnan(MathHelper::Max(7.0f, nan)),
                  "Min and Max propagate NaN from either operand");

        ctx.Check(MathHelper::Distance(-4.0f, 6.0f) == 10.0f &&
                      std::isnan(MathHelper::Distance(std::numeric_limits<float>::infinity(),
                                                      std::numeric_limits<float>::infinity())) &&
                      std::isinf(MathHelper::Distance(std::numeric_limits<float>::max(),
                                                      -std::numeric_limits<float>::max())),
                  "Distance exposes ordinary, indeterminate-infinity and overflowing ranges");
        ctx.Check(MathHelper::Lerp(10.0f, 20.0f, -0.5f) == 5.0f &&
                      MathHelper::Lerp(10.0f, 20.0f, 1.5f) == 25.0f &&
                      MathHelper::Lerp(1e20f, -1e20f, 0.5f) == 0.0f &&
                      std::isnan(MathHelper::Lerp(10.0f, 20.0f, nan)) &&
                      std::isinf(MathHelper::Lerp(10.0f, 20.0f,
                                                  std::numeric_limits<float>::infinity())),
                  "Lerp extrapolates, cancels wide finite endpoints and propagates nonfinite amounts");

        ctx.Check(MathHelper::Clamp(-5, 0, 10) == 0 && MathHelper::Clamp(5, 0, 10) == 5 &&
                      MathHelper::Clamp(15, 0, 10) == 10 && MathHelper::Clamp(5, 10, 0) == 10,
                  "CNA's public integer Clamp follows the same ordered comparisons");
        const float epsilon = MathHelper::MachineEpsilonFloat;
        ctx.Check(epsilon == std::numeric_limits<float>::epsilon() * 0.5f &&
                      MathHelper::WithinEpsilon(0.0f, epsilon * 0.5f) &&
                      !MathHelper::WithinEpsilon(0.0f, epsilon) &&
                      !MathHelper::WithinEpsilon(nan, 0.0f),
                  "CNA's exposed epsilon helper uses a strict half-machine-epsilon threshold");
        ctx.Check(MathHelper::ClosestMSAAPower(0) == 0 && MathHelper::ClosestMSAAPower(1) == 0 &&
                      MathHelper::ClosestMSAAPower(2) == 2 && MathHelper::ClosestMSAAPower(3) == 2 &&
                      MathHelper::ClosestMSAAPower(4) == 4 && MathHelper::ClosestMSAAPower(17) == 16,
                  "CNA's exposed MSAA helper floors positive sample counts to usable powers of two");
    }
}

CNA_DEMOS_CONSOLE(MathHelperScalars, {
    .id = "math.math_helper.scalar_functions",
    .title = "Scalar helpers",
    .description = "Exercises MathHelper scalar bounds, comparison, distance and interpolation across ordinary, "
                   "reversed, signed-zero and nonfinite inputs, plus CNA's exposed internal helpers.",
    .apis = {"MathHelper::Clamp", "MathHelper::Lerp", "MathHelper::Min", "MathHelper::Max",
             "MathHelper::Distance", "MathHelper::MachineEpsilonFloat", "MathHelper::WithinEpsilon",
             "MathHelper::ClosestMSAAPower"},
    .tags = {"scalar", "clamp", "interpolation", "boundaries", "ieee-754"},
})
