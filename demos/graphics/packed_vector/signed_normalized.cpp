// SPDX-License-Identifier: MS-PL
// Signed normalized formats (-1..1): NormalizedByte2/4 and NormalizedShort2/4, e.g. for normals.
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/IPackedVector.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedByte2.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedByte4.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedShort2.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedShort4.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    void SignedNormalized(CnaDemos::DemoContext& ctx)
    {
        // A unit normal stored in 4 bytes instead of 12.
        const Vector3 normal = Vector3::Normalize(Vector3(0.3f, -0.8f, 0.5f));
        const NormalizedByte4 packedNormal(Vector4(normal, 0.0f));
        const Vector4 unpacked = packedNormal.ToVector4();
        ctx.Print("normal {} -> NormalizedByte4 0x{:08X} -> {}", normal.ToString(), packedNormal.getPackedValueProperty(),
                  unpacked.ToString());
        ctx.CheckNear(Vector3::Distance(Vector3(unpacked.X, unpacked.Y, unpacked.Z), normal), 0.0, 0.01,
                      "8 bits per component keep a normal within about 1 %");

        // 16 bits per component are far more precise.
        const NormalizedShort4 precise(Vector4(normal, 0.0f));
        const Vector4 back = precise.ToVector4();
        ctx.CheckNear(Vector3::Distance(Vector3(back.X, back.Y, back.Z), normal), 0.0, 1e-4, "16 bits keep it within 0.01 %");

        // Out-of-range input clamps to -1..1.
        const NormalizedByte2 bytePair(Vector2(-1.0f, 1.0f));
        const NormalizedShort2 shortPair(Vector2(-1.0f, 1.0f));
        ctx.Check(NormalizedByte2(Vector2(2.0f, -3.0f)).ToVector2() == Vector2(1.0f, -1.0f), "values clamp to -1..1");
        ctx.Check(shortPair.ToVector2() == Vector2(-1.0f, 1.0f), "the end points are exact");

        NormalizedByte2 byteCopy;
        byteCopy.setPackedValueProperty(bytePair.getPackedValueProperty());
        ctx.Check(bytePair.getPackedValueProperty() == 0x7F81u && byteCopy == bytePair && !(byteCopy != bytePair) &&
                      bytePair.Equals(byteCopy) && bytePair.Equals(std::any(byteCopy)) &&
                      !bytePair.Equals(std::any(shortPair)) && byteCopy.GetHashCode() == bytePair.GetHashCode() &&
                      bytePair.ToString() == "7F81",
                  "NormalizedByte2 preserves its packed value, equality, hash and four-digit text");
        NormalizedByte2 byteInterface;
        IPackedVector& packedByte = byteInterface;
        packedByte.PackFromVector4(Vector4(-1.0f, 1.0f, 8.0f, -4.0f));
        ctx.Check(packedByte.ToVector4() == Vector4(-1.0f, 1.0f, 0.0f, 1.0f),
                  "NormalizedByte2's IPackedVector path consumes XY and supplies canonical ZW");

        NormalizedShort2 shortCopy;
        shortCopy.setPackedValueProperty(shortPair.getPackedValueProperty());
        ctx.Check(shortPair.getPackedValueProperty() == 0x7FFF8001u && shortCopy == shortPair &&
                      !(shortCopy != shortPair) && shortPair.Equals(shortCopy) &&
                      shortPair.Equals(std::any(shortCopy)) && !shortPair.Equals(std::any(bytePair)) &&
                      shortCopy.GetHashCode() == shortPair.GetHashCode() &&
                      shortPair.ToString() == "7FFF8001",
                  "NormalizedShort2 preserves its packed value, equality, hash and eight-digit text");
        NormalizedShort2 shortInterface;
        IPackedVector& packedShort = shortInterface;
        packedShort.PackFromVector4(Vector4(-1.0f, 1.0f, 8.0f, -4.0f));
        ctx.Check(packedShort.ToVector4() == Vector4(-1.0f, 1.0f, 0.0f, 1.0f),
                  "NormalizedShort2's IPackedVector path consumes XY and supplies canonical ZW");
    }
}

CNA_DEMOS_CONSOLE(SignedNormalized, {
    .id = "graphics.packed_vector.signed_normalized",
    .title = "Signed normalized formats",
    .description = "Stores a unit normal in NormalizedByte4 and NormalizedShort4, compares their precision, and "
                   "shows NormalizedByte2/NormalizedShort2 clamping to -1..1.",
    .apis = {"NormalizedByte4::NormalizedByte4", "NormalizedByte4::ToVector4", "NormalizedShort4::NormalizedShort4",
             "NormalizedShort4::ToVector4", "NormalizedByte2::NormalizedByte2", "NormalizedByte2::ToVector2",
             "NormalizedByte2::PackedValue", "NormalizedByte2::Equals", "NormalizedByte2::GetHashCode",
             "NormalizedByte2::op_Equality", "NormalizedByte2::op_Inequality", "NormalizedByte2::ToString",
             "NormalizedByte2::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "NormalizedByte2::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#ToVector4",
             "NormalizedShort2::NormalizedShort2", "NormalizedShort2::ToVector2", "NormalizedShort2::PackedValue",
             "NormalizedShort2::Equals", "NormalizedShort2::GetHashCode", "NormalizedShort2::op_Equality",
             "NormalizedShort2::op_Inequality", "NormalizedShort2::ToString",
             "NormalizedShort2::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "NormalizedShort2::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#ToVector4",
             "NormalizedByte4::PackedValue"},
    .tags = {"packed", "normal", "snorm", "vertex-format"},
})
