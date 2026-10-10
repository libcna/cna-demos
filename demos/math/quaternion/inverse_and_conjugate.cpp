// SPDX-License-Identifier: MS-PL
// Undoing a rotation: Quaternion::Inverse and Quaternion::Conjugate.
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
    }
}

CNA_DEMOS_CONSOLE(QuaternionInverse, {
    .id = "math.quaternion.inverse_and_conjugate",
    .title = "Inverse and conjugate",
    .description = "Undoes a rotation with Quaternion::Inverse and shows that Quaternion::Conjugate gives the same "
                   "result for unit quaternions.",
    .apis = {"Quaternion::Inverse", "Quaternion::Conjugate", "Quaternion::op_Multiply"},
    .tags = {"quaternion", "inverse"},
})
