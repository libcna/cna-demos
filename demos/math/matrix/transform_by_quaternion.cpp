// SPDX-License-Identifier: MS-PL
// Rotating an existing matrix with Matrix::Transform(Matrix, Quaternion).
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixTransformByQuaternion(CnaDemos::DemoContext& ctx)
    {
        const Matrix placed = Matrix::CreateTranslation(5.0f, 0.0f, 0.0f);
        const Quaternion turn = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::PiOver2);

        // Equivalent to placed * CreateFromQuaternion(turn): the rotation is applied after the
        // existing transform, so the translation swings around the origin too.
        const Matrix rotated = Matrix::Transform(placed, turn);
        ctx.Print("translation after Transform(placed, quarter turn) = {}", rotated.getTranslationProperty().ToString());
        ctx.CheckNear(CnaDemos::MaxDifference(rotated, placed * Matrix::CreateFromQuaternion(turn)), 0.0, 1e-5,
                      "equals multiplying by the quaternion's matrix");
        ctx.CheckNear(Vector3::Distance(rotated.getTranslationProperty(), Vector3(0.0f, 0.0f, -5.0f)), 0.0, 1e-5,
                      "the offset swung from +X to -Z");

        Matrix output;
        Matrix::Transform(placed, turn, output);
        Matrix aliased = placed;
        Matrix::Transform(aliased, turn, aliased);
        ctx.Check(output == rotated && aliased == rotated,
                  "output and same-variable output forms match the value result");
    }
}

CNA_DEMOS_CONSOLE(MatrixTransformByQuaternion, {
    .id = "math.matrix.transform_by_quaternion",
    .title = "Rotate a matrix by a quaternion",
    .description = "Applies a Quaternion rotation through value, output and same-variable output forms and shows "
                   "that it equals multiplying by Matrix::CreateFromQuaternion.",
    .apis = {"Matrix::Transform", "Matrix::CreateFromQuaternion", "Quaternion::CreateFromAxisAngle"},
    .tags = {"matrix", "quaternion", "rotation"},
})
