// SPDX-License-Identifier: MS-PL
// Interpolating orientations: Quaternion::Slerp versus Quaternion::Lerp.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
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

        // XNA takes the short representation path for exact antipodes too. The linear branch
        // subtracts the negative endpoint before normalization, avoiding a zero midpoint.
        const Quaternion antipode = -Quaternion::Identity;
        ctx.Check(Quaternion::Slerp(Quaternion::Identity, antipode, 0.5f) == Quaternion::Identity &&
                      Quaternion::Lerp(Quaternion::Identity, antipode, 0.5f) == Quaternion::Identity,
                  "exact antipodes interpolate through the first orientation representation");

        // Amount is not clamped: spherical interpolation extrapolates along the same great circle.
        const Quaternion quarterTurn = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::PiOver2);
        const Quaternion before = Quaternion::Slerp(Quaternion::Identity, quarterTurn, -0.5f);
        const Quaternion after = Quaternion::Slerp(Quaternion::Identity, quarterTurn, 1.5f);
        ctx.CheckNear(CnaDemos::RotationDifference(
                          before, Quaternion::CreateFromAxisAngle(Vector3::Up, -MathHelper::PiOver4)),
                      0.0, 1e-5, "Slerp extrapolates to -45 degrees before the interval");
        ctx.CheckNear(CnaDemos::RotationDifference(
                          after, Quaternion::CreateFromAxisAngle(Vector3::Up, 3.0f * MathHelper::PiOver4)),
                      0.0, 1e-5, "Slerp extrapolates to 135 degrees beyond the interval");

        const Quaternion lerpBefore = Quaternion::Lerp(Quaternion::Identity, quarterTurn, -0.5f);
        const Quaternion lerpAfter = Quaternion::Lerp(Quaternion::Identity, quarterTurn, 1.5f);
        ctx.CheckNear(lerpBefore.Length(), 1.0, 1e-6, "Lerp normalizes negative-amount extrapolation");
        ctx.CheckNear(lerpAfter.Length(), 1.0, 1e-6, "Lerp normalizes beyond-one extrapolation");

        const Quaternion zero;
        ctx.Check(Quaternion::Slerp(zero, zero, 0.5f) == zero,
                  "Slerp of two zero quaternions remains zero");
        const Quaternion zeroLerp = Quaternion::Lerp(zero, zero, 0.5f);
        ctx.Check(std::isnan(zeroLerp.X) && std::isnan(zeroLerp.Y) &&
                      std::isnan(zeroLerp.Z) && std::isnan(zeroLerp.W),
                  "Lerp of two zero quaternions exposes normalization NaNs");

        const Quaternion halfZeroIdentity = Quaternion::Slerp(zero, Quaternion::Identity, 0.5f);
        ctx.Check(halfZeroIdentity.X == 0.0f && halfZeroIdentity.Y == 0.0f &&
                      halfZeroIdentity.Z == 0.0f &&
                      std::abs(halfZeroIdentity.W - std::sqrt(0.5f)) < 1e-6f,
                  "Slerp with a zero endpoint keeps XNA's nonunit half-way magnitude");

        Quaternion byReference;
        Quaternion::Slerp(Quaternion::Identity, quarterTurn, 1.5f, byReference);
        ctx.Check(byReference == after, "the by-reference Slerp overload matches extrapolation");
    }
}

CNA_DEMOS_CONSOLE(QuaternionSlerp, {
    .id = "math.quaternion.slerp",
    .title = "Spherical interpolation",
    .description = "Compares Slerp and normalized Lerp for ordinary turns, exact antipodes, unclamped "
                   "extrapolation and zero-quaternion boundaries.",
    .apis = {"Quaternion::Slerp", "Quaternion::Lerp"},
    .tags = {"quaternion", "interpolation", "animation"},
})
