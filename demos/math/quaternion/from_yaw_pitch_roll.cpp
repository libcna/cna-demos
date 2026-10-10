// SPDX-License-Identifier: MS-PL
// Quaternion::CreateFromYawPitchRoll and conversion between quaternions and matrices.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
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
    }
}

CNA_DEMOS_CONSOLE(QuaternionFromYawPitchRoll, {
    .id = "math.quaternion.from_yaw_pitch_roll",
    .title = "Quaternions and rotation matrices",
    .description = "Builds a rotation with Quaternion::CreateFromYawPitchRoll and converts between quaternion and "
                   "matrix with Matrix::CreateFromQuaternion and Quaternion::CreateFromRotationMatrix.",
    .apis = {"Quaternion::CreateFromYawPitchRoll", "Quaternion::CreateFromRotationMatrix", "Matrix::CreateFromQuaternion"},
    .tags = {"quaternion", "matrix", "conversion"},
})
