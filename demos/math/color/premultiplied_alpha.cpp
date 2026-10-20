// SPDX-License-Identifier: MS-PL
// Color scalar multiplication and straight-to-premultiplied alpha conversion.
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorPremultiplied(CnaDemos::DemoContext& ctx)
    {
        const Color source(10, 20, 30, 40);
        const Color half = source * 0.5f;
        ctx.Print("{} * 0.5 = {}", source.ToString(), half.ToString());
        ctx.Check(half == Color(5, 10, 15, 20) && Color::Multiply(source, 0.5f) == half,
                  "operator and named Multiply scale every channel with byte truncation");
        ctx.Check(Color::Multiply(source, -1.0f) == Color(0, 0, 0, 0) &&
                      Color::Multiply(source, 0.0f) == Color(0, 0, 0, 0) &&
                      Color::Multiply(source, 1.0f) == source &&
                      Color::Multiply(source, 20.0f) == Color(200, 255, 255, 255),
                  "Multiply clamps negative, zero, identity and overflowing finite scales");

        const float infinity = std::numeric_limits<float>::infinity();
        const float nan = std::numeric_limits<float>::quiet_NaN();
        ctx.Check(Color::Multiply(source, nan) == Color(0, 0, 0, 0) &&
                      Color::Multiply(source, infinity) == Color(255, 255, 255, 255),
                  "XNA maps NaN products to zero and saturates positive-infinite products");

        const Color straight = Color::FromNonPremultiplied(255, 128, 1, 128);
        ctx.Print("FromNonPremultiplied(255, 128, 1, 128) = {}", straight.ToString());
        ctx.Check(straight == Color(128, 64, 0, 128),
                  "integer premultiplication scales RGB by alpha and truncates");
        ctx.Check(Color::FromNonPremultiplied(255, 128, 1, 0) == Color(0, 0, 0, 0) &&
                      Color::FromNonPremultiplied(255, 128, 1, 255) == Color(255, 128, 1, 255),
                  "zero and opaque alpha are the integer premultiplication endpoints");
        ctx.Check(Color::FromNonPremultiplied(300, -20, 128, 300) == Color(255, 0, 150, 255),
                  "integer premultiplication computes first and clamps resulting channels and alpha");

        const Color vector = Color::FromNonPremultiplied(
            Vector4(1.0f, 0.5f, 1.0f / 255.0f, 128.0f / 255.0f));
        ctx.Check(vector == Color(128, 64, 1, 128),
                  "Vector4 premultiplication uses float-constructor rounding after scaling");
        ctx.Check(Color::FromNonPremultiplied(Vector4(2.0f, -1.0f, 0.5f, 0.5f)) ==
                      Color(255, 0, 64, 128),
                  "Vector4 premultiplication clamps straight channels after multiplying by alpha");
        ctx.Check(Color(255, 128, 1, 128) != straight,
                  "ordinary Color construction stores authored channels without premultiplication");
    }
}

CNA_DEMOS_CONSOLE(ColorPremultiplied, {
    .id = "math.color.premultiplied_alpha",
    .title = "Premultiplied alpha",
    .description = "Exercises Color scalar multiplication over finite and nonfinite boundaries, then compares integer "
                   "and Vector4 straight-to-premultiplied alpha conversions and their clamping/rounding.",
    .apis = {"Color::FromNonPremultiplied", "Color::Multiply", "Color::op_Multiply"},
    .tags = {"color", "alpha", "blending", "transparency", "boundary"},
})
