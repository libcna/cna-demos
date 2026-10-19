// SPDX-License-Identifier: MS-PL
// Equal orientations can have opposite quaternion components.
#include <algorithm>
#include <any>
#include <cmath>
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    float MaxComponentDifference(const Quaternion& a, const Quaternion& b)
    {
        return std::max(std::max(std::abs(a.X - b.X), std::abs(a.Y - b.Y)),
                        std::max(std::abs(a.Z - b.Z), std::abs(a.W - b.W)));
    }

    void DoubleCover(CnaDemos::DemoContext& ctx)
    {
        const Quaternion a = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::ToRadians(10.0f));
        const Quaternion positiveB = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::ToRadians(30.0f));
        const Quaternion b = Quaternion::Negate(positiveB);
        ctx.Check(b != positiveB && b == -positiveB, "opposite components are unequal values");
        const Quaternion samePositiveB(positiveB.X, positiveB.Y, positiveB.Z, positiveB.W);
        ctx.Check(positiveB.Equals(samePositiveB) && positiveB.Equals(std::any{samePositiveB}) &&
                      !positiveB.Equals(std::any{42}) && !positiveB.Equals(std::any{}) &&
                      positiveB.GetHashCode() == samePositiveB.GetHashCode(),
                  "component construction, typed/boxed equality and hashing agree");
        const Quaternion ordered(1.0f, 2.0f, 3.0f, 4.0f);
        ctx.Check(ordered.X == 1.0f && ordered.Y == 2.0f && ordered.Z == 3.0f && ordered.W == 4.0f,
                  "the component constructor preserves X, Y, Z and W in order");
        ctx.CheckNear(Quaternion::Dot(b, positiveB), -1.0, 1e-6, "antipodal unit quaternions have dot -1");
        ctx.CheckNear(Vector3::Distance(Vector3::Transform(Vector3::Forward, b),
                                       Vector3::Transform(Vector3::Forward, positiveB)),
                      0.0, 1e-6, "q and -q rotate a vector identically");
        const Quaternion zero = Quaternion::Add(b, positiveB);
        ctx.CheckNear(zero.LengthSquared(), 0.0, 1e-6, "adding antipodes gives zero: do not normalize it");
        ctx.Check(b + positiveB == zero, "the addition operator matches component-wise Add");

        // For UNIT quaternions, use abs(dot) to ignore the representation's sign.
        const float dot = Quaternion::Dot(a, b);
        const float degrees = MathHelper::ToDegrees(2.0f * std::acos(MathHelper::Clamp(std::abs(dot), 0.0f, 1.0f)));
        ctx.Print("Orientation separation: {:.3f} degrees (signed dot {:.6f})", degrees, dot);
        ctx.CheckNear(degrees, 20.0, 1e-3, "orientation distance uses the absolute dot product");

        const Quaternion naive = Quaternion::Normalize(Quaternion::Add(a, b));
        const Quaternion alignedB = dot < 0.0f ? Quaternion::Negate(b) : b;
        const Quaternion blend = Quaternion::Normalize(Quaternion::Add(a, alignedB));
        const Quaternion spherical = Quaternion::Slerp(a, b, 0.5f);
        const Vector3 expected = Vector3::Transform(Vector3::Forward,
            Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::ToRadians(20.0f)));
        // Comparing transformed vectors avoids acos's poor precision near dot == 1.
        ctx.CheckNear(Vector3::Distance(Vector3::Transform(Vector3::Forward, blend), expected),
                      0.0, 1e-5, "hemisphere-aligned normalized blending gives the 20 degree turn");
        ctx.CheckNear(Vector3::Distance(Vector3::Transform(Vector3::Forward, spherical), expected),
                      0.0, 1e-5, "Slerp takes the short path despite the negative dot");
        ctx.Check(Vector3::Distance(Vector3::Transform(Vector3::Forward, naive), expected) > 1.9f,
                  "naive blending takes the opposite orientation");

        // Subtract is component arithmetic, not a relative rotation. Divide is target * inverse(start).
        // Use different axes here: same-axis turns would hide a multiplication-order mistake.
        const Quaternion start = Quaternion::CreateFromYawPitchRoll(0.2f, -0.5f, 0.1f);
        const Quaternion target = Quaternion::CreateFromYawPitchRoll(-0.4f, 0.3f, 0.6f);
        const Quaternion componentDelta = Quaternion::Subtract(target, start);
        ctx.Check(componentDelta == target - start &&
                      componentDelta == Quaternion(target.X - start.X, target.Y - start.Y,
                                                   target.Z - start.Z, target.W - start.W),
                  "Subtract and operator - compute a component delta, not a relative turn");
        const Quaternion turn = Quaternion::Divide(target, start);
        ctx.Check(turn == target / start, "Divide and operator / agree");
        for (const Vector3 axis : {Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ})
            ctx.CheckNear(Vector3::Distance(Vector3::Transform(Vector3::Transform(axis, start), turn),
                                           Vector3::Transform(axis, target)),
                          0.0, 1e-5, "applying the relative turn after start reaches target");

        const Quaternion numerator(1.0f, 2.0f, 3.0f, 4.0f);
        const Quaternion divisor(-2.0f, 0.5f, 5.0f, -1.0f);
        const Quaternion quotient = Quaternion::Divide(numerator, divisor);
        const Quaternion expectedQuotient(-0.0495867729f, 0.23140496f, -0.909090936f, 0.3305785f);
        ctx.Check(MaxComponentDifference(quotient, expectedQuotient) < 1e-7f,
                  "a nonunit divisor produces XNA's ordered Hamilton quotient");

        Quaternion quotientOutput;
        Quaternion::Divide(numerator, divisor, quotientOutput);
        ctx.Check(MaxComponentDifference(quotientOutput, quotient) < 1e-7f,
                  "the Divide output overload matches the value result within XNA's one-ULP variation");
        ctx.Check(numerator / divisor == quotient &&
                      MaxComponentDifference(Quaternion::Multiply(numerator, Quaternion::Inverse(divisor)),
                                             quotient) < 1e-7f,
                  "operator division agrees and Divide is numerator times the divisor inverse");

        ctx.Check(Quaternion::Divide(numerator, Quaternion::Identity) == numerator &&
                      Quaternion::Divide(Quaternion::Identity, divisor) == Quaternion::Inverse(divisor),
                  "division by Identity preserves the numerator and Identity divided by q is Inverse(q)");

        const Quaternion zeroNumerator = Quaternion::Divide(Quaternion(), divisor);
        ctx.Check(zeroNumerator.X == 0.0f && zeroNumerator.Y == 0.0f &&
                      zeroNumerator.Z == 0.0f && zeroNumerator.W == 0.0f,
                  "a zero numerator divided by a finite nonzero value remains zero");

        const Quaternion zeroDivisor = Quaternion::Divide(numerator, Quaternion());
        ctx.Check(std::isnan(zeroDivisor.X) && std::isnan(zeroDivisor.Y) &&
                      std::isnan(zeroDivisor.Z) && std::isnan(zeroDivisor.W),
                  "division by the zero quaternion exposes four NaN components");

        const Quaternion huge(1e20f, 0.0f, 0.0f, 0.0f);
        const Quaternion hugeQuotient = Quaternion::Divide(Quaternion::Identity, huge);
        Quaternion hugeOutput;
        Quaternion::Divide(Quaternion::Identity, huge, hugeOutput);
        ctx.Check(hugeQuotient.Y == 0.0f && hugeQuotient.Z == 0.0f && hugeQuotient.W == 0.0f &&
                      std::isfinite(hugeQuotient.X) &&
                      std::abs((hugeQuotient.X / -1e-20f) - 1.0f) < 1e-5f &&
                      hugeOutput == hugeQuotient,
                  "XNA divides by a huge finite component without overflowing its squared norm");

        const Quaternion tiny(1e-20f, 0.0f, 0.0f, 0.0f);
        const Quaternion tinyQuotient = Quaternion::Divide(Quaternion::Identity, tiny);
        Quaternion tinyOutput;
        Quaternion::Divide(Quaternion::Identity, tiny, tinyOutput);
        ctx.Check(tinyQuotient.Y == 0.0f && tinyQuotient.Z == 0.0f && tinyQuotient.W == 0.0f &&
                      std::isfinite(tinyQuotient.X) &&
                      std::abs((tinyQuotient.X / -1e20f) - 1.0f) < 1e-5f &&
                      tinyOutput == tinyQuotient,
                  "XNA divides by a tiny finite component without overflowing the reciprocal");
    }
}

CNA_DEMOS_CONSOLE(DoubleCover, {
    .id = "math.quaternion.double_cover",
    .title = "Two quaternion values, one orientation",
    .description = "Shows q and -q as the same rotation, aligns signs before blending, and exercises ordered "
                   "Quaternion division with nonunit, zero and extreme finite divisors.",
    .apis = {"Quaternion::Quaternion", "Quaternion::Negate", "Quaternion::op_UnaryNegation", "Quaternion::Dot",
             "Quaternion::Add", "Quaternion::Subtract", "Quaternion::Normalize", "Quaternion::LengthSquared",
             "Quaternion::Equals", "Quaternion::GetHashCode", "Quaternion::op_Addition",
             "Quaternion::op_Equality", "Quaternion::op_Inequality", "Quaternion::op_Subtraction",
             "Quaternion::Slerp", "Quaternion::Divide", "Quaternion::op_Division",
             "Quaternion::X", "Quaternion::Y", "Quaternion::Z", "Quaternion::W"},
    .tags = {"quaternion", "rotation", "interpolation", "precision"},
})
