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
    }
}

CNA_DEMOS_CONSOLE(QuaternionFromAxisAngle, {
    .id = "math.quaternion.from_axis_angle",
    .title = "Quaternion from axis and angle",
    .description = "Builds a rotation with Quaternion::CreateFromAxisAngle, shows how axis and angle are stored, "
                   "and compares it with Quaternion::Identity.",
    .apis = {"Quaternion::CreateFromAxisAngle", "Quaternion::Identity", "Quaternion::Length", "Quaternion::ToString"},
    .tags = {"quaternion", "rotation"},
})
