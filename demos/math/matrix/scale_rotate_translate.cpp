// SPDX-License-Identifier: MS-PL
// Building a world matrix as Scale * Rotation * Translation.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixScaleRotateTranslate(CnaDemos::DemoContext& ctx)
    {
        // The usual world matrix: scale the model, then rotate it, then place it in the world.
        const Matrix scale = Matrix::CreateScale(2.0f);
        const Matrix rotation = Matrix::CreateRotationY(MathHelper::PiOver2);
        const Matrix translation = Matrix::CreateTranslation(10.0f, 0.0f, 0.0f);
        const Matrix world = scale * rotation * translation;

        const Vector3 tip(1.0f, 0.0f, 0.0f);   // a point on the model, one unit along +X
        const Vector3 placed = Vector3::Transform(tip, world);
        ctx.Print("model point {} -> world {}", tip.ToString(), placed.ToString());
        ctx.CheckNear(Vector3::Distance(placed, Vector3(10.0f, 0.0f, -2.0f)), 0.0, 1e-5,
                      "scaled to 2, turned to -Z, then moved to x = 10");

        // The other overloads: per-axis scale and Vector3 arguments.
        const Matrix stretch = Matrix::CreateScale(1.0f, 3.0f, 1.0f);
        ctx.Check(Vector3::Transform(Vector3::One, stretch) == Vector3(1.0f, 3.0f, 1.0f), "CreateScale(x, y, z) scales per axis");
        ctx.Check(Matrix::CreateScale(Vector3(1.0f, 3.0f, 1.0f)) == stretch, "CreateScale(Vector3) is the same");
        ctx.Check(Matrix::CreateTranslation(Vector3(10.0f, 0.0f, 0.0f)) == translation, "CreateTranslation(Vector3) is the same");
    }
}

CNA_DEMOS_CONSOLE(MatrixScaleRotateTranslate, {
    .id = "math.matrix.scale_rotate_translate",
    .title = "Scale, rotate, translate",
    .description = "Builds a world matrix from CreateScale, CreateRotationY and CreateTranslation in the "
                   "conventional scale-rotate-translate order and places a model point with it.",
    .apis = {"Matrix::CreateScale", "Matrix::CreateRotationY", "Matrix::CreateTranslation", "Matrix::op_Multiply"},
    .tags = {"matrix", "world", "srt"},
})
