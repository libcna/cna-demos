// SPDX-License-Identifier: MS-PL
// Vector4's static helpers as colour maths, the way a shader treats a colour: RGBA in 0..1.
// (The Vector3 demos show each helper on its own; these are the same calls with four components.)
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    bool Near(Vector4 a, Vector4 b, float tolerance = 1e-4f) { return Vector4::Distance(a, b) < tolerance; }

    void ColorMath(CnaDemos::DemoContext& ctx)
    {
        const Vector4 orange = Color::Orange.ToVector4();  // (1, 0.647, 0, 1)
        const Vector4 teal = Color::Teal.ToVector4();      // (0, 0.502, 0.502, 1)
        ctx.Print("Orange as Vector4 {}, Teal {}", orange.ToString(), teal.ToString());

        // Fade between two tints: Lerp is even, SmoothStep eases in and out.
        const Vector4 quarter = Vector4::Lerp(orange, teal, 0.25f);
        ctx.Check(Near(quarter, orange * 0.75f + teal * 0.25f), "Lerp at 0.25 is 3/4 orange, 1/4 teal");
        ctx.Check(Vector4::SmoothStep(orange, teal, 0.25f).X > quarter.X, "SmoothStep has faded less at 0.25");

        // Modulate (a texture times a vertex colour): component-wise Multiply.
        const Vector4 texel(0.8f, 0.6f, 0.4f, 1.0f), tint(0.5f, 1.0f, 0.5f, 1.0f);
        ctx.Check(Near(Vector4::Multiply(texel, tint), Vector4(0.4f, 0.6f, 0.2f, 1.0f)),
                  "Multiply(texel, tint) modulates per channel");

        // Lighting can push a value past 1; Clamp saturates it for an 8-bit target.
        const Vector4 lit = Vector4::Add(Vector4::Multiply(texel, 1.5f), Vector4(0.1f, 0.1f, 0.1f, 0.0f));
        const Vector4 saturated = Vector4::Clamp(lit, Vector4::Zero, Vector4::One);
        ctx.Print("lit {} -> saturated {}", lit.ToString(), saturated.ToString());
        ctx.Check(Near(saturated, Vector4(1.0f, 1.0f, 0.7f, 1.0f)), "Clamp to 0..1 saturates the over-bright channels");

        // Luminance: a Dot with the Rec. 601 weights (alpha weighted 0).
        const Vector4 weights(0.299f, 0.587f, 0.114f, 0.0f);
        const float luma = Vector4::Dot(orange, weights);
        ctx.Print("luminance of orange {:.3f}", luma);
        ctx.Check(luma > 0.67f && luma < 0.69f, "Dot with (0.299, 0.587, 0.114, 0) gives orange's luminance, 0.679");

        // Invert RGB, keep alpha: One - colour, then put alpha back.
        Vector4 inverted = Vector4::Subtract(Vector4::One, orange);
        inverted.W = orange.W;
        ctx.Check(Near(inverted, Vector4(0.0f, 1.0f - orange.Y, 1.0f, 1.0f)), "One - colour inverts the RGB channels");
        ctx.Check(Near(Vector4::Negate(orange) + orange, Vector4::Zero), "Negate is the additive inverse");

        // Gouraud shading: the colour at a point inside a triangle from its corner colours.
        const Vector4 red(1.0f, 0.0f, 0.0f, 1.0f), green(0.0f, 1.0f, 0.0f, 1.0f), blue(0.0f, 0.0f, 1.0f, 1.0f);
        const Vector4 centre = Vector4::Barycentric(red, green, blue, 1.0f / 3.0f, 1.0f / 3.0f);
        ctx.Check(Near(centre, Vector4(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f)),
                  "Barycentric at the centre mixes the three corners equally");

        // A gradient through colour keys: CatmullRom passes through the inner keys, Hermite takes
        // explicit slopes.
        ctx.Check(Near(Vector4::CatmullRom(red, orange, teal, blue, 1.0f), teal), "CatmullRom reaches the third key at 1");
        ctx.Check(Near(Vector4::Hermite(red, Vector4::Zero, blue, Vector4::Zero, 0.5f), Vector4::Lerp(red, blue, 0.5f)),
                  "Hermite with flat slopes is half way at 0.5");

        // The brightest and darkest of each channel, e.g. for a palette's range.
        ctx.Check(Near(Vector4::Max(orange, teal), Vector4(1.0f, orange.Y, teal.Z, 1.0f)) &&
                      Near(Vector4::Min(orange, teal), Vector4(0.0f, teal.Y, 0.0f, 1.0f)),
                  "Max and Min work per channel");

        // How different two colours are: the distance between them as points in RGBA.
        ctx.Print("DistanceSquared(orange, teal) = {:.3f}, Length(orange) = {:.3f}",
                  Vector4::DistanceSquared(orange, teal), orange.Length());
        ctx.Check(Vector4::DistanceSquared(orange, orange) == 0.0f && Vector4::Distance(orange, teal) > 1.0f,
                  "Distance: zero for the same colour, over 1 for orange vs teal");
        ctx.Check(Near(Vector4::Divide(Vector4(2.0f, 1.0f, 0.5f, 1.0f), 2.0f), Vector4(1.0f, 0.5f, 0.25f, 0.5f)),
                  "Divide by a scalar scales every channel, alpha too");
    }
}

CNA_DEMOS_CONSOLE(ColorMath, {
    .id = "math.vector4.color_math",
    .title = "Vector4 as colour maths",
    .description = "Vector4's static helpers on RGBA colours as a shader sees them: Lerp/SmoothStep fades, Multiply "
                   "modulation, Clamp saturation, a Dot for luminance, inversion with Subtract, Barycentric (Gouraud) "
                   "mixing, CatmullRom/Hermite gradients, Min/Max and Distance between colours.",
    .apis = {"Vector4::Lerp", "Vector4::SmoothStep", "Vector4::Multiply", "Vector4::Add", "Vector4::Clamp",
             "Vector4::Dot", "Vector4::Subtract", "Vector4::Negate", "Vector4::Barycentric", "Vector4::CatmullRom",
             "Vector4::Hermite", "Vector4::Max", "Vector4::Min", "Vector4::Distance", "Vector4::DistanceSquared",
             "Vector4::Length", "Vector4::Divide", "Color::ToVector4"},
    .tags = {"vector", "color", "math", "shader"},
})
