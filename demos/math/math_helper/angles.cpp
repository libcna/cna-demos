// SPDX-License-Identifier: MS-PL
// Angles in CNA are radians: MathHelper's constants, conversions and WrapAngle.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

using Microsoft::Xna::Framework::MathHelper;

namespace
{
    void MathHelperAngles(CnaDemos::DemoContext& ctx)
    {
        ctx.Print("Pi = {}, TwoPi = {}, PiOver2 = {}, PiOver4 = {}", MathHelper::Pi, MathHelper::TwoPi, MathHelper::PiOver2,
                  MathHelper::PiOver4);
        ctx.Print("ToRadians(180) = {}", MathHelper::ToRadians(180.0f));
        ctx.Print("ToDegrees(PiOver2) = {}", MathHelper::ToDegrees(MathHelper::PiOver2));

        ctx.CheckNear(MathHelper::ToRadians(180.0f), MathHelper::Pi, 1e-6, "180 degrees is Pi radians");
        ctx.CheckNear(MathHelper::ToDegrees(MathHelper::PiOver4), 45.0, 1e-4, "PiOver4 is 45 degrees");

        // WrapAngle brings any angle into (-Pi, Pi] -- useful for "which way is shorter" turns.
        const float heading = MathHelper::ToRadians(350.0f);
        const float wrapped = MathHelper::WrapAngle(heading);
        ctx.Print("WrapAngle(350 deg) = {} rad = {} deg", wrapped, MathHelper::ToDegrees(wrapped));
        ctx.CheckNear(MathHelper::ToDegrees(wrapped), -10.0, 1e-3, "350 degrees wraps to -10 degrees");
        ctx.CheckNear(MathHelper::WrapAngle(MathHelper::TwoPi * 3.0f + 0.5f), 0.5, 1e-4, "whole turns are removed");
    }
}

CNA_DEMOS_CONSOLE(MathHelperAngles, {
    .id = "math.math_helper.angles",
    .title = "Angles and radians",
    .description = "Uses MathHelper's Pi constants, ToRadians, ToDegrees and WrapAngle to work with angles, which "
                   "CNA expresses in radians.",
    .apis = {"MathHelper::Pi", "MathHelper::TwoPi", "MathHelper::PiOver2", "MathHelper::PiOver4",
             "MathHelper::ToRadians", "MathHelper::ToDegrees", "MathHelper::WrapAngle"},
    .tags = {"angle", "radians", "degrees"},
})
