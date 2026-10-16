// SPDX-License-Identifier: MS-PL
// Quaternion::CreateFromYawPitchRoll and conversion between quaternions and matrices.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionFromYawPitchRoll(CnaDemos::DemoContext& ctx)
    {
        const Quaternion q = Quaternion::CreateFromYawPitchRoll(0.6f, -0.2f, 0.3f);
        const Matrix m = Matrix::CreateFromYawPitchRoll(0.6f, -0.2f, 0.3f);

        // Quaternion -> matrix and matrix -> quaternion describe the same rotation.
        ctx.CheckNear(CnaDemos::MaxDifference(Matrix::CreateFromQuaternion(q), m), 0.0, 1e-5,
                      "CreateFromQuaternion(q) equals the yaw/pitch/roll matrix");
        const Quaternion back = Quaternion::CreateFromRotationMatrix(m);
        ctx.Print("q    = {}", q.ToString());
        ctx.Print("back = {}", back.ToString());
        ctx.CheckNear(CnaDemos::RotationDifference(back, q), 0.0, 1e-6, "CreateFromRotationMatrix recovers the rotation");

        ctx.Check(Quaternion::CreateFromYawPitchRoll(0.0f, 0.0f, 0.0f) == Quaternion::Identity,
                  "zero yaw, pitch and roll produce identity");

        // Quaternion components use half angles: one full 2-Pi turn changes representation sign,
        // while a second full turn restores the original components. Both remain the same rotation.
        const Quaternion oneExtraTurn = Quaternion::CreateFromYawPitchRoll(
            0.6f + MathHelper::TwoPi, -0.2f, 0.3f);
        const Quaternion twoExtraTurns = Quaternion::CreateFromYawPitchRoll(
            0.6f + 2.0f * MathHelper::TwoPi, -0.2f, 0.3f);
        ctx.Check(Quaternion::Subtract(oneExtraTurn, Quaternion::Negate(q)).Length() < 1e-6f,
                  "adding 2 Pi to yaw negates the quaternion representation");
        ctx.CheckNear(CnaDemos::RotationDifference(oneExtraTurn, q), 0.0, 1e-6,
                      "the antipodal 2-Pi representation keeps the same orientation");
        ctx.Check(Quaternion::Subtract(twoExtraTurns, q).Length() < 1e-6f,
                  "adding 4 Pi restores the original quaternion representation");

        const Quaternion nanYaw = Quaternion::CreateFromYawPitchRoll(NAN, -0.2f, 0.3f);
        ctx.Check(std::isnan(nanYaw.X) && std::isnan(nanYaw.Y) &&
                      std::isnan(nanYaw.Z) && std::isnan(nanYaw.W),
                  "a NaN yaw propagates to all four components");

        Quaternion byReference;
        Quaternion::CreateFromYawPitchRoll(0.6f, -0.2f, 0.3f, byReference);
        ctx.Check(byReference == q, "the output overload matches the value result");
    }
}

CNA_DEMOS_CONSOLE(QuaternionFromYawPitchRoll, {
    .id = "math.quaternion.from_yaw_pitch_roll",
    .title = "Quaternions and rotation matrices",
    .description = "Builds yaw/pitch/roll rotations, demonstrates quaternion 2-Pi/4-Pi representation periods, "
                   "NaN propagation and conversion/output overload parity.",
    .apis = {"Quaternion::CreateFromYawPitchRoll", "Quaternion::CreateFromRotationMatrix", "Matrix::CreateFromQuaternion"},
    .tags = {"quaternion", "matrix", "conversion"},
})
