// SPDX-License-Identifier: MS-PL
// Undoing a transform with Matrix::Invert.
#include <cmath>

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

        Matrix aliased = world;
        Matrix::Invert(aliased, aliased);
        ctx.Check(aliased == inverse, "the output overload supports the same input and output variable");

        // XNA exposes the underlying floating-point boundary instead of rejecting singular input.
        const Matrix zeroInverse = Matrix::Invert(Matrix());
        ctx.Check(std::isnan(zeroInverse.M11) && std::isnan(zeroInverse.M12) &&
                      std::isnan(zeroInverse.M13) && std::isnan(zeroInverse.M14) &&
                      std::isnan(zeroInverse.M21) && std::isnan(zeroInverse.M22) &&
                      std::isnan(zeroInverse.M23) && std::isnan(zeroInverse.M24) &&
                      std::isnan(zeroInverse.M31) && std::isnan(zeroInverse.M32) &&
                      std::isnan(zeroInverse.M33) && std::isnan(zeroInverse.M34) &&
                      std::isnan(zeroInverse.M41) && std::isnan(zeroInverse.M42) &&
                      std::isnan(zeroInverse.M43) && std::isnan(zeroInverse.M44),
                  "XNA inversion of the zero matrix publishes sixteen NaN components");

        const Matrix flat = Matrix::CreateScale(1.0f, 0.0f, 1.0f) *
                            Matrix::CreateTranslation(7.0f, 8.0f, 9.0f);
        const Matrix flatInverse = Matrix::Invert(flat);
        ctx.Print("singular affine inverse: {}", flatInverse.ToString());
        ctx.Check(flatInverse.M22 == 8.0f && flatInverse.M42 == -8.0f &&
                      std::isnan(flatInverse.M11) && std::isnan(flatInverse.M12) &&
                      std::isnan(flatInverse.M13) && std::isnan(flatInverse.M14) &&
                      std::isnan(flatInverse.M21) && std::isnan(flatInverse.M23) &&
                      std::isnan(flatInverse.M24) && std::isnan(flatInverse.M31) &&
                      std::isnan(flatInverse.M32) && std::isnan(flatInverse.M33) &&
                      std::isnan(flatInverse.M34) && std::isnan(flatInverse.M41) &&
                      std::isnan(flatInverse.M43) && std::isnan(flatInverse.M44),
                  "XNA singular affine inversion retains its two finite cofactor quotients");

        const Matrix tiny = Matrix::CreateScale(1.0f, 1e-20f, 1.0f);
        const Matrix tinyInverse = Matrix::Invert(tiny);
        ctx.CheckNear(tinyInverse.M22, 1e20, 1e14,
                      "a nonzero scale remains invertible even far below ordinary epsilon");
    }
}

CNA_DEMOS_CONSOLE(MatrixInvert, {
    .id = "math.matrix.invert",
    .title = "Invert a matrix",
    .description = "Computes Matrix::Invert for ordinary, tiny-scale and singular matrices, including XNA's "
                   "public IEEE-754 NaN boundary behavior.",
    .apis = {"Matrix::Invert", "Matrix::Identity"},
    .tags = {"matrix", "inverse"},
    .restrictions = "CNA-DEMOS-ISSUE-116 records CNA's differing finite components for a singular affine inverse.",
    .knownIssue = "CNA-DEMOS-ISSUE-116",
})
