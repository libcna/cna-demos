// SPDX-License-Identifier: MS-PL
// Color construction, mutable channels, value identity, hashing and text.
#include <any>
#include <cstdint>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void ColorConstruct(CnaDemos::DemoContext& ctx)
    {
        const Color zero;
        const Color fromInts(255, 128, 0);
        ctx.Check(zero == Color(0, 0, 0, 0) && fromInts == Color(255, 128, 0, 255),
                  "default construction is transparent black and three integers default alpha to 255");

        const Color float3(0.25f, 0.5f, 0.75f);
        const Color float4(0.25f, 0.5f, 0.75f, 0.125f);
        const Color vector3(Vector3(0.25f, 0.5f, 0.75f));
        const Color vector4(Vector4(0.25f, 0.5f, 0.75f, 0.125f));
        ctx.Check(float3 == Color(64, 128, 191, 255) && vector3 == float3 &&
                      float4 == Color(64, 128, 191, 32) && vector4 == float4,
                  "float and vector constructors clamp and round channels to the nearest even byte");

        const float infinity = std::numeric_limits<float>::infinity();
        const float nan = std::numeric_limits<float>::quiet_NaN();
        ctx.Check(Color(nan, infinity, -infinity, 1.0f) == Color(0, 255, 0, 255),
                  "float construction maps NaN and negative infinity to zero and positive infinity to 255");

        const Color clampedInts(300, -20, 128, 999);
        const Color clampedFloats(Vector3(2.0f, -1.0f, 0.5f));
        ctx.Check(clampedInts == Color(255, 0, 128, 255), "integer channels clamp to 0..255");
        ctx.Check(clampedFloats == Color(255, 0, 128, 255), "float channels clamp to 0..1");

        const Color original(1, 2, 3, 4);
        Color edited = original;
        edited.setRProperty(10);
        edited.setGProperty(20);
        edited.setBProperty(30);
        edited.setAProperty(40);
        ctx.Check(edited == Color(10, 20, 30, 40) && original == Color(1, 2, 3, 4),
                  "all four mutable channel properties preserve the other bits and copy value semantics");

        const Color equal(10, 20, 30, 40);
        ctx.Check(edited.Equals(equal) && edited.Equals(std::any{equal}) && edited == equal &&
                      !(edited != equal) && !edited.Equals(std::any{42}) &&
                      !edited.Equals(std::any{}),
                  "typed, boxed and operator equality agree and reject other boxed values");
        ctx.Check(edited != Color(10, 20, 30, 41) && edited != Color(10, 20, 31, 40) &&
                      edited != Color(10, 21, 30, 40) && edited != Color(11, 20, 30, 40),
                  "changing any channel makes a Color unequal");
        ctx.Check(edited.GetHashCode() == equal.GetHashCode() &&
                      static_cast<std::uint32_t>(edited.GetHashCode()) == edited.getPackedValueProperty(),
                  "equal colors hash equally and XNA's hash preserves the packed 32-bit pattern");

        ctx.Print("edited color = {}", edited.ToString());
        ctx.Check(edited.ToString() == "{R:10 G:20 B:30 A:40}",
                  "ToString uses XNA's exact channel representation");
    }
}

CNA_DEMOS_CONSOLE(ColorConstruct, {
    .id = "math.color.construct",
    .title = "Construct colours",
    .description = "Constructs Color values from every scalar/vector form, checks clamping and rounding including "
                   "nonfinite inputs, mutates all channel properties, and proves identity, hashing and exact text.",
    .apis = {"Color::Color", "Color::R", "Color::G", "Color::B", "Color::A", "Color::Equals",
             "Color::GetHashCode", "Color::op_Equality", "Color::op_Inequality", "Color::ToString"},
    .tags = {"color", "rgba", "value-semantics", "formatting"},
})
