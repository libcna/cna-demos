// SPDX-License-Identifier: MS-PL
// Undoing a transform with Matrix::Invert.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixInvert(CnaDemos::DemoContext& ctx)
    {
        const Matrix world = Matrix::CreateScale(2.0f) * Matrix::CreateRotationY(0.8f) * Matrix::CreateTranslation(5.0f, 1.0f, -3.0f);
        const Matrix inverse = Matrix::Invert(world);

        // world * inverse is the identity...
        ctx.CheckNear(CnaDemos::MaxDifference(world * inverse, Matrix::getIdentityProperty()), 0.0, 1e-5,
                      "world * Invert(world) is the identity");

        // ...so the inverse takes a world-space point back into model space. This is how a view
        // matrix is the inverse of the camera's own world matrix.
        const Vector3 modelPoint(1.0f, 2.0f, 3.0f);
        const Vector3 worldPoint = Vector3::Transform(modelPoint, world);
        const Vector3 back = Vector3::Transform(worldPoint, inverse);
        ctx.Print("model {} -> world {} -> back {}", modelPoint.ToString(), worldPoint.ToString(), back.ToString());
        ctx.CheckNear(Vector3::Distance(back, modelPoint), 0.0, 1e-5, "the inverse undoes the transform");

        Matrix byReference;
        Matrix::Invert(world, byReference);
        ctx.Check(byReference == inverse, "the by-reference overload gives the same inverse");
    }
}

CNA_DEMOS_CONSOLE(MatrixInvert, {
    .id = "math.matrix.invert",
    .title = "Invert a matrix",
    .description = "Computes Matrix::Invert of a world matrix and uses it to take a world-space point back into "
                   "model space.",
    .apis = {"Matrix::Invert", "Matrix::Identity"},
    .tags = {"matrix", "inverse"},
})
