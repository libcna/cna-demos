// SPDX-License-Identifier: MS-PL
// XNA colours are premultiplied: Color * float and Color::FromNonPremultiplied.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorPremultiplied(CnaDemos::DemoContext& ctx)
    {
        // Fading a sprite: multiply its tint by the opacity -- every channel, alpha included.
        const Color halfWhite = Color::White * 0.5f;
        ctx.Print("Color::White * 0.5 = {}", halfWhite.ToString());
        // XNA multiplies in 16.16 fixed point and truncates: 255 * 0.5 = 127.5 -> 127.
        ctx.Check(halfWhite == Color(127, 127, 127, 127), "multiplying scales colour and alpha together (truncating)");
        ctx.Check(Color::Multiply(Color::White, 0.5f) == halfWhite, "Color::Multiply is the named form");

        // Half-transparent red written the "straight alpha" way...
        const Color straight = Color::FromNonPremultiplied(255, 0, 0, 128);
        ctx.Print("FromNonPremultiplied(255, 0, 0, 128) = {}", straight.ToString());
        // ...becomes premultiplied: colour channels already multiplied by alpha.
        ctx.Check(straight == Color(128, 0, 0, 128), "red is scaled by alpha");
        ctx.Check(Color::FromNonPremultiplied(Vector4(1.0f, 0.0f, 0.0f, 0.5f)) == straight,
                  "the Vector4 overload gives the same colour");

        // Writing Color(255, 0, 0, 128) directly would be "brighter than its alpha" -- invalid
        // under premultiplied blending (BlendState::AlphaBlend), where it looks additive.
        ctx.Check(Color(255, 0, 0, 128) != straight, "Color(r, g, b, a) does NOT premultiply");
    }
}

CNA_DEMOS_CONSOLE(ColorPremultiplied, {
    .id = "math.color.premultiplied_alpha",
    .title = "Premultiplied alpha",
    .description = "Fades a colour with Color * float, converts straight alpha with Color::FromNonPremultiplied, "
                   "and explains why Color(r, g, b, a) is not premultiplied for you.",
    .apis = {"Color::FromNonPremultiplied", "Color::Multiply", "Color::op_Multiply", "Color::White"},
    .tags = {"color", "alpha", "blending", "transparency"},
})
