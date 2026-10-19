// SPDX-License-Identifier: MS-PL
// Vector2 magnitude and normalization across ordinary, zero and extreme finite inputs.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using Microsoft::Xna::Framework::Vector2;

namespace
{
    void Vector2Length(CnaDemos::DemoContext& ctx)
    {
        const Vector2 v(3.0f, 4.0f);
        ctx.Check(v.X == 3.0f && v.Y == 4.0f, "the constructor preserves X and Y");

        const float length = v.Length();                // sqrt(3*3 + 4*4)
        const float lengthSquared = v.LengthSquared();  // 3*3 + 4*4, no square root

        ctx.Print("Vector2(3, 4).Length()        = {}", length);
        ctx.Print("Vector2(3, 4).LengthSquared() = {}", lengthSquared);

        ctx.Check(length == 5.0f, "Length of (3, 4) is 5");
        ctx.Check(lengthSquared == 25.0f, "LengthSquared of (3, 4) is 25");
        ctx.Check(Vector2::Zero.Length() == 0.0f, "Length of Vector2::Zero is 0");
        ctx.Check(Vector2::One == Vector2::UnitX + Vector2::UnitY &&
                      Vector2::UnitX.LengthSquared() == 1.0f && Vector2::UnitY.LengthSquared() == 1.0f,
                  "One is the sum of the two unit axes");

        const Vector2 scalar(-2.5f);
        ctx.Check(scalar == Vector2(-2.5f, -2.5f),
                  "the scalar constructor copies its value to both components");

        const Vector2 ordinaryStatic = Vector2::Normalize(v);
        Vector2 ordinaryOutput;
        Vector2::Normalize(v, ordinaryOutput);
        Vector2 ordinaryMember = v;
        ordinaryMember.Normalize();
        ctx.Check(ordinaryStatic == Vector2(0.6f, 0.8f) && ordinaryOutput == ordinaryStatic &&
                      ordinaryMember == ordinaryStatic,
                  "static, output and member normalization agree for (3, 4)");

        const Vector2 zeroStatic = Vector2::Normalize(Vector2::Zero);
        Vector2 zeroOutput;
        Vector2::Normalize(Vector2::Zero, zeroOutput);
        Vector2 zeroMember = Vector2::Zero;
        zeroMember.Normalize();
        ctx.Check(std::isnan(zeroStatic.X) && std::isnan(zeroStatic.Y) &&
                      std::isnan(zeroOutput.X) && std::isnan(zeroOutput.Y) &&
                      std::isnan(zeroMember.X) && std::isnan(zeroMember.Y),
                  "zero normalization exposes two NaNs in static, output and member forms");

        const Vector2 huge(1e20f, 0.0f);
        ctx.Check(std::isfinite(huge.Length()) && std::abs((huge.Length() / 1e20f) - 1.0f) < 1e-5f,
                  "XNA keeps Vector2 Length finite when the squared magnitude overflows");
        ctx.Check(std::isinf(huge.LengthSquared()),
                  "LengthSquared exposes the huge component's overflow");

        const Vector2 tiny(1e-30f, 0.0f);
        ctx.Check(std::isfinite(tiny.Length()) && std::abs((tiny.Length() / 1e-30f) - 1.0f) < 1e-5f,
                  "XNA keeps Vector2 Length finite when the squared magnitude underflows");
        ctx.Check(tiny.LengthSquared() == 0.0f,
                  "LengthSquared exposes the tiny component's underflow");

        const Vector2 hugeStatic = Vector2::Normalize(huge);
        Vector2 hugeOutput;
        Vector2::Normalize(huge, hugeOutput);
        Vector2 hugeMember = huge;
        hugeMember.Normalize();
        ctx.Check(hugeStatic == Vector2::UnitX && hugeOutput == Vector2::UnitX &&
                      hugeMember == Vector2::UnitX,
                  "XNA normalizes a huge finite Vector2 in every public form");

        const Vector2 tinyStatic = Vector2::Normalize(tiny);
        Vector2 tinyOutput;
        Vector2::Normalize(tiny, tinyOutput);
        Vector2 tinyMember = tiny;
        tinyMember.Normalize();
        ctx.Check(tinyStatic == Vector2::UnitX && tinyOutput == Vector2::UnitX &&
                      tinyMember == Vector2::UnitX,
                  "XNA normalizes a tiny finite Vector2 in every public form");

        float hugeDistanceOutput = 0.0f;
        Vector2::Distance(Vector2::Zero, huge, hugeDistanceOutput);
        ctx.Check(std::isfinite(Vector2::Distance(Vector2::Zero, huge)) &&
                      std::abs((Vector2::Distance(Vector2::Zero, huge) / 1e20f) - 1.0f) < 1e-5f &&
                      hugeDistanceOutput == Vector2::Distance(Vector2::Zero, huge),
                  "XNA keeps value and output Distance finite across a huge displacement");
        ctx.Check(std::isinf(Vector2::DistanceSquared(Vector2::Zero, huge)),
                  "DistanceSquared exposes the huge displacement's overflow");

        ctx.Check(std::isfinite(Vector2::Distance(Vector2::Zero, tiny)) &&
                      std::abs((Vector2::Distance(Vector2::Zero, tiny) / 1e-30f) - 1.0f) < 1e-5f,
                  "XNA keeps Distance finite across a tiny displacement");
        ctx.Check(Vector2::DistanceSquared(Vector2::Zero, tiny) == 0.0f,
                  "DistanceSquared exposes the tiny displacement's underflow");
    }
}

CNA_DEMOS_CONSOLE(Vector2Length, {
    .id = "math.vector2.length",
    .title = "Vector2 length",
    .description = "Compares Vector2 length, squared length, distance and normalization for ordinary, zero and "
                   "extreme finite values.",
    .apis = {"Vector2::Vector2", "Vector2::Length", "Vector2::LengthSquared", "Vector2::Zero",
             "Vector2::One", "Vector2::UnitX", "Vector2::UnitY", "Vector2::X", "Vector2::Y",
             "Vector2::Normalize", "Vector2::Distance", "Vector2::DistanceSquared"},
    .tags = {"vector", "magnitude", "precision"},
})
