// SPDX-License-Identifier: MS-PL
// Moving a point with a matrix: Vector3::Transform(position, Matrix).
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector3Transform(CnaDemos::DemoContext& ctx)
    {
        const Vector3 point(1.0f, 0.0f, 0.0f);

        // Rotate 90 degrees about Y, then move 10 units up. XNA matrices compose left to right:
        // in "rotation * translation" the rotation happens first.
        const Matrix world = Matrix::CreateRotationY(MathHelper::PiOver2) * Matrix::CreateTranslation(0.0f, 10.0f, 0.0f);
        const Vector3 moved = Vector3::Transform(point, world);
        ctx.Print("Transform({}, rotateY(90) * translate(0, 10, 0)) = {}", point.ToString(), moved.ToString());
        ctx.CheckNear(Vector3::Distance(moved, Vector3(0.0f, 10.0f, -1.0f)), 0.0, 1e-5,
                      "+X rotated 90 degrees about Y points to -Z, then rises by 10");

        // Reversing the order rotates the translation too: here it makes no difference to Y but
        // shows that order matters for X/Z offsets.
        const Matrix translateThenRotate = Matrix::CreateTranslation(5.0f, 0.0f, 0.0f) * Matrix::CreateRotationY(MathHelper::PiOver2);
        const Matrix rotateThenTranslate = Matrix::CreateRotationY(MathHelper::PiOver2) * Matrix::CreateTranslation(5.0f, 0.0f, 0.0f);
        const Vector3 a = Vector3::Transform(Vector3::Zero, translateThenRotate);
        const Vector3 b = Vector3::Transform(Vector3::Zero, rotateThenTranslate);
        ctx.Print("origin, translate then rotate = {}", a.ToString());
        ctx.Print("origin, rotate then translate = {}", b.ToString());
        ctx.CheckNear(Vector3::Distance(a, Vector3(0.0f, 0.0f, -5.0f)), 0.0, 1e-5, "translate-then-rotate swings the offset to -Z");
        ctx.CheckNear(Vector3::Distance(b, Vector3(5.0f, 0.0f, 0.0f)), 0.0, 1e-5, "rotate-then-translate keeps the offset on +X");

        const Matrix scaleAndTranslate = Matrix::CreateScale(2.0f, 3.0f, 4.0f) *
                                         Matrix::CreateTranslation(5.0f, -7.0f, 11.0f);
        const Vector3 authored(2.0f, -3.0f, 4.0f);
        Vector3 output;
        Vector3::Transform(authored, scaleAndTranslate, output);
        Vector3 alias = authored;
        Vector3::Transform(alias, scaleAndTranslate, alias);
        ctx.Check(output == Vector3(9.0f, -16.0f, 27.0f) && alias == output,
                  "matrix output transforms may alias their input safely");
    }
}

CNA_DEMOS_CONSOLE(Vector3Transform, {
    .id = "math.vector3.transform",
    .title = "Transform a point by a matrix",
    .description = "Applies a world matrix to a position with Vector3::Transform and shows that XNA matrix "
                   "products apply left to right, so their order matters.",
    .apis = {"Vector3::Transform", "Matrix::CreateRotationY", "Matrix::CreateTranslation", "Matrix::op_Multiply"},
    .tags = {"matrix", "world", "position"},
})
