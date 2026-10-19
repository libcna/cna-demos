// SPDX-License-Identifier: MS-PL
// Vector4 magnitude, distance and normalization across ordinary, zero and extreme finite inputs.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using Microsoft::Xna::Framework::Vector4;

namespace
{
    void Vector4MagnitudeRange(CnaDemos::DemoContext& ctx)
    {
        const Vector4 ordinary(1.0f, 2.0f, 2.0f, 4.0f);
        const Vector4 ordinaryDirection = ordinary / 5.0f;
        Vector4 ordinaryOutput;
        Vector4::Normalize(ordinary, ordinaryOutput);
        Vector4 ordinaryMember = ordinary;
        ordinaryMember.Normalize();
        ctx.Check(ordinary.Length() == 5.0f && ordinary.LengthSquared() == 25.0f &&
                      Vector4::Normalize(ordinary) == ordinaryDirection &&
                      ordinaryOutput == ordinaryDirection && ordinaryMember == ordinaryDirection,
                  "ordinary length and every normalization form agree");

        float ordinaryDistanceOutput = 0.0f;
        Vector4::Distance(Vector4::Zero, ordinary, ordinaryDistanceOutput);
        ctx.Check(Vector4::Distance(Vector4::Zero, ordinary) == 5.0f &&
                      ordinaryDistanceOutput == 5.0f &&
                      Vector4::DistanceSquared(Vector4::Zero, ordinary) == 25.0f,
                  "ordinary value/output Distance and DistanceSquared agree");

        const Vector4 zeroStatic = Vector4::Normalize(Vector4::Zero);
        Vector4 zeroOutput;
        Vector4::Normalize(Vector4::Zero, zeroOutput);
        Vector4 zeroMember = Vector4::Zero;
        zeroMember.Normalize();
        ctx.Check(std::isnan(zeroStatic.X) && std::isnan(zeroStatic.Y) &&
                      std::isnan(zeroStatic.Z) && std::isnan(zeroStatic.W) &&
                      std::isnan(zeroOutput.X) && std::isnan(zeroOutput.Y) &&
                      std::isnan(zeroOutput.Z) && std::isnan(zeroOutput.W) &&
                      std::isnan(zeroMember.X) && std::isnan(zeroMember.Y) &&
                      std::isnan(zeroMember.Z) && std::isnan(zeroMember.W),
                  "zero normalization exposes four NaNs in static, output and member forms");

        const Vector4 subnormalSquare(1e-20f, 0.0f, 0.0f, 0.0f);
        const Vector4 subnormalDirection = Vector4::Normalize(subnormalSquare);
        ctx.Check(subnormalSquare.LengthSquared() > 0.0f &&
                      std::abs((subnormalSquare.Length() / 1e-20f) - 1.0f) < 1e-5f &&
                      std::abs(subnormalDirection.X - 1.0f) < 1e-5f &&
                      subnormalDirection.Y == 0.0f && subnormalDirection.Z == 0.0f &&
                      subnormalDirection.W == 0.0f,
                  "a subnormal squared magnitude retains finite length and direction");

        const Vector4 huge(1e20f, 0.0f, 0.0f, 0.0f);
        ctx.Check(std::isfinite(huge.Length()) &&
                      std::abs((huge.Length() / 1e20f) - 1.0f) < 1e-5f,
                  "XNA keeps Vector4 Length finite when the squared magnitude overflows");
        ctx.Check(std::isinf(huge.LengthSquared()),
                  "LengthSquared exposes the huge component's overflow");
        const Vector4 hugeStatic = Vector4::Normalize(huge);
        Vector4 hugeOutput;
        Vector4::Normalize(huge, hugeOutput);
        Vector4 hugeMember = huge;
        hugeMember.Normalize();
        ctx.Check(hugeStatic == Vector4::UnitX && hugeOutput == Vector4::UnitX &&
                      hugeMember == Vector4::UnitX,
                  "XNA normalizes a huge finite Vector4 in every public form");

        const Vector4 tiny(1e-30f, 0.0f, 0.0f, 0.0f);
        ctx.Check(std::isfinite(tiny.Length()) &&
                      std::abs((tiny.Length() / 1e-30f) - 1.0f) < 1e-5f,
                  "XNA keeps Vector4 Length finite when the squared magnitude underflows");
        ctx.Check(tiny.LengthSquared() == 0.0f,
                  "LengthSquared exposes the tiny component's underflow");
        const Vector4 tinyStatic = Vector4::Normalize(tiny);
        Vector4 tinyOutput;
        Vector4::Normalize(tiny, tinyOutput);
        Vector4 tinyMember = tiny;
        tinyMember.Normalize();
        ctx.Check(tinyStatic == Vector4::UnitX && tinyOutput == Vector4::UnitX &&
                      tinyMember == Vector4::UnitX,
                  "XNA normalizes a tiny finite Vector4 in every public form");

        float hugeDistanceOutput = 0.0f;
        Vector4::Distance(Vector4::Zero, huge, hugeDistanceOutput);
        ctx.Check(std::isfinite(Vector4::Distance(Vector4::Zero, huge)) &&
                      std::abs((Vector4::Distance(Vector4::Zero, huge) / 1e20f) - 1.0f) < 1e-5f &&
                      hugeDistanceOutput == Vector4::Distance(Vector4::Zero, huge),
                  "XNA keeps value and output Distance finite across a huge displacement");
        ctx.Check(std::isinf(Vector4::DistanceSquared(Vector4::Zero, huge)),
                  "DistanceSquared exposes the huge displacement's overflow");

        ctx.Check(std::isfinite(Vector4::Distance(Vector4::Zero, tiny)) &&
                      std::abs((Vector4::Distance(Vector4::Zero, tiny) / 1e-30f) - 1.0f) < 1e-5f,
                  "XNA keeps Distance finite across a tiny displacement");
        ctx.Check(Vector4::DistanceSquared(Vector4::Zero, tiny) == 0.0f,
                  "DistanceSquared exposes the tiny displacement's underflow");
    }
}

CNA_DEMOS_CONSOLE(Vector4MagnitudeRange, {
    .id = "math.vector4.magnitude_range",
    .title = "Vector4 magnitude range",
    .description = "Compares Vector4 length, squared length, distance and member/static/output normalization for "
                   "ordinary, zero and extreme finite values.",
    .apis = {"Vector4::Length", "Vector4::LengthSquared", "Vector4::Normalize", "Vector4::Distance",
             "Vector4::DistanceSquared", "Vector4::Zero", "Vector4::UnitX"},
    .tags = {"vector", "magnitude", "distance", "precision"},
})
