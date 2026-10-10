// SPDX-License-Identifier: MS-PL
// Vector3::Barycentric: a point inside a triangle from two weights.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Barycentric(CnaDemos::DemoContext& ctx)
    {
        const Vector3 a(0.0f, 0.0f, 0.0f);
        const Vector3 b(6.0f, 0.0f, 0.0f);
        const Vector3 c(0.0f, 6.0f, 0.0f);

        // result = a + amount1 * (b - a) + amount2 * (c - a)
        const Vector3 centroid = Vector3::Barycentric(a, b, c, 1.0f / 3.0f, 1.0f / 3.0f);
        ctx.Print("centroid      = {}", centroid.ToString());
        ctx.Print("weights (0,0) = {}", Vector3::Barycentric(a, b, c, 0.0f, 0.0f).ToString());
        ctx.Print("weights (1,0) = {}", Vector3::Barycentric(a, b, c, 1.0f, 0.0f).ToString());
        ctx.Print("weights (0,1) = {}", Vector3::Barycentric(a, b, c, 0.0f, 1.0f).ToString());

        ctx.CheckNear(Vector3::Distance(centroid, Vector3(2.0f, 2.0f, 0.0f)), 0.0, 1e-5, "equal weights give the centroid");
        ctx.Check(Vector3::Barycentric(a, b, c, 0.0f, 0.0f) == a, "weights (0, 0) give the first vertex");
        ctx.Check(Vector3::Barycentric(a, b, c, 1.0f, 0.0f) == b, "weights (1, 0) give the second vertex");
        ctx.Check(Vector3::Barycentric(a, b, c, 0.0f, 1.0f) == c, "weights (0, 1) give the third vertex");
    }
}

CNA_DEMOS_CONSOLE(Vector3Barycentric, {
    .id = "math.vector3.barycentric",
    .title = "Barycentric coordinates",
    .description = "Finds points inside a triangle with Vector3::Barycentric from two weights relative to the "
                   "first vertex.",
    .apis = {"Vector3::Barycentric"},
    .tags = {"triangle", "interpolation"},
})
