// SPDX-License-Identifier: MS-PL
// 16-bit floating point: HalfSingle, HalfVector2 and HalfVector4.
#include <cmath>
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfSingle.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfVector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfVector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/IPackedVector.hpp"
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

        const HalfSingle one(1.0f);
        HalfSingle oneCopy;
        oneCopy.setPackedValueProperty(one.getPackedValueProperty());
        ctx.Check(one.getPackedValueProperty() == 0x3C00u && oneCopy == one && !(oneCopy != one) &&
                      one.Equals(oneCopy) && one.Equals(std::any(oneCopy)) && !one.Equals(std::any(1.0f)) &&
                      oneCopy.GetHashCode() == one.GetHashCode() && one.ToString() == "1",
                  "HalfSingle preserves its packed value, equality, hash and expanded text");
        HalfSingle singleInterface;
        IPackedVector& packedSingle = singleInterface;
        packedSingle.PackFromVector4(Vector4(1.0f, 9.0f, -4.0f, 0.0f));
        ctx.Check(packedSingle.ToVector4() == Vector4(1.0f, 0.0f, 0.0f, 1.0f),
                  "HalfSingle's IPackedVector path consumes X and supplies canonical YZW");

        // Two and four halves pack into 32 and 64 bits (HDR colour, texture coordinates).
        const HalfVector2 uv(0.25f, 0.75f);
        const HalfVector4 hdr(Vector4(4.0f, 2.0f, 0.5f, 1.0f));
        ctx.Print("HalfVector2 -> {}, HalfVector4 -> {}", uv.ToVector2().ToString(), hdr.ToVector4().ToString());
        ctx.Check(uv.ToVector2() == Vector2(0.25f, 0.75f), "quarters are exact in half precision");
        ctx.Check(hdr.ToVector4().X == 4.0f, "values above 1 survive (unlike normalized formats)");

        HalfVector2 uvCopy;
        uvCopy.setPackedValueProperty(uv.getPackedValueProperty());
        ctx.Check(uvCopy == uv && !(uvCopy != uv) && uv.Equals(uvCopy) && uv.Equals(std::any(uvCopy)) &&
                      !uv.Equals(std::any(hdr)) && uvCopy.GetHashCode() == uv.GetHashCode() &&
                      uv.ToString() == "{X:0.25 Y:0.75}",
                  "HalfVector2 packed copies preserve equality, boxed equality, hash and expanded text");
        HalfVector2 interfaceUv;
        IPackedVector& packedUv = interfaceUv;
        packedUv.PackFromVector4(Vector4(0.25f, 0.75f, 9.0f, -3.0f));
        ctx.Check(packedUv.ToVector4() == Vector4(0.25f, 0.75f, 0.0f, 1.0f),
                  "HalfVector2's IPackedVector path consumes XY and supplies canonical ZW");

        HalfVector4 hdrCopy;
        hdrCopy.setPackedValueProperty(hdr.getPackedValueProperty());
        ctx.Check(hdr.getPackedValueProperty() == 0x3C00380040004400ull && hdrCopy == hdr &&
                      !(hdrCopy != hdr) && hdr.Equals(hdrCopy) && hdr.Equals(std::any(hdrCopy)) &&
                      !hdr.Equals(std::any(uv)) && hdrCopy.GetHashCode() == hdr.GetHashCode() &&
                      hdr.ToString() == "{X:4 Y:2 Z:0.5 W:1}",
                  "HalfVector4 packed copies preserve equality, boxed equality, hash and expanded text");
        HalfVector4 interfaceHdr;
        IPackedVector& packedHdr = interfaceHdr;
        packedHdr.PackFromVector4(Vector4(4.0f, 2.0f, 0.5f, 1.0f));
        ctx.Check(packedHdr.ToVector4() == Vector4(4.0f, 2.0f, 0.5f, 1.0f),
                  "HalfVector4's IPackedVector path preserves all four components");
    }
}

CNA_DEMOS_CONSOLE(HalfPrecision, {
    .id = "graphics.packed_vector.half_precision",
    .title = "Half-precision floats",
    .description = "Packs values into HalfSingle, HalfVector2 and HalfVector4 and shows the precision and range of "
                   "16-bit floating point.",
    .apis = {"HalfSingle::HalfSingle", "HalfSingle::ToSingle", "HalfSingle::PackedValue", "HalfSingle::Equals",
             "HalfSingle::GetHashCode", "HalfSingle::op_Equality", "HalfSingle::op_Inequality",
             "HalfSingle::ToString",
             "HalfSingle::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "HalfSingle::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#ToVector4",
             "HalfVector2::HalfVector2",
             "HalfVector2::ToVector2", "HalfVector2::PackedValue", "HalfVector2::Equals",
             "HalfVector2::GetHashCode", "HalfVector2::op_Equality", "HalfVector2::op_Inequality",
             "HalfVector2::ToString",
             "HalfVector2::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "HalfVector2::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#ToVector4",
             "HalfVector4::HalfVector4", "HalfVector4::ToVector4", "HalfVector4::PackedValue",
             "HalfVector4::Equals", "HalfVector4::GetHashCode", "HalfVector4::op_Equality",
             "HalfVector4::op_Inequality", "HalfVector4::ToString",
             "HalfVector4::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4"},
    .tags = {"packed", "half", "hdr", "float16"},
})
