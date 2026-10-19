// SPDX-License-Identifier: MS-PL
// Making a unit-length direction: instance Normalize() versus static Vector3::Normalize().
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Normalize(CnaDemos::DemoContext& ctx)
    {
        const Vector3 velocity(3.0f, 0.0f, 4.0f);

        // The static form returns a new vector and leaves its argument alone.
        const Vector3 direction = Vector3::Normalize(velocity);
        ctx.Print("Normalize({}) = {}  (length {})", velocity.ToString(), direction.ToString(), direction.Length());
        ctx.Check(velocity.LengthSquared() == 25.0f, "LengthSquared gives 3 squared plus 4 squared without a square root");
        ctx.CheckNear(direction.Length(), 1.0, 1e-6, "a normalized vector has length 1");
        ctx.Check(direction == Vector3(0.6f, 0.0f, 0.8f), "the direction of (3, 0, 4) is (0.6, 0, 0.8)");

        // The member form normalizes in place.
        Vector3 inPlace = velocity;
        inPlace.Normalize();
        ctx.Check(inPlace == direction, "member Normalize() gives the same result in place");

        Vector3 output;
        Vector3::Normalize(velocity, output);
        ctx.Check(output == direction, "the output Normalize form agrees for an ordinary vector");

        // Direction times speed rebuilds the original vector.
        ctx.Check(direction * velocity.Length() == velocity, "direction * length restores the vector");

        const Vector3 zeroStatic = Vector3::Normalize(Vector3::Zero);
        Vector3 zeroOutput;
        Vector3::Normalize(Vector3::Zero, zeroOutput);
        Vector3 zeroMember = Vector3::Zero;
        zeroMember.Normalize();
        ctx.Check(std::isnan(zeroStatic.X) && std::isnan(zeroStatic.Y) && std::isnan(zeroStatic.Z) &&
                      std::isnan(zeroOutput.X) && std::isnan(zeroOutput.Y) && std::isnan(zeroOutput.Z) &&
                      std::isnan(zeroMember.X) && std::isnan(zeroMember.Y) && std::isnan(zeroMember.Z),
                  "zero normalization exposes three NaNs in static, output and member forms");

        const Vector3 subnormalSquare(1e-20f, 0.0f, 0.0f);
        const Vector3 subnormalDirection = Vector3::Normalize(subnormalSquare);
        ctx.Check(subnormalSquare.LengthSquared() > 0.0f &&
                      std::abs((subnormalSquare.Length() / 1e-20f) - 1.0f) < 1e-5f &&
                      std::abs(subnormalDirection.X - 1.0f) < 1e-5f &&
                      subnormalDirection.Y == 0.0f && subnormalDirection.Z == 0.0f,
                  "a subnormal squared magnitude retains finite length and direction");

        const Vector3 huge(1e20f, 0.0f, 0.0f);
        ctx.Check(std::isfinite(huge.Length()) && std::abs((huge.Length() / 1e20f) - 1.0f) < 1e-5f,
                  "XNA keeps Vector3 Length finite when the squared magnitude overflows");
        ctx.Check(std::isinf(huge.LengthSquared()),
                  "LengthSquared exposes the huge component's overflow");
        const Vector3 hugeStatic = Vector3::Normalize(huge);
        Vector3 hugeOutput;
        Vector3::Normalize(huge, hugeOutput);
        Vector3 hugeMember = huge;
        hugeMember.Normalize();
        ctx.Check(hugeStatic == Vector3::UnitX && hugeOutput == Vector3::UnitX &&
                      hugeMember == Vector3::UnitX,
                  "XNA normalizes a huge finite Vector3 in every public form");

        const Vector3 tiny(1e-30f, 0.0f, 0.0f);
        ctx.Check(std::isfinite(tiny.Length()) && std::abs((tiny.Length() / 1e-30f) - 1.0f) < 1e-5f,
                  "XNA keeps Vector3 Length finite when the squared magnitude underflows");
        ctx.Check(tiny.LengthSquared() == 0.0f,
                  "LengthSquared exposes the tiny component's underflow");
        const Vector3 tinyStatic = Vector3::Normalize(tiny);
        Vector3 tinyOutput;
        Vector3::Normalize(tiny, tinyOutput);
        Vector3 tinyMember = tiny;
        tinyMember.Normalize();
        ctx.Check(tinyStatic == Vector3::UnitX && tinyOutput == Vector3::UnitX &&
                      tinyMember == Vector3::UnitX,
                  "XNA normalizes a tiny finite Vector3 in every public form");
    }
}

CNA_DEMOS_CONSOLE(Vector3Normalize, {
    .id = "math.vector3.normalize",
    .title = "Normalize a Vector3",
    .description = "Compares Vector3 length and member, static and output normalization for ordinary, zero and "
                   "extreme finite magnitudes.",
    .apis = {"Vector3::Normalize", "Vector3::Length", "Vector3::LengthSquared"},
    .tags = {"vector", "direction", "unit-vector", "precision"},
})
