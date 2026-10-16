// SPDX-License-Identifier: MS-PL
// Vector3::Cross: perpendicular vectors and triangle normals.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Cross(CnaDemos::DemoContext& ctx)
    {
        // Right-handed: X x Y = Z.
        const Vector3 z = Vector3::Cross(Vector3::UnitX, Vector3::UnitY);
        ctx.Print("Cross(UnitX, UnitY) = {}", z.ToString());
        ctx.Check(z == Vector3::UnitZ, "UnitX x UnitY is UnitZ");
        ctx.Check(Vector3::Cross(Vector3::UnitY, Vector3::UnitX) == -Vector3::UnitZ, "swapping the operands negates the result");

        // The normal of a triangle (counter-clockwise when seen from the front).
        const Vector3 a(0.0f, 0.0f, 0.0f);
        const Vector3 b(1.0f, 0.0f, 0.0f);
        const Vector3 c(0.0f, 0.0f, -1.0f);
        const Vector3 normal = Vector3::Normalize(Vector3::Cross(b - a, c - a));
        ctx.Print("normal of the floor triangle a, b, c = {}", normal.ToString());
        ctx.Check(normal == Vector3::Up, "a floor triangle wound this way faces Up");

        // The result is perpendicular to both inputs.
        const Vector3 u(1.0f, 2.0f, 3.0f);
        const Vector3 v(-2.0f, 0.5f, 4.0f);
        const Vector3 n = Vector3::Cross(u, v);
        ctx.CheckNear(Vector3::Dot(n, u), 0.0, 1e-5, "Cross(u, v) is perpendicular to u");
        ctx.CheckNear(Vector3::Dot(n, v), 0.0, 1e-5, "Cross(u, v) is perpendicular to v");

        Vector3 output;
        Vector3::Cross(u, v, output);
        Vector3 alias = u;
        Vector3::Cross(alias, v, alias);
        ctx.Check(output == n && alias == n,
                  "the output overload matches the value form and may alias its first input");

        const Vector3 hugeParallel(1e20f);
        Vector3 hugeParallelOutput;
        Vector3::Cross(hugeParallel, hugeParallel, hugeParallelOutput);
        ctx.Check(Vector3::Cross(hugeParallel, hugeParallel) == Vector3::Zero &&
                      hugeParallelOutput == Vector3::Zero,
                  "XNA cancels equal huge products before narrowing a parallel cross product");

        const Vector3 hugeA(1e20f, 1e20f, 1.0f);
        const Vector3 hugeB(1e20f, 1e20f, 2.0f);
        ctx.Check(Vector3::Cross(hugeA, hugeB) == Vector3(1e20f, -1e20f, 0.0f),
                  "XNA preserves finite terms and exact cancellation in a partly huge cross product");
    }
}

CNA_DEMOS_CONSOLE(Vector3Cross, {
    .id = "math.vector3.cross",
    .title = "Vector3 cross product",
    .description = "Uses Vector3::Cross for perpendiculars and triangle normals, including output aliasing and "
                   "wide-intermediate cancellation.",
    .apis = {"Vector3::Cross", "Vector3::Normalize", "Vector3::Dot"},
    .tags = {"vector", "normal", "perpendicular", "precision"},
    .restrictions = "CNA-DEMOS-ISSUE-123 records loss of XNA's wide Cross intermediates.",
    .knownIssue = "CNA-DEMOS-ISSUE-123",
})
