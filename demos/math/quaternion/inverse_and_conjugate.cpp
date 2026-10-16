// SPDX-License-Identifier: MS-PL
// Undoing a rotation: Quaternion::Inverse and Quaternion::Conjugate.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionInverse(CnaDemos::DemoContext& ctx)
    {
        const Quaternion q = Quaternion::CreateFromYawPitchRoll(0.9f, 0.3f, -0.4f);

        const Quaternion inverse = Quaternion::Inverse(q);
        const Quaternion conjugate = Quaternion::Conjugate(q);
        ctx.Print("q           = {}", q.ToString());
        ctx.Print("Inverse(q)  = {}", inverse.ToString());
        ctx.Print("Conjugate(q)= {}", conjugate.ToString());

        // Rotating by q and then by its inverse returns the original vector.
        const Vector3 v(1.0f, 2.0f, 3.0f);
        const Vector3 back = Vector3::Transform(Vector3::Transform(v, q), inverse);
        ctx.CheckNear(Vector3::Distance(back, v), 0.0, 1e-5, "Inverse undoes the rotation");
        ctx.CheckNear(CnaDemos::RotationDifference(q * inverse, Quaternion::Identity), 0.0, 1e-6, "q * Inverse(q) is the identity");

        // For a unit quaternion the conjugate (negated X, Y, Z) is the inverse -- and cheaper.
        ctx.CheckNear(CnaDemos::RotationDifference(conjugate, inverse), 0.0, 1e-6, "Conjugate equals Inverse for a unit quaternion");

        Quaternion inPlace = q;
        inPlace.Conjugate();
        ctx.Check(inPlace == conjugate, "the member Conjugate() works in place");

        const Quaternion raw(1.0f, 2.0f, 3.0f, 4.0f);
        const Quaternion rawConjugate = Quaternion::Conjugate(raw);
        const Quaternion rawInverse = Quaternion::Inverse(raw);
        ctx.Check(rawConjugate == Quaternion(-1.0f, -2.0f, -3.0f, 4.0f) &&
                      rawConjugate != rawInverse,
                  "conjugate flips vector components but does not divide a nonunit value");
        ctx.CheckNear(rawInverse.X, -1.0 / 30.0, 1e-7, "nonunit inverse divides conjugate X by length squared");
        ctx.CheckNear(rawInverse.W, 4.0 / 30.0, 1e-7, "nonunit inverse divides conjugate W by length squared");

        Quaternion inverseByReference;
        Quaternion::Inverse(raw, inverseByReference);
        Quaternion conjugateByReference;
        Quaternion::Conjugate(raw, conjugateByReference);
        ctx.Check(inverseByReference == rawInverse && conjugateByReference == rawConjugate,
                  "the output inverse and conjugate overloads match their value forms");

        const Quaternion zeroInverse = Quaternion::Inverse(Quaternion());
        ctx.Check(std::isnan(zeroInverse.X) && std::isnan(zeroInverse.Y) &&
                      std::isnan(zeroInverse.Z) && std::isnan(zeroInverse.W),
                  "inverting zero exposes four NaN components");

        const Quaternion hugeInverse = Quaternion::Inverse(Quaternion(1e20f, 0.0f, 0.0f, 0.0f));
        ctx.Print("inverse of huge X = {}", hugeInverse.ToString());
        ctx.Check(hugeInverse.Y == 0.0f && hugeInverse.Z == 0.0f && hugeInverse.W == 0.0f &&
                      std::abs(hugeInverse.X + 1e-20f) < 1e-26f,
                  "XNA inverts a huge finite component without overflowing length squared");

        const Quaternion tinyInverse = Quaternion::Inverse(Quaternion(1e-20f, 0.0f, 0.0f, 0.0f));
        ctx.Print("inverse of tiny X = {}", tinyInverse.ToString());
        ctx.Check(tinyInverse.Y == 0.0f && tinyInverse.Z == 0.0f && tinyInverse.W == 0.0f &&
                      std::isfinite(tinyInverse.X) && std::abs((tinyInverse.X / -1e20f) - 1.0f) < 1e-5f,
                  "XNA inverts a tiny finite component without overflowing the reciprocal");
    }
}

CNA_DEMOS_CONSOLE(QuaternionInverse, {
    .id = "math.quaternion.inverse_and_conjugate",
    .title = "Inverse and conjugate",
    .description = "Compares inverse and conjugate for unit and nonunit values, including output overloads, zero "
                   "and extreme finite magnitude boundaries.",
    .apis = {"Quaternion::Inverse", "Quaternion::Conjugate", "Quaternion::op_Multiply"},
    .tags = {"quaternion", "inverse"},
    .restrictions = "CNA-DEMOS-ISSUE-119 records overflow and underflow for extreme finite inverses.",
    .knownIssue = "CNA-DEMOS-ISSUE-119",
})
