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

        // The same order rule applies to arbitrary, nonunit values; this also keeps the
        // component arithmetic visible instead of relying only on equivalent rotations.
        const Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
        const Quaternion b(-2.0f, 0.5f, 5.0f, -1.0f);
        const Quaternion expected(-17.5f, 11.0f, 12.5f, -18.0f);
        const Quaternion concatenated = Quaternion::Concatenate(a, b);
        ctx.Check(concatenated == expected,
                  "Concatenate preserves XNA's exact reversed Hamilton product for nonunit values");

        Quaternion output;
        Quaternion::Concatenate(a, b, output);
        ctx.Check(output == concatenated, "the Concatenate output overload matches the value overload");

        Quaternion multipliedOutput;
        Quaternion::Multiply(b, a, multipliedOutput);
        ctx.Check(Quaternion::Multiply(b, a) == expected && multipliedOutput == expected && b * a == expected,
                  "Multiply value, output and operator forms agree with Concatenate's reversed operands");

        ctx.Check(Quaternion::Concatenate(a, Quaternion::Identity) == a &&
                      Quaternion::Concatenate(Quaternion::Identity, a) == a &&
                      Quaternion::Multiply(a, Quaternion::Identity) == a &&
                      Quaternion::Multiply(Quaternion::Identity, a) == a,
                  "identity is neutral on either side of Concatenate and Multiply");

        const Quaternion scaledExpected(-2.5f, -5.0f, -7.5f, -10.0f);
        const Quaternion scaled = Quaternion::Multiply(a, -2.5f);
        ctx.Check(scaled == scaledExpected, "scalar Multiply scales all four components, including W");
        Quaternion::Multiply(a, -2.5f, output);
        ctx.Check(output == scaled && a * -2.5f == scaled,
                  "scalar Multiply's output and operator forms match its value overload");

        const Quaternion zero;
        ctx.Check(Quaternion::Multiply(a, 0.0f) == zero && Quaternion::Multiply(zero, a) == zero &&
                      Quaternion::Multiply(a, zero) == zero && Quaternion::Concatenate(zero, a) == zero &&
                      Quaternion::Concatenate(a, zero) == zero,
                  "zero scaling and zero operands produce the zero quaternion");
    }
}

CNA_DEMOS_CONSOLE(QuaternionConcatenate, {
    .id = "math.quaternion.concatenate",
    .title = "Combine rotations",
    .description = "Chains two rotations, proves Concatenate's reversed multiplication order on arbitrary values, "
                   "and distinguishes quaternion composition from scalar component scaling.",
    .apis = {"Quaternion::Concatenate", "Quaternion::Multiply", "Quaternion::op_Multiply"},
    .tags = {"quaternion", "rotation", "order"},
})
