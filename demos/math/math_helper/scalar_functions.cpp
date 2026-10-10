// SPDX-License-Identifier: MS-PL
// MathHelper's scalar helpers: Clamp, Lerp, Min, Max, Distance and the constants E, Log2E, Log10E.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

using Microsoft::Xna::Framework::MathHelper;

namespace
{
    void MathHelperScalars(CnaDemos::DemoContext& ctx)
    {
        ctx.Print("Clamp(150, 0, 100) = {}", MathHelper::Clamp(150.0f, 0.0f, 100.0f));
        ctx.Print("Lerp(10, 20, 0.3)  = {}", MathHelper::Lerp(10.0f, 20.0f, 0.3f));
        ctx.Print("Min(3, -2) = {}, Max(3, -2) = {}", MathHelper::Min(3.0f, -2.0f), MathHelper::Max(3.0f, -2.0f));
        ctx.Print("Distance(-4, 6)    = {}", MathHelper::Distance(-4.0f, 6.0f));

        ctx.Check(MathHelper::Clamp(150.0f, 0.0f, 100.0f) == 100.0f, "Clamp limits to the maximum");
        ctx.Check(MathHelper::Clamp(-5.0f, 0.0f, 100.0f) == 0.0f, "Clamp limits to the minimum");
        ctx.CheckNear(MathHelper::Lerp(10.0f, 20.0f, 0.3f), 13.0, 1e-5, "Lerp moves 30 % of the way");
        ctx.Check(MathHelper::Min(3.0f, -2.0f) == -2.0f && MathHelper::Max(3.0f, -2.0f) == 3.0f, "Min and Max");
        ctx.Check(MathHelper::Distance(-4.0f, 6.0f) == 10.0f, "Distance is the absolute difference");

        ctx.Print("E = {}, Log2E = {}, Log10E = {}", MathHelper::E, MathHelper::Log2E, MathHelper::Log10E);
        ctx.CheckNear(MathHelper::Log2E, 1.0 / std::log(2.0), 1e-6, "Log2E is log2(e)");
        ctx.CheckNear(MathHelper::Log10E, 1.0 / std::log(10.0), 1e-6, "Log10E is log10(e)");
    }
}

CNA_DEMOS_CONSOLE(MathHelperScalars, {
    .id = "math.math_helper.scalar_functions",
    .title = "Scalar helpers",
    .description = "Clamps, interpolates, compares and measures single float values with MathHelper, and prints "
                   "its E, Log2E and Log10E constants.",
    .apis = {"MathHelper::Clamp", "MathHelper::Lerp", "MathHelper::Min", "MathHelper::Max", "MathHelper::Distance",
             "MathHelper::E", "MathHelper::Log2E", "MathHelper::Log10E"},
    .tags = {"scalar", "clamp", "interpolation"},
})
