// SPDX-License-Identifier: MS-PL
// Vector3::SmoothStep: interpolation that eases in and out.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3SmoothStep(CnaDemos::DemoContext& ctx)
    {
        const Vector3 start = Vector3::Zero;
        const Vector3 end(100.0f, 0.0f, 0.0f);

        ctx.Print("amount   Lerp X   SmoothStep X");
        for (const float amount : {0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 1.0f})
        {
            ctx.Print("{:6.2f} {:8.2f} {:14.2f}", amount, Vector3::Lerp(start, end, amount).X,
                      Vector3::SmoothStep(start, end, amount).X);
        }

        // Same end points and midpoint as Lerp, but slower near both ends.
        ctx.Check(Vector3::SmoothStep(start, end, 0.0f) == start, "amount 0 returns the start");
        ctx.Check(Vector3::SmoothStep(start, end, 1.0f) == end, "amount 1 returns the end");
        ctx.CheckNear(Vector3::SmoothStep(start, end, 0.5f).X, 50.0, 1e-4, "the midpoint matches Lerp");
        ctx.Check(Vector3::SmoothStep(start, end, 0.1f).X < Vector3::Lerp(start, end, 0.1f).X,
                  "it starts slower than Lerp");
        // Unlike Lerp, the amount is clamped to [0, 1].
        ctx.Check(Vector3::SmoothStep(start, end, 1.5f) == end, "amounts above 1 are clamped");

        const Vector3 offsetStart(2.0f, -4.0f, 1.0f);
        const Vector3 offsetEnd(10.0f, 8.0f, -3.0f);
        Vector3 belowOutput;
        Vector3::SmoothStep(offsetStart, offsetEnd, -0.5f, belowOutput);
        ctx.Check(Vector3::SmoothStep(offsetStart, offsetEnd, -0.5f) == offsetStart &&
                      belowOutput == offsetStart,
                  "a negative amount clamps to the nonzero start in value and output forms");

        const Vector3 nan = Vector3::SmoothStep(offsetStart, offsetEnd, NAN);
        ctx.Check(std::isnan(nan.X) && std::isnan(nan.Y) && std::isnan(nan.Z),
                  "a NaN amount propagates to every component");
    }
}

CNA_DEMOS_CONSOLE(Vector3SmoothStep, {
    .id = "math.vector3.smooth_step",
    .title = "Smooth-step interpolation",
    .description = "Compares Vector3::SmoothStep with Vector3::Lerp: same end points, but SmoothStep eases in and "
                   "out and clamps the amount.",
    .apis = {"Vector3::SmoothStep", "Vector3::Lerp"},
    .tags = {"interpolation", "easing"},
})
