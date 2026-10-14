// SPDX-License-Identifier: MS-PL
// 16-bit and 8-bit colour formats: Bgr565, Bgra5551, Bgra4444 and Alpha8 trade precision for size.
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Alpha8.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgr565.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra4444.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra5551.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/IPackedVector.hpp"
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
        Bgr565 rgb565Copy;
        rgb565Copy.setPackedValueProperty(rgb565.getPackedValueProperty());
        ctx.Check(rgb565.getPackedValueProperty() == 0xFC00u && rgb565Copy == rgb565 &&
                      !(rgb565Copy != rgb565) && rgb565.Equals(rgb565Copy) && rgb565.Equals(std::any(rgb565Copy)) &&
                      !rgb565.Equals(std::any(orange)) && rgb565Copy.GetHashCode() == rgb565.GetHashCode() &&
                      rgb565.ToString() == "FC00",
                  "Bgr565 preserves its packed value, equality, hash and four-digit text");
        Bgr565 rgb565Interface;
        IPackedVector& packedRgb565 = rgb565Interface;
        packedRgb565.PackFromVector4(orange);
        ctx.Check(packedRgb565.ToVector4() == Vector4(rgb565.ToVector3(), 1.0f),
                  "Bgr565's IPackedVector path consumes RGB and supplies opaque alpha");

        // 5-5-5 colour and a 1-bit alpha: alpha is either fully on or off.
        const Bgra5551 rgba5551(orange);
        ctx.Print("Bgra5551 0x{:04X} -> {}", rgba5551.getPackedValueProperty(), rgba5551.ToVector4().ToString());
        ctx.Check(rgba5551.ToVector4().W == 1.0f, "an alpha of 0.6 rounds to the single alpha bit: 1");
        Bgra5551 rgba5551Copy;
        rgba5551Copy.setPackedValueProperty(rgba5551.getPackedValueProperty());
        ctx.Check(rgba5551.getPackedValueProperty() == 0xFE00u && rgba5551Copy == rgba5551 &&
                      !(rgba5551Copy != rgba5551) && rgba5551.Equals(rgba5551Copy) &&
                      rgba5551.Equals(std::any(rgba5551Copy)) && !rgba5551.Equals(std::any(rgb565)) &&
                      rgba5551Copy.GetHashCode() == rgba5551.GetHashCode() && rgba5551.ToString() == "FE00",
                  "Bgra5551 preserves its packed value, equality, hash and four-digit text");
        Bgra5551 rgba5551Interface;
        IPackedVector& packedRgba5551 = rgba5551Interface;
        packedRgba5551.PackFromVector4(orange);
        ctx.Check(packedRgba5551.ToVector4() == rgba5551.ToVector4(),
                  "Bgra5551's IPackedVector path preserves the quantized channels");

        // 4 bits per channel: 16 levels each.
        const Bgra4444 rgba4444(orange);
        ctx.Print("Bgra4444 0x{:04X} -> {}", rgba4444.getPackedValueProperty(), rgba4444.ToVector4().ToString());
        ctx.CheckNear(rgba4444.ToVector4().W, 9.0 / 15.0, 1e-6, "alpha 0.6 becomes 9/15");
        Bgra4444 rgba4444Copy;
        rgba4444Copy.setPackedValueProperty(rgba4444.getPackedValueProperty());
        ctx.Check(rgba4444.getPackedValueProperty() == 0x9F80u && rgba4444Copy == rgba4444 &&
                      !(rgba4444Copy != rgba4444) && rgba4444.Equals(rgba4444Copy) &&
                      rgba4444.Equals(std::any(rgba4444Copy)) && !rgba4444.Equals(std::any(rgba5551)) &&
                      rgba4444Copy.GetHashCode() == rgba4444.GetHashCode() && rgba4444.ToString() == "9F80",
                  "Bgra4444 preserves its packed value, equality, hash and four-digit text");
        Bgra4444 rgba4444Interface;
        IPackedVector& packedRgba4444 = rgba4444Interface;
        packedRgba4444.PackFromVector4(orange);
        ctx.Check(packedRgba4444.ToVector4() == rgba4444.ToVector4(),
                  "Bgra4444's IPackedVector path preserves the quantized channels");

        // Alpha only, 8 bits (masks, fonts).
        const Alpha8 mask(0.6f);
        ctx.Print("Alpha8   0x{:02X} -> {}", mask.getPackedValueProperty(), mask.ToAlpha());
        ctx.Check(mask.getPackedValueProperty() == 153, "0.6 * 255 rounds to 153");
        Alpha8 maskCopy;
        maskCopy.setPackedValueProperty(mask.getPackedValueProperty());
        ctx.Check(maskCopy == mask && !(maskCopy != mask) && mask.Equals(maskCopy) &&
                      mask.Equals(std::any(maskCopy)) && !mask.Equals(std::any(rgba4444)) &&
                      maskCopy.GetHashCode() == mask.GetHashCode() && mask.ToString() == "99",
                  "Alpha8 preserves its packed value, equality, hash and two-digit text");
        Alpha8 maskInterface;
        IPackedVector& packedMask = maskInterface;
        packedMask.PackFromVector4(orange);
        ctx.Check(packedMask.ToVector4() == Vector4(0.0f, 0.0f, 0.0f, mask.ToAlpha()),
                  "Alpha8's IPackedVector path consumes alpha and supplies zero RGB");
    }
}

CNA_DEMOS_CONSOLE(LowPrecisionColor, {
    .id = "graphics.packed_vector.low_precision_color",
    .title = "Compact colour formats",
    .description = "Packs one colour into Bgr565, Bgra5551, Bgra4444 and Alpha8 and shows how much precision each "
                   "keeps after unpacking.",
    .apis = {"Bgr565::Bgr565", "Bgr565::ToVector3", "Bgr565::PackedValue", "Bgr565::Equals",
             "Bgr565::GetHashCode", "Bgr565::op_Equality", "Bgr565::op_Inequality", "Bgr565::ToString",
             "Bgr565::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "Bgr565::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#ToVector4",
             "Bgra5551::Bgra5551", "Bgra5551::ToVector4", "Bgra5551::PackedValue", "Bgra5551::Equals",
             "Bgra5551::GetHashCode", "Bgra5551::op_Equality", "Bgra5551::op_Inequality", "Bgra5551::ToString",
             "Bgra5551::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "Bgra4444::Bgra4444", "Bgra4444::ToVector4", "Bgra4444::PackedValue", "Bgra4444::Equals",
             "Bgra4444::GetHashCode", "Bgra4444::op_Equality", "Bgra4444::op_Inequality", "Bgra4444::ToString",
             "Bgra4444::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "Alpha8::Alpha8", "Alpha8::ToAlpha", "Alpha8::PackedValue", "Alpha8::Equals",
             "Alpha8::GetHashCode", "Alpha8::op_Equality", "Alpha8::op_Inequality", "Alpha8::ToString",
             "Alpha8::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#PackFromVector4",
             "Alpha8::Microsoft#Xna#Framework#Graphics#PackedVector#IPackedVector#ToVector4"},
    .tags = {"packed", "pixel-format", "texture", "quantization"},
})
