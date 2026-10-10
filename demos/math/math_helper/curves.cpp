// SPDX-License-Identifier: MS-PL
// One-dimensional curves: MathHelper::SmoothStep, Hermite, CatmullRom and Barycentric.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

using Microsoft::Xna::Framework::MathHelper;

namespace
{
    void MathHelperCurves(CnaDemos::DemoContext& ctx)
    {
        // Fade an opacity from 0 to 1 over a transition; SmoothStep eases both ends.
        ctx.Print("t     SmoothStep(0, 1, t)");
        for (const float t : {0.0f, 0.2f, 0.5f, 0.8f, 1.0f})
        {
            ctx.Print("{:.1f}   {:.4f}", t, MathHelper::SmoothStep(0.0f, 1.0f, t));
        }
        ctx.CheckNear(MathHelper::SmoothStep(0.0f, 1.0f, 0.2f), 0.104, 1e-4, "SmoothStep at 0.2 is 3t^2 - 2t^3 = 0.104");

        // The scalar versions of the spline functions behave like their Vector counterparts.
        ctx.Check(MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, 0.0f) == 10.0f, "CatmullRom starts at the second value");
        ctx.CheckNear(MathHelper::CatmullRom(0.0f, 10.0f, 20.0f, 30.0f, 0.5f), 15.0, 1e-4, "evenly spaced values give a straight line");
        ctx.CheckNear(MathHelper::Hermite(0.0f, 0.0f, 1.0f, 0.0f, 0.5f), 0.5, 1e-6, "Hermite with flat tangents is symmetric");
        ctx.CheckNear(MathHelper::Barycentric(0.0f, 10.0f, 20.0f, 0.5f, 0.25f), 10.0, 1e-5,
                      "Barycentric(v1, v2, v3, a, b) = v1 + a(v2 - v1) + b(v3 - v1)");
    }
}

CNA_DEMOS_CONSOLE(MathHelperCurves, {
    .id = "math.math_helper.curves",
    .title = "Scalar interpolation curves",
    .description = "Evaluates MathHelper::SmoothStep, Hermite, CatmullRom and Barycentric on plain float values, "
                   "for fades, easing and blending.",
    .apis = {"MathHelper::SmoothStep", "MathHelper::Hermite", "MathHelper::CatmullRom", "MathHelper::Barycentric"},
    .tags = {"interpolation", "easing", "spline"},
})
