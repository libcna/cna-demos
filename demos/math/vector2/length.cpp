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

        const float length = v.Length();                // sqrt(3*3 + 4*4)
        const float lengthSquared = v.LengthSquared();  // 3*3 + 4*4, no square root

        ctx.Print("Vector2(3, 4).Length()        = {}", length);
        ctx.Print("Vector2(3, 4).LengthSquared() = {}", lengthSquared);

        ctx.Check(length == 5.0f, "Length of (3, 4) is 5");
        ctx.Check(lengthSquared == 25.0f, "LengthSquared of (3, 4) is 25");
        ctx.Check(Vector2::Zero.Length() == 0.0f, "Length of Vector2::Zero is 0");
    }
}

CNA_DEMOS_CONSOLE(Vector2Length, {
    .id = "math.vector2.length",
    .title = "Vector2 length",
    .description = "Computes the length and squared length of a Vector2; LengthSquared avoids the square root "
                   "when only comparing distances.",
    .apis = {"Vector2::Length", "Vector2::LengthSquared", "Vector2::Zero"},
    .tags = {"vector", "magnitude"},
})
