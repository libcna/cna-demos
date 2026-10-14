// SPDX-License-Identifier: MS-PL
// Color's 32-bit packed layout (ABGR in memory order R, G, B, A).
#include <cstdint>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/IPackedVector.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorPackedValue(CnaDemos::DemoContext& ctx)
    {
        const Color c(0x11, 0x22, 0x33, 0x44);
        const std::uint32_t packed = c.getPackedValueProperty();
        ctx.Print("Color(0x11, 0x22, 0x33, 0x44).PackedValue = 0x{:08X}", packed);

        // R is the lowest byte, A the highest: 0xAABBGGRR. In memory on a little-endian machine
        // the bytes are R, G, B, A -- the layout SurfaceFormat::Color textures use.
        ctx.Check(packed == 0x44332211u, "PackedValue is 0xAABBGGRR");

        Color fromPacked;
        fromPacked.setPackedValueProperty(0xFF0000FFu);
        ctx.Print("PackedValue 0xFF0000FF = {}", fromPacked.ToString());
        ctx.Check(fromPacked == Color::Red, "0xFF0000FF is opaque red");
        ctx.Check(Color::Transparent.getPackedValueProperty() == 0u, "Color::Transparent packs to zero");

        // Color implements the non-generic packed-vector interface used by format-agnostic converters.
        Graphics::PackedVector::IPackedVector& packedColor = fromPacked;
        packedColor.PackFromVector4(Vector4(0.0f, 0.5f, 1.5f, 0.25f));
        ctx.Check(fromPacked == Color(0, 128, 255, 64),
                  "IPackedVector packing rounds unit values and saturates out-of-range channels");
    }
}

CNA_DEMOS_CONSOLE(ColorPackedValue, {
    .id = "math.color.packed_value",
    .title = "Packed colour value",
    .description = "Reads and writes Color::PackedValue to show the 0xAABBGGRR layout that SurfaceFormat::Color "
                   "pixel data uses.",
    .apis = {"Color::PackedValue", "Color::Transparent", "Color::Red",
             "Color::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4"},
    .tags = {"color", "pixel", "binary"},
})
