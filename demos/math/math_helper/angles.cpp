// SPDX-License-Identifier: MS-PL
// MathHelper angle constants, conversions and exact WrapAngle boundaries.
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

using Microsoft::Xna::Framework::MathHelper;

namespace
{
    bool HasBits(float value, std::uint32_t bits)
    {
        return std::bit_cast<std::uint32_t>(value) == bits;
    }

    float FromBits(std::uint32_t bits)
    {
        return std::bit_cast<float>(bits);
    }

    void MathHelperAngles(CnaDemos::DemoContext& ctx)
    {
        ctx.Print("Pi = {}, TwoPi = {}, PiOver2 = {}, PiOver4 = {}", MathHelper::Pi,
                  MathHelper::TwoPi, MathHelper::PiOver2, MathHelper::PiOver4);
        ctx.Check(HasBits(MathHelper::E, 0x402df854) && HasBits(MathHelper::Log10E, 0x3ede5bd9) &&
                      HasBits(MathHelper::Log2E, 0x3fb8aa3b) && HasBits(MathHelper::Pi, 0x40490fdb) &&
                      HasBits(MathHelper::PiOver2, 0x3fc90fdb) && HasBits(MathHelper::PiOver4, 0x3f490fdb) &&
                      HasBits(MathHelper::TwoPi, 0x40c90fdb),
                  "all seven public constants retain XNA's exact single-precision values");

        const float negativeZero = FromBits(0x80000000);
        ctx.Check(HasBits(MathHelper::ToRadians(180.0f), 0x40490fdb) &&
                      HasBits(MathHelper::ToRadians(1e-7f), 0x30efe050) &&
                      std::signbit(MathHelper::ToRadians(negativeZero)),
                  "ToRadians preserves exact ordinary, tiny and negative-zero results");
        ctx.Check(HasBits(MathHelper::ToDegrees(MathHelper::Pi), 0x43340000) &&
                      HasBits(MathHelper::ToDegrees(1e-7f), 0x36c040b3) &&
                      std::signbit(MathHelper::ToDegrees(negativeZero)),
                  "ToDegrees preserves exact ordinary, tiny and negative-zero results");

        ctx.Check(HasBits(MathHelper::WrapAngle(0.0f), 0x00000000) &&
                      HasBits(MathHelper::WrapAngle(negativeZero), 0x80000000) &&
                      MathHelper::WrapAngle(MathHelper::Pi) == MathHelper::Pi &&
                      MathHelper::WrapAngle(-MathHelper::Pi) == MathHelper::Pi,
                  "WrapAngle preserves signed zero and maps its interval endpoints to positive Pi");
        ctx.Check(HasBits(MathHelper::WrapAngle(FromBits(0x40490fdc)), 0xc0490fda) &&
                      HasBits(MathHelper::WrapAngle(FromBits(0xc0490fdc)), 0x40490fda) &&
                      HasBits(MathHelper::WrapAngle(3.0f * MathHelper::Pi), 0x40490fda) &&
                      HasBits(MathHelper::WrapAngle(-3.0f * MathHelper::Pi), 0xc0490fda),
                  "WrapAngle handles values immediately beyond and several turns around its endpoints");
        ctx.Check(HasBits(MathHelper::WrapAngle(std::numeric_limits<float>::max()), 0x3fddb0f8) &&
                      std::isnan(MathHelper::WrapAngle(std::numeric_limits<float>::infinity())) &&
                      std::isnan(MathHelper::WrapAngle(std::numeric_limits<float>::quiet_NaN())),
                  "WrapAngle matches XNA for the largest finite value, infinity and NaN");
    }
}

CNA_DEMOS_CONSOLE(MathHelperAngles, {
    .id = "math.math_helper.angles",
    .title = "Angles and radians",
    .description = "Checks MathHelper's exact constants, degree/radian conversions, signed zero, interval endpoints, "
                   "whole turns and nonfinite WrapAngle behavior.",
    .apis = {"MathHelper::E", "MathHelper::Log10E", "MathHelper::Log2E", "MathHelper::Pi",
             "MathHelper::PiOver2", "MathHelper::PiOver4", "MathHelper::TwoPi",
             "MathHelper::ToRadians", "MathHelper::ToDegrees", "MathHelper::WrapAngle"},
    .tags = {"angle", "radians", "degrees", "boundaries", "ieee-754"},
})
