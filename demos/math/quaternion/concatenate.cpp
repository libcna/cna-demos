// SPDX-License-Identifier: MS-PL
// Combining rotations: Quaternion::Concatenate and quaternion multiplication order.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionConcatenate(CnaDemos::DemoContext& ctx)
    {
        const Quaternion turnLeft = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::PiOver2);
        const Quaternion tiltUp = Quaternion::CreateFromAxisAngle(Vector3::Right, MathHelper::PiOver2);

        // Concatenate(first, second): apply "first", then "second".
        const Quaternion leftThenUp = Quaternion::Concatenate(turnLeft, tiltUp);
        const Vector3 result = Vector3::Transform(Vector3::Forward, leftThenUp);
        const Vector3 stepByStep = Vector3::Transform(Vector3::Transform(Vector3::Forward, turnLeft), tiltUp);
        ctx.Print("Forward, turned left then tilted up = {}", result.ToString());
        ctx.CheckNear(Vector3::Distance(result, stepByStep), 0.0, 1e-6, "Concatenate applies the first rotation first");

        // The order matters: rotations do not commute.
        const Vector3 other = Vector3::Transform(Vector3::Forward, Quaternion::Concatenate(tiltUp, turnLeft));
        ctx.Print("Forward, tilted up then turned left = {}", other.ToString());
        ctx.Check(Vector3::Distance(result, other) > 0.5f, "swapping the order gives a different result");

        // In operator form, the rotation applied first is on the right: second * first.
        ctx.CheckNear(CnaDemos::RotationDifference(tiltUp * turnLeft, leftThenUp), 0.0, 1e-6,
                      "Concatenate(a, b) equals b * a");
    }
}

CNA_DEMOS_CONSOLE(QuaternionConcatenate, {
    .id = "math.quaternion.concatenate",
    .title = "Combine rotations",
    .description = "Chains two rotations with Quaternion::Concatenate, shows that rotation order matters, and "
                   "relates it to quaternion multiplication order.",
    .apis = {"Quaternion::Concatenate", "Quaternion::Multiply", "Quaternion::op_Multiply"},
    .tags = {"quaternion", "rotation", "order"},
})
