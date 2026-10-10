// SPDX-License-Identifier: MS-PL
// Rotation about an arbitrary axis with Matrix::CreateFromAxisAngle.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixFromAxisAngle(CnaDemos::DemoContext& ctx)
    {
        // The axis must be unit length.
        const Vector3 diagonal = Vector3::Normalize(Vector3::One);
        const Matrix turn = Matrix::CreateFromAxisAngle(diagonal, MathHelper::TwoPi / 3.0f);

        // A third of a turn about (1, 1, 1) cycles the axes X -> Y -> Z -> X.
        const Vector3 x = Vector3::Transform(Vector3::UnitX, turn);
        ctx.Print("UnitX after 120 degrees about (1,1,1) = {}", x.ToString());
        ctx.CheckNear(Vector3::Distance(x, Vector3::UnitY), 0.0, 1e-5, "X goes to Y");
        ctx.CheckNear(Vector3::Distance(Vector3::Transform(Vector3::UnitY, turn), Vector3::UnitZ), 0.0, 1e-5, "Y goes to Z");

        // About a coordinate axis it matches the dedicated function.
        ctx.CheckNear(CnaDemos::MaxDifference(Matrix::CreateFromAxisAngle(Vector3::UnitY, 0.7f), Matrix::CreateRotationY(0.7f)),
                      0.0, 1e-6, "axis UnitY equals CreateRotationY");
    }
}

CNA_DEMOS_CONSOLE(MatrixFromAxisAngle, {
    .id = "math.matrix.from_axis_angle",
    .title = "Rotation about any axis",
    .description = "Builds a rotation about an arbitrary unit axis with Matrix::CreateFromAxisAngle; a third of a "
                   "turn about (1, 1, 1) cycles the coordinate axes.",
    .apis = {"Matrix::CreateFromAxisAngle", "Matrix::CreateRotationY"},
    .tags = {"matrix", "rotation", "axis-angle"},
})
