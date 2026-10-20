// SPDX-License-Identifier: MS-PL
// Color interpolation boundaries and conversion to/from normalized vectors.
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorLerpAndVectors(CnaDemos::DemoContext& ctx)
    {
        const Color first(10, 20, 30, 40);
        const Color second(110, 220, 130, 240);
        const float infinity = std::numeric_limits<float>::infinity();
        ctx.Check(Color::Lerp(first, second, -1.0f) == first &&
                      Color::Lerp(first, second, 0.0f) == first &&
                      Color::Lerp(first, second, 1.0f) == second &&
                      Color::Lerp(first, second, 2.0f) == second &&
                      Color::Lerp(first, second, -infinity) == first &&
                      Color::Lerp(first, second, infinity) == second,
                  "Lerp clamps finite and infinite amounts to its two endpoints");
        ctx.Check(Color::Lerp(first, second, 0.5f) == Color(60, 120, 80, 140),
                  "Lerp interpolates every channel and truncates fractional byte results");

        const float nan = std::numeric_limits<float>::quiet_NaN();
        ctx.Check(Color::Lerp(first, second, nan) == first,
                  "XNA treats a NaN Lerp amount like zero and returns the first color");

        const Vector3 rgb = first.ToVector3();
        const Vector4 rgba = first.ToVector4();
        ctx.Print("{} -> {}", first.ToString(), rgba.ToString());
        ctx.CheckNear(rgb.X, 10.0 / 255.0, 1e-7, "ToVector3 converts R to its normalized byte value");
        ctx.CheckNear(rgb.Y, 20.0 / 255.0, 1e-7, "ToVector3 converts G to its normalized byte value");
        ctx.Check(rgb.Z == rgba.Z && rgba.W == 40.0f / 255.0f,
                  "ToVector4 retains normalized alpha while ToVector3 omits it");
        ctx.Check(Color(rgba) == first, "constructing from ToVector4 round-trips every byte channel");
    }
}

CNA_DEMOS_CONSOLE(ColorLerpAndVectors, {
    .id = "math.color.lerp_and_vectors",
    .title = "Blend colours and convert to vectors",
    .description = "Checks Color::Lerp endpoint clamping, byte truncation and NaN behavior, then converts all channels "
                   "to normalized Vector3/Vector4 values and round-trips them.",
    .apis = {"Color::Lerp", "Color::ToVector3", "Color::ToVector4"},
    .tags = {"color", "interpolation", "vector", "boundary"},
})
