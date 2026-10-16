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

        // CreateFromRotationMatrix has separate trace/X/Y/Z-dominant branches. Identity exercises
        // the trace branch; half turns make each diagonal branch dominant.
        ctx.Check(Quaternion::CreateFromRotationMatrix(Matrix::getIdentityProperty()) == Quaternion::Identity,
                  "identity uses the positive-trace conversion branch");
        for (const Matrix& halfTurn : {Matrix::CreateRotationX(MathHelper::Pi),
                                       Matrix::CreateRotationY(MathHelper::Pi),
                                       Matrix::CreateRotationZ(MathHelper::Pi)})
        {
            const Quaternion converted = Quaternion::CreateFromRotationMatrix(halfTurn);
            ctx.CheckNear(CnaDemos::MaxDifference(Matrix::CreateFromQuaternion(converted), halfTurn),
                          0.0, 1e-6, "a half turn survives its dominant-diagonal conversion branch");
        }

        // XNA neither validates nor normalizes a matrix passed as a rotation matrix.
        const Quaternion zeroMatrix = Quaternion::CreateFromRotationMatrix(Matrix());
        ctx.Check(zeroMatrix == Quaternion(0.5f, 0.0f, 0.0f, 0.0f),
                  "the zero matrix follows the X-dominant formula and yields a half-length value");
        const Matrix scale = Matrix::CreateScale(2.0f, 3.0f, 4.0f);
        const Quaternion scaleResult = Quaternion::CreateFromRotationMatrix(scale);
        ctx.Check(scaleResult.X == 0.0f && scaleResult.Y == 0.0f && scaleResult.Z == 0.0f &&
                      std::abs(scaleResult.W - std::sqrt(2.5f)) < 1e-6f,
                  "a scale matrix follows the trace formula without normalizing the result");
        Quaternion scaleByReference;
        Quaternion::CreateFromRotationMatrix(scale, scaleByReference);
        ctx.Check(scaleByReference == scaleResult,
                  "the rotation-matrix output overload preserves the nonrotation result");
    }
}

CNA_DEMOS_CONSOLE(QuaternionFromYawPitchRoll, {
    .id = "math.quaternion.from_yaw_pitch_roll",
    .title = "Quaternions and rotation matrices",
    .description = "Builds Euler rotations, demonstrates representation periods and exercises all rotation-matrix "
                   "conversion branches, including nonrotation inputs and output overloads.",
    .apis = {"Quaternion::CreateFromYawPitchRoll", "Quaternion::CreateFromRotationMatrix", "Matrix::CreateFromQuaternion"},
    .tags = {"quaternion", "matrix", "conversion"},
})
