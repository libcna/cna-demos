// SPDX-License-Identifier: MS-PL
// Interpolating orientations: Quaternion::Slerp versus Quaternion::Lerp.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    float AngleBetween(Vector3 a, Vector3 b)
    {
        return MathHelper::ToDegrees(std::acos(MathHelper::Clamp(Vector3::Dot(a, b), -1.0f, 1.0f)));
    }

    void QuaternionSlerp(CnaDemos::DemoContext& ctx)
    {
        const Quaternion start = Quaternion::Identity;
        const Quaternion end = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::ToRadians(160.0f));

        ctx.Print("amount  Slerp angle  Lerp angle");
        for (const float amount : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f})
        {
            const Vector3 bySlerp = Vector3::Transform(Vector3::Forward, Quaternion::Slerp(start, end, amount));
            const Vector3 byLerp = Vector3::Transform(Vector3::Forward, Quaternion::Lerp(start, end, amount));
            ctx.Print("{:6.2f} {:12.2f} {:11.2f}", amount, AngleBetween(Vector3::Forward, bySlerp),
                      AngleBetween(Vector3::Forward, byLerp));
        }

        // Slerp turns at a constant rate: a quarter of the time gives a quarter of the angle.
        const Vector3 quarter = Vector3::Transform(Vector3::Forward, Quaternion::Slerp(start, end, 0.25f));
        ctx.CheckNear(AngleBetween(Vector3::Forward, quarter), 40.0, 1e-2, "Slerp at 0.25 has turned 40 of 160 degrees");
        // Lerp (normalized) follows the same path but speeds up in the middle.
        const Vector3 lerpQuarter = Vector3::Transform(Vector3::Forward, Quaternion::Lerp(start, end, 0.25f));
        ctx.Check(AngleBetween(Vector3::Forward, lerpQuarter) < 39.0f, "Lerp lags behind at 0.25");
        ctx.CheckNear(Quaternion::Lerp(start, end, 0.25f).Length(), 1.0, 1e-5, "Lerp returns a normalized quaternion");
    }
}

CNA_DEMOS_CONSOLE(QuaternionSlerp, {
    .id = "math.quaternion.slerp",
    .title = "Spherical interpolation",
    .description = "Interpolates between two orientations with Quaternion::Slerp (constant turning speed) and "
                   "Quaternion::Lerp (cheaper, uneven speed) and prints the angle reached.",
    .apis = {"Quaternion::Slerp", "Quaternion::Lerp"},
    .tags = {"quaternion", "interpolation", "animation"},
})
