// SPDX-License-Identifier: MS-PL
// 16-bit and 8-bit colour formats: Bgr565, Bgra5551, Bgra4444 and Alpha8 trade precision for size.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Alpha8.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgr565.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra4444.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra5551.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    void LowPrecisionColor(CnaDemos::DemoContext& ctx)
    {
        const Vector4 orange(1.0f, 0.5f, 0.0f, 0.6f);

        // 5 bits red, 6 bits green, 5 bits blue, no alpha: 2 bytes per pixel.
        const Bgr565 rgb565(orange.X, orange.Y, orange.Z);
        ctx.Print("Bgr565   0x{:04X} -> {}", rgb565.getPackedValueProperty(), rgb565.ToVector3().ToString());
        ctx.CheckNear(rgb565.ToVector3().Y, 32.0 / 63.0, 1e-6, "green is quantised to 6 bits (32/63)");

        // 5-5-5 colour and a 1-bit alpha: alpha is either fully on or off.
        const Bgra5551 rgba5551(orange);
        ctx.Print("Bgra5551 0x{:04X} -> {}", rgba5551.getPackedValueProperty(), rgba5551.ToVector4().ToString());
        ctx.Check(rgba5551.ToVector4().W == 1.0f, "an alpha of 0.6 rounds to the single alpha bit: 1");

        // 4 bits per channel: 16 levels each.
        const Bgra4444 rgba4444(orange);
        ctx.Print("Bgra4444 0x{:04X} -> {}", rgba4444.getPackedValueProperty(), rgba4444.ToVector4().ToString());
        ctx.CheckNear(rgba4444.ToVector4().W, 9.0 / 15.0, 1e-6, "alpha 0.6 becomes 9/15");

        // Alpha only, 8 bits (masks, fonts).
        const Alpha8 mask(0.6f);
        ctx.Print("Alpha8   0x{:02X} -> {}", mask.getPackedValueProperty(), mask.ToAlpha());
        ctx.Check(mask.getPackedValueProperty() == 153, "0.6 * 255 rounds to 153");
    }
}

CNA_DEMOS_CONSOLE(LowPrecisionColor, {
    .id = "graphics.packed_vector.low_precision_color",
    .title = "Compact colour formats",
    .description = "Packs one colour into Bgr565, Bgra5551, Bgra4444 and Alpha8 and shows how much precision each "
                   "keeps after unpacking.",
    .apis = {"Bgr565::Bgr565", "Bgr565::ToVector3", "Bgr565::PackedValue", "Bgra5551::Bgra5551", "Bgra5551::ToVector4",
             "Bgra4444::Bgra4444", "Bgra4444::ToVector4", "Alpha8::Alpha8", "Alpha8::ToAlpha", "Alpha8::PackedValue",
             "Bgra5551::PackedValue", "Bgra4444::PackedValue"},
    .tags = {"packed", "pixel-format", "texture", "quantization"},
})
