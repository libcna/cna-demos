// SPDX-License-Identifier: MS-PL
// Making colours from bytes, ints, floats and vectors -- and how out-of-range values clamp.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorConstruct(CnaDemos::DemoContext& ctx)
    {
        const Color fromInts(255, 128, 0);              // alpha defaults to 255
        const Color fromFloats(1.0f, 0.5f, 0.0f, 1.0f); // 0..1 per channel
        const Color fromVector(Vector4(1.0f, 0.5f, 0.0f, 1.0f));
        ctx.Print("Color(255, 128, 0)         = {}", fromInts.ToString());
        ctx.Print("Color(1, 0.5, 0, 1)        = {}", fromFloats.ToString());
        ctx.Print("Color(Vector4(1, .5, 0, 1)) = {}", fromVector.ToString());

        ctx.Check(fromInts.getRProperty() == 255 && fromInts.getGProperty() == 128 && fromInts.getAProperty() == 255,
                  "integer channels are taken as they are, alpha defaults to 255");
        ctx.Check(fromFloats == fromVector, "the float and Vector4 constructors agree");
        ctx.Check(fromFloats.getGProperty() == 128, "0.5 rounds to 128");

        // Out-of-range input clamps instead of wrapping.
        const Color clampedInts(300, -20, 128, 999);
        const Color clampedFloats(Vector3(2.0f, -1.0f, 0.5f));
        ctx.Print("Color(300, -20, 128, 999)  = {}", clampedInts.ToString());
        ctx.Check(clampedInts == Color(255, 0, 128, 255), "ints clamp to 0..255");
        ctx.Check(clampedFloats == Color(255, 0, 128, 255), "floats clamp to 0..1");

        Color edited = Color::Black;
        edited.setGProperty(200);
        ctx.Check(edited == Color(0, 200, 0), "channels can be set one by one");
    }
}

CNA_DEMOS_CONSOLE(ColorConstruct, {
    .id = "math.color.construct",
    .title = "Construct colours",
    .description = "Creates Color values from ints, floats, Vector3 and Vector4, shows that out-of-range channels "
                   "clamp, and edits single channels through the R/G/B/A properties.",
    .apis = {"Color::Color", "Color::R", "Color::G", "Color::B", "Color::A", "Color::ToString"},
    .tags = {"color", "rgba"},
})
