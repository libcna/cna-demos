// SPDX-License-Identifier: MS-PL
// Higher-precision unsigned normalized formats: Rgba1010102, Rg32 and Rgba64.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rg32.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rgba1010102.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rgba64.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    void WideUnorm(CnaDemos::DemoContext& ctx)
    {
        const Vector4 value(0.1f, 0.2f, 0.3f, 0.4f);

        // 10 bits per colour channel, 2 bits of alpha: 32 bits total.
        const Rgba1010102 rgb10(value);
        ctx.Print("Rgba1010102 -> {}", rgb10.ToVector4().ToString());
        ctx.CheckNear(rgb10.ToVector4().X, 0.1, 1.0 / 1023.0, "10-bit channels are accurate to 1/1023");
        ctx.CheckNear(rgb10.ToVector4().W, 1.0 / 3.0, 1e-6, "the 2-bit alpha has only 4 levels (0.4 -> 1/3)");

        // Two 16-bit channels (e.g. a high-precision 2D lookup or motion vector).
        const Rg32 rg(Vector2(0.25f, 0.123456f));
        ctx.Print("Rg32        -> {}", rg.ToVector2().ToString());
        ctx.CheckNear(rg.ToVector2().Y, 0.123456, 1.0 / 65535.0, "16-bit channels are accurate to 1/65535");

        // Four 16-bit channels in 64 bits.
        const Rgba64 rgba64(value.X, value.Y, value.Z, value.W);
        ctx.Print("Rgba64      -> {} (0x{:016X})", rgba64.ToVector4().ToString(), rgba64.getPackedValueProperty());
        ctx.CheckNear(rgba64.ToVector4().W, 0.4, 1.0 / 65535.0, "Rgba64 keeps alpha precise too");
    }
}

CNA_DEMOS_CONSOLE(WideUnorm, {
    .id = "graphics.packed_vector.wide_unorm",
    .title = "High-precision normalized formats",
    .description = "Packs values into Rgba1010102, Rg32 and Rgba64 and compares their per-channel precision.",
    .apis = {"Rgba1010102::Rgba1010102", "Rgba1010102::ToVector4", "Rg32::Rg32", "Rg32::ToVector2", "Rgba64::Rgba64",
             "Rgba64::ToVector4", "Rgba64::PackedValue"},
    .tags = {"packed", "unorm", "precision"},
})
