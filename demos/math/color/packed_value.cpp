// SPDX-License-Identifier: MS-PL
// Color's 32-bit packed layout and IPackedVector conversion boundaries.
#include <cstdint>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/IPackedVector.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorPackedValue(CnaDemos::DemoContext& ctx)
    {
        const Color authored(0x11, 0x22, 0x33, 0x44);
        const std::uint32_t packed = authored.getPackedValueProperty();
        ctx.Print("Color(0x11, 0x22, 0x33, 0x44).PackedValue = 0x{:08X}", packed);
        ctx.Check(packed == 0x44332211u, "PackedValue stores R as the low byte and A as the high byte");

        Color unpacked;
        unpacked.setPackedValueProperty(0xA1B2C3D4u);
        ctx.Check(unpacked == Color(212, 195, 178, 161) &&
                      unpacked.getPackedValueProperty() == 0xA1B2C3D4u,
                  "writing PackedValue exposes its four AABBGGRR bytes through the channel properties");
        ctx.Check(Color::Transparent.getPackedValueProperty() == 0u,
                  "the all-zero packed value is XNA's Transparent color");

        Graphics::PackedVector::IPackedVector& packedColor = unpacked;
        packedColor.PackFromVector4(Vector4(0.25f, 0.5f, 1.5f, 0.125f));
        ctx.Check(unpacked == Color(64, 128, 255, 32),
                  "IPackedVector packing rounds unit values and saturates out-of-range channels");

        const float infinity = std::numeric_limits<float>::infinity();
        const float nan = std::numeric_limits<float>::quiet_NaN();
        packedColor.PackFromVector4(Vector4(nan, infinity, -infinity, 1.0f));
        ctx.Check(unpacked == Color(0, 255, 0, 255),
                  "IPackedVector packing handles NaN and both infinities like Color construction");
    }
}

CNA_DEMOS_CONSOLE(ColorPackedValue, {
    .id = "math.color.packed_value",
    .title = "Packed colour value",
    .description = "Reads and writes Color::PackedValue's AABBGGRR layout and exercises IPackedVector packing through "
                   "rounding, saturation, NaN and infinity boundaries.",
    .apis = {"Color::PackedValue", "Color::Transparent",
             "Color::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4"},
    .tags = {"color", "pixel", "binary", "packed-vector"},
})
