// SPDX-License-Identifier: MS-PL
// Vector3::Hermite: a curve defined by two end points and their tangents.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Hermite(CnaDemos::DemoContext& ctx)
    {
        const Vector3 start(0.0f, 0.0f, 0.0f);
        const Vector3 end(10.0f, 0.0f, 0.0f);
        // Leave upwards, arrive downwards: the path arcs over the straight line.
        const Vector3 startTangent(0.0f, 20.0f, 0.0f);
        const Vector3 endTangent(0.0f, -20.0f, 0.0f);

        for (const float amount : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f})
        {
            ctx.Print("Hermite(..., {:.2f}) = {}", amount,
                      Vector3::Hermite(start, startTangent, end, endTangent, amount).ToString());
        }
        ctx.Check(Vector3::Hermite(start, startTangent, end, endTangent, 0.0f) == start, "amount 0 is the start");
        ctx.Check(Vector3::Hermite(start, startTangent, end, endTangent, 1.0f) == end, "amount 1 is the end");
        ctx.CheckNear(Vector3::Hermite(start, startTangent, end, endTangent, 0.5f).Y, 5.0, 1e-4,
                      "the midpoint is lifted by an eighth of the tangent difference");
    }
}

CNA_DEMOS_CONSOLE(Vector3Hermite, {
    .id = "math.vector3.hermite",
    .title = "Hermite curve",
    .description = "Evaluates Vector3::Hermite from two end points and two tangents; the tangents bend the path "
                   "away from the straight line.",
    .apis = {"Vector3::Hermite"},
    .tags = {"spline", "interpolation", "tangent"},
})
