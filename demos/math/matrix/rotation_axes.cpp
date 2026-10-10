// SPDX-License-Identifier: MS-PL
// CreateRotationX, CreateRotationY and CreateRotationZ: which way each one turns.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixRotationAxes(CnaDemos::DemoContext& ctx)
    {
        const float quarter = MathHelper::PiOver2;   // angles are in radians

        // Positive angles turn counter-clockwise when looking down the axis towards the origin.
        const Vector3 yAboutX = Vector3::Transform(Vector3::UnitY, Matrix::CreateRotationX(quarter));
        const Vector3 zAboutY = Vector3::Transform(Vector3::UnitZ, Matrix::CreateRotationY(quarter));
        const Vector3 xAboutZ = Vector3::Transform(Vector3::UnitX, Matrix::CreateRotationZ(quarter));
        ctx.Print("RotationX(90): UnitY -> {}", yAboutX.ToString());
        ctx.Print("RotationY(90): UnitZ -> {}", zAboutY.ToString());
        ctx.Print("RotationZ(90): UnitX -> {}", xAboutZ.ToString());

        ctx.CheckNear(Vector3::Distance(yAboutX, Vector3::UnitZ), 0.0, 1e-6, "about X: +Y turns to +Z");
        ctx.CheckNear(Vector3::Distance(zAboutY, Vector3::UnitX), 0.0, 1e-6, "about Y: +Z turns to +X");
        ctx.CheckNear(Vector3::Distance(xAboutZ, Vector3::UnitY), 0.0, 1e-6, "about Z: +X turns to +Y");
    }
}

CNA_DEMOS_CONSOLE(MatrixRotationAxes, {
    .id = "math.matrix.rotation_axes",
    .title = "Rotation about the X, Y and Z axes",
    .description = "Rotates unit vectors a quarter turn with Matrix::CreateRotationX, CreateRotationY and "
                   "CreateRotationZ to show the direction of positive angles.",
    .apis = {"Matrix::CreateRotationX", "Matrix::CreateRotationY", "Matrix::CreateRotationZ", "MathHelper::PiOver2"},
    .tags = {"matrix", "rotation", "radians"},
})
