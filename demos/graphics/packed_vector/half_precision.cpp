// SPDX-License-Identifier: MS-PL
// 16-bit floating point: HalfSingle, HalfVector2 and HalfVector4.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfSingle.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfVector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfVector4.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    void HalfPrecision(CnaDemos::DemoContext& ctx)
    {
        // A half has 11 significant bits: about 3 decimal digits, but a wide range (up to 65504).
        for (const float value : {0.1f, 1.0f / 3.0f, 1000.25f, 65504.0f})
        {
            const HalfSingle half(value);
            ctx.Print("HalfSingle({}) = 0x{:04X} -> {}", value, half.getPackedValueProperty(), half.ToSingle());
        }
        ctx.Check(HalfSingle(1.0f).ToSingle() == 1.0f, "1.0 is exact");
        ctx.Check(HalfSingle(65504.0f).ToSingle() == 65504.0f, "65504 is the largest finite half");
        ctx.CheckNear(HalfSingle(1.0f / 3.0f).ToSingle(), 1.0 / 3.0, 2e-4, "1/3 keeps about three digits");
        ctx.Check(HalfSingle(1000.25f).ToSingle() != 1000.25f, "fractions are lost at large magnitudes");

        // Two and four halves pack into 32 and 64 bits (HDR colour, texture coordinates).
        const HalfVector2 uv(0.25f, 0.75f);
        const HalfVector4 hdr(Vector4(4.0f, 2.0f, 0.5f, 1.0f));
        ctx.Print("HalfVector2 -> {}, HalfVector4 -> {}", uv.ToVector2().ToString(), hdr.ToVector4().ToString());
        ctx.Check(uv.ToVector2() == Vector2(0.25f, 0.75f), "quarters are exact in half precision");
        ctx.Check(hdr.ToVector4().X == 4.0f, "values above 1 survive (unlike normalized formats)");
    }
}

CNA_DEMOS_CONSOLE(HalfPrecision, {
    .id = "graphics.packed_vector.half_precision",
    .title = "Half-precision floats",
    .description = "Packs values into HalfSingle, HalfVector2 and HalfVector4 and shows the precision and range of "
                   "16-bit floating point.",
    .apis = {"HalfSingle::HalfSingle", "HalfSingle::ToSingle", "HalfSingle::PackedValue", "HalfVector2::HalfVector2",
             "HalfVector2::ToVector2", "HalfVector4::HalfVector4", "HalfVector4::ToVector4"},
    .tags = {"packed", "half", "hdr", "float16"},
})
