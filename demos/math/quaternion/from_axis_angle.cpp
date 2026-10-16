// SPDX-License-Identifier: MS-PL
// A rotation as a quaternion: Quaternion::CreateFromAxisAngle and Quaternion::Identity.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionFromAxisAngle(CnaDemos::DemoContext& ctx)
    {
        const float angle = MathHelper::ToRadians(60.0f);
        const Quaternion q = Quaternion::CreateFromAxisAngle(Vector3::Up, angle);

        // (X, Y, Z) = axis * sin(angle / 2), W = cos(angle / 2).
        ctx.Print("CreateFromAxisAngle(Up, 60 deg) = {}", q.ToString());
        ctx.CheckNear(q.Y, std::sin(angle / 2.0f), 1e-6, "Y holds sin(angle / 2) along the axis");
        ctx.CheckNear(q.W, std::cos(angle / 2.0f), 1e-6, "W holds cos(angle / 2)");
        ctx.CheckNear(q.Length(), 1.0, 1e-6, "a rotation quaternion has length 1");

        // Identity is "no rotation".
        ctx.Print("Quaternion::Identity = {}", Quaternion::Identity.ToString());
        ctx.Check(Vector3::Transform(Vector3::Forward, Quaternion::Identity) == Vector3::Forward,
                  "the identity quaternion leaves vectors unchanged");
        ctx.Check(Quaternion::Identity == Quaternion(0.0f, 0.0f, 0.0f, 1.0f), "Identity is (0, 0, 0, 1)");

        // XNA consumes the axis verbatim. Callers that need a rotation quaternion must normalize
        // it themselves; scaled and zero axes deliberately produce nonunit values.
        const Vector3 scaledAxis(0.0f, 2.0f, 0.0f);
        const Quaternion scaled = Quaternion::CreateFromAxisAngle(scaledAxis, angle);
        ctx.CheckNear(scaled.Y, 1.0, 1e-6, "a doubled axis doubles the sine-weighted vector component");
        ctx.CheckNear(scaled.Length(), std::sqrt(1.75), 1e-6, "a nonunit axis produces a nonunit quaternion");

        const Quaternion zeroAxis = Quaternion::CreateFromAxisAngle(Vector3::Zero, angle);
        ctx.Check(zeroAxis.X == 0.0f && zeroAxis.Y == 0.0f && zeroAxis.Z == 0.0f &&
                      std::abs(zeroAxis.W - std::cos(angle / 2.0f)) < 1e-6f,
                  "a zero axis retains only cosine in W");

        ctx.Check(Quaternion::CreateFromAxisAngle(Vector3(2.0f, 3.0f, 4.0f), 0.0f) == Quaternion::Identity,
                  "a zero angle produces identity regardless of axis magnitude");

        const Quaternion nanAngle = Quaternion::CreateFromAxisAngle(Vector3::Up, NAN);
        ctx.Check(std::isnan(nanAngle.X) && std::isnan(nanAngle.Y) &&
                      std::isnan(nanAngle.Z) && std::isnan(nanAngle.W),
                  "a NaN angle propagates to all four components");

        Quaternion byReference;
        Quaternion::CreateFromAxisAngle(scaledAxis, angle, byReference);
        ctx.Check(byReference == scaled, "the output overload preserves the unnormalized axis result");
    }
}

CNA_DEMOS_CONSOLE(QuaternionFromAxisAngle, {
    .id = "math.quaternion.from_axis_angle",
    .title = "Quaternion from axis and angle",
    .description = "Builds axis-angle quaternions from unit, scaled, zero and NaN inputs and demonstrates that "
                   "XNA consumes the axis verbatim through value and output overloads.",
    .apis = {"Quaternion::CreateFromAxisAngle", "Quaternion::Identity", "Quaternion::Length", "Quaternion::ToString"},
    .tags = {"quaternion", "rotation"},
})
