// SPDX-License-Identifier: MS-PL
// Blending colours with Color::Lerp and converting to vectors for maths.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorLerpAndVectors(CnaDemos::DemoContext& ctx)
    {
        // A health bar going from green to red.
        for (const float damage : {0.0f, 0.5f, 1.0f})
        {
            ctx.Print("damage {:.1f}: {}", damage, Color::Lerp(Color::Lime, Color::Red, damage).ToString());
        }
        ctx.Check(Color::Lerp(Color::Lime, Color::Red, 0.0f) == Color::Lime, "amount 0 gives the first colour");
        // Like Multiply, Lerp works in fixed point and truncates: halfway from 0 to 255 is 127.
        ctx.Check(Color::Lerp(Color::Black, Color::White, 0.5f) == Color(127, 127, 127, 255), "halfway is mid grey (127)");

        // ToVector3/ToVector4 give 0..1 floats for shader parameters and arithmetic.
        const Vector4 v = Color::CornflowerBlue.ToVector4();
        const Vector3 rgb = Color::CornflowerBlue.ToVector3();
        ctx.Print("CornflowerBlue = {} -> {}", Color::CornflowerBlue.ToString(), v.ToString());
        ctx.CheckNear(v.X, 100.0 / 255.0, 1e-6, "R 100 becomes 100/255");
        ctx.Check(v.W == 1.0f && rgb.X == v.X, "ToVector3 drops alpha");
        ctx.Check(Color(v) == Color::CornflowerBlue, "and the round trip restores the colour");
    }
}

CNA_DEMOS_CONSOLE(ColorLerpAndVectors, {
    .id = "math.color.lerp_and_vectors",
    .title = "Blend colours and convert to vectors",
    .description = "Blends two colours with Color::Lerp and converts colours to 0..1 Vector3/Vector4 values with "
                   "ToVector3 and ToVector4.",
    .apis = {"Color::Lerp", "Color::ToVector3", "Color::ToVector4", "Color::CornflowerBlue"},
    .tags = {"color", "interpolation", "vector"},
})
