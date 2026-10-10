// SPDX-License-Identifier: MS-PL
// Integer packed formats: Byte4, Short2 and Short4 (bone indices, integer coordinates).
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Byte4.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Short2.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Short4.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    void IntegerFormats(CnaDemos::DemoContext& ctx)
    {
        // Four bone indices of a skinned vertex in one 32-bit value.
        const Byte4 bones(3.0f, 17.0f, 42.0f, 255.0f);
        ctx.Print("Byte4(3, 17, 42, 255) = 0x{:08X} -> {}", bones.getPackedValueProperty(), bones.ToVector4().ToString());
        ctx.Check(bones.getPackedValueProperty() == 0xFF2A1103u, "X is the lowest byte, W the highest");
        ctx.Check(Byte4(300.0f, -5.0f, 1.4f, 1.6f).ToVector4() == Vector4(255.0f, 0.0f, 1.0f, 2.0f),
                  "values round and clamp to 0..255");

        // Signed 16-bit integers.
        const Short2 offset(-1200.0f, 40000.0f);
        ctx.Print("Short2(-1200, 40000) -> {}", offset.ToVector2().ToString());
        ctx.Check(offset.ToVector2() == Vector2(-1200.0f, 32767.0f), "Short2 clamps to -32768..32767");
        ctx.Check(Short4(1.0f, -2.0f, 3.0f, -4.0f).ToVector4() == Vector4(1.0f, -2.0f, 3.0f, -4.0f),
                  "Short4 keeps small integers exactly");
    }
}

CNA_DEMOS_CONSOLE(IntegerFormats, {
    .id = "graphics.packed_vector.integer_formats",
    .title = "Integer packed formats",
    .description = "Packs bone indices into Byte4 and signed values into Short2 and Short4, showing the byte layout, "
                   "rounding and clamping.",
    .apis = {"Byte4::Byte4", "Byte4::ToVector4", "Byte4::PackedValue", "Short2::Short2", "Short2::ToVector2",
             "Short4::Short4", "Short4::ToVector4"},
    .tags = {"packed", "integer", "skinning", "vertex-format"},
})
