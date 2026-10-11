// SPDX-License-Identifier: MS-PL
// Signed normalized formats (-1..1): NormalizedByte2/4 and NormalizedShort2/4, e.g. for normals.
#include "CnaDemos/Demo.hpp"
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
        ctx.Check(NormalizedByte2(Vector2(2.0f, -3.0f)).ToVector2() == Vector2(1.0f, -1.0f), "values clamp to -1..1");
        ctx.Check(NormalizedShort2(Vector2(-1.0f, 1.0f)).ToVector2() == Vector2(-1.0f, 1.0f), "the end points are exact");
    }
}

CNA_DEMOS_CONSOLE(SignedNormalized, {
    .id = "graphics.packed_vector.signed_normalized",
    .title = "Signed normalized formats",
    .description = "Stores a unit normal in NormalizedByte4 and NormalizedShort4, compares their precision, and "
                   "shows NormalizedByte2/NormalizedShort2 clamping to -1..1.",
    .apis = {"NormalizedByte4::NormalizedByte4", "NormalizedByte4::ToVector4", "NormalizedShort4::NormalizedShort4",
             "NormalizedShort4::ToVector4", "NormalizedByte2::NormalizedByte2", "NormalizedByte2::ToVector2",
             "NormalizedShort2::NormalizedShort2", "NormalizedShort2::ToVector2", "NormalizedByte4::PackedValue"},
    .tags = {"packed", "normal", "snorm", "vertex-format"},
})
