// SPDX-License-Identifier: MS-PL
// Linear interpolation between two positions with Vector3::Lerp.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Lerp(CnaDemos::DemoContext& ctx)
    {
        const Vector3 start(0.0f, 0.0f, 0.0f);
        const Vector3 end(10.0f, 20.0f, -40.0f);

        // amount 0 gives start, 1 gives end, anything between moves along the straight line.
        for (const float amount : {0.0f, 0.25f, 0.5f, 1.0f})
        {
            ctx.Print("Lerp(start, end, {:.2f}) = {}", amount, Vector3::Lerp(start, end, amount).ToString());
        }
        ctx.Check(Vector3::Lerp(start, end, 0.0f) == start, "amount 0 returns the start");
        ctx.Check(Vector3::Lerp(start, end, 1.0f) == end, "amount 1 returns the end");
        ctx.Check(Vector3::Lerp(start, end, 0.25f) == Vector3(2.5f, 5.0f, -10.0f), "amount 0.25 is a quarter of the way");

        // Lerp does not clamp: amounts outside [0, 1] extrapolate.
        ctx.Check(Vector3::Lerp(start, end, 2.0f) == end * 2.0f, "amount 2 extrapolates past the end");
    }
}

CNA_DEMOS_CONSOLE(Vector3Lerp, {
    .id = "math.vector3.lerp",
    .title = "Linear interpolation",
    .description = "Moves between two positions with Vector3::Lerp; amounts outside 0..1 extrapolate along the "
                   "same line.",
    .apis = {"Vector3::Lerp"},
    .tags = {"interpolation", "tween"},
})
