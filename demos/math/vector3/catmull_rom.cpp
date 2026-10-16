// SPDX-License-Identifier: MS-PL
// A smooth path through points with Vector3::CatmullRom.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3CatmullRom(CnaDemos::DemoContext& ctx)
    {
        // Four control points; CatmullRom interpolates between the middle two (p1 -> p2) and
        // uses the outer two to shape the tangents, so the curve passes through every point.
        const Vector3 p0(0.0f, 0.0f, 0.0f);
        const Vector3 p1(1.0f, 2.0f, 0.0f);
        const Vector3 p2(3.0f, 2.0f, 0.0f);
        const Vector3 p3(4.0f, 0.0f, 0.0f);

        for (const float amount : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f})
        {
            ctx.Print("CatmullRom(p0, p1, p2, p3, {:.2f}) = {}", amount,
                      Vector3::CatmullRom(p0, p1, p2, p3, amount).ToString());
        }
        ctx.Check(Vector3::CatmullRom(p0, p1, p2, p3, 0.0f) == p1, "amount 0 is the second point");
        ctx.Check(Vector3::CatmullRom(p0, p1, p2, p3, 1.0f) == p2, "amount 1 is the third point");
        // The symmetric control points make the curve bulge above the straight segment.
        ctx.Check(Vector3::CatmullRom(p0, p1, p2, p3, 0.5f).Y > 2.0f, "the curve overshoots the chord between p1 and p2");

        const Vector3 a(2.0f, -4.0f, 1.0f);
        const Vector3 b(10.0f, 8.0f, -3.0f);
        const Vector3 c(-6.0f, 12.0f, 5.0f);
        const Vector3 d(14.0f, -20.0f, -7.0f);
        ctx.Check(Vector3::CatmullRom(a, b, c, d, -0.5f) == Vector3(-2.25f, 8.25f, 3.5f),
                  "a negative amount extrapolates before the middle segment");

        Vector3 output;
        Vector3::CatmullRom(a, b, c, d, 1.5f, output);
        ctx.Check(output == Vector3(10.75f, -4.75f, -4.5f) &&
                      output == Vector3::CatmullRom(a, b, c, d, 1.5f),
                  "an above-one amount extrapolates and the output overload matches");
    }
}

CNA_DEMOS_CONSOLE(Vector3CatmullRom, {
    .id = "math.vector3.catmull_rom",
    .title = "Catmull-Rom spline",
    .description = "Evaluates Vector3::CatmullRom between the middle two of four control points; the curve "
                   "passes through the points and bends smoothly.",
    .apis = {"Vector3::CatmullRom"},
    .tags = {"spline", "interpolation", "path"},
})
