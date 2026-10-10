// SPDX-License-Identifier: MS-PL
// Orientation from yaw, pitch and roll with Matrix::CreateFromYawPitchRoll.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixFromYawPitchRoll(CnaDemos::DemoContext& ctx)
    {
        const float yaw = MathHelper::ToRadians(30.0f);    // about Y
        const float pitch = MathHelper::ToRadians(20.0f);  // about X
        const float roll = MathHelper::ToRadians(10.0f);   // about Z

        const Matrix orientation = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);

        // Applied as roll first, then pitch, then yaw.
        const Matrix composed = Matrix::CreateRotationZ(roll) * Matrix::CreateRotationX(pitch) * Matrix::CreateRotationY(yaw);
        ctx.Print("forward after yaw 30, pitch 20, roll 10 = {}", orientation.getForwardProperty().ToString());
        ctx.CheckNear(CnaDemos::MaxDifference(orientation, composed), 0.0, 1e-5,
                      "equals RotationZ(roll) * RotationX(pitch) * RotationY(yaw)");

        // Yaw alone is a turn about Up.
        ctx.CheckNear(CnaDemos::MaxDifference(Matrix::CreateFromYawPitchRoll(yaw, 0.0f, 0.0f), Matrix::CreateRotationY(yaw)),
                      0.0, 1e-6, "yaw alone equals CreateRotationY");
    }
}

CNA_DEMOS_CONSOLE(MatrixFromYawPitchRoll, {
    .id = "math.matrix.from_yaw_pitch_roll",
    .title = "Yaw, pitch and roll",
    .description = "Builds an orientation with Matrix::CreateFromYawPitchRoll and shows it equals roll, then "
                   "pitch, then yaw applied as separate axis rotations.",
    .apis = {"Matrix::CreateFromYawPitchRoll", "Matrix::Forward", "MathHelper::ToRadians"},
    .tags = {"matrix", "rotation", "euler"},
})
