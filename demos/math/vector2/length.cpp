// SPDX-License-Identifier: MS-PL
// Vector2::Length and Vector2::LengthSquared.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using Microsoft::Xna::Framework::Vector2;

namespace
{
    void Vector2Length(CnaDemos::DemoContext& ctx)
    {
        const Vector2 v(3.0f, 4.0f);
        ctx.Check(v.X == 3.0f && v.Y == 4.0f, "the constructor preserves X and Y");

        const float length = v.Length();                // sqrt(3*3 + 4*4)
        const float lengthSquared = v.LengthSquared();  // 3*3 + 4*4, no square root

        ctx.Print("Vector2(3, 4).Length()        = {}", length);
        ctx.Print("Vector2(3, 4).LengthSquared() = {}", lengthSquared);

        ctx.Check(length == 5.0f, "Length of (3, 4) is 5");
        ctx.Check(lengthSquared == 25.0f, "LengthSquared of (3, 4) is 25");
        ctx.Check(Vector2::Zero.Length() == 0.0f, "Length of Vector2::Zero is 0");
        ctx.Check(Vector2::One == Vector2::UnitX + Vector2::UnitY &&
                      Vector2::UnitX.LengthSquared() == 1.0f && Vector2::UnitY.LengthSquared() == 1.0f,
                  "One is the sum of the two unit axes");
    }
}

CNA_DEMOS_CONSOLE(Vector2Length, {
    .id = "math.vector2.length",
    .title = "Vector2 length",
    .description = "Computes the length and squared length of a Vector2; LengthSquared avoids the square root "
                   "when only comparing distances.",
    .apis = {"Vector2::Vector2", "Vector2::Length", "Vector2::LengthSquared", "Vector2::Zero",
             "Vector2::One", "Vector2::UnitX", "Vector2::UnitY", "Vector2::X", "Vector2::Y"},
    .tags = {"vector", "magnitude"},
})
