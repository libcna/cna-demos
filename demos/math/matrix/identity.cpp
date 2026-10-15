// SPDX-License-Identifier: MS-PL
// The identity matrix: the "do nothing" transform.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixIdentity(CnaDemos::DemoContext& ctx)
    {
        // In CNA the C# static property Matrix.Identity is the static getter getIdentityProperty().
        const Matrix identity = Matrix::getIdentityProperty();
        ctx.Print("Matrix::getIdentityProperty() = {}", identity.ToString());

        const Matrix some = Matrix::CreateRotationX(0.3f) * Matrix::CreateTranslation(1.0f, 2.0f, 3.0f);
        ctx.Check(identity * some == some && some * identity == some, "multiplying by the identity changes nothing");
        ctx.Check(Vector3::Transform(Vector3(4.0f, 5.0f, 6.0f), identity) == Vector3(4.0f, 5.0f, 6.0f),
                  "transforming by the identity leaves a point where it is");
        ctx.Check(identity.M11 == 1.0f && identity.M22 == 1.0f && identity.M33 == 1.0f && identity.M44 == 1.0f
                      && identity.M12 == 0.0f && identity.M41 == 0.0f,
                  "ones on the diagonal, zeros elsewhere");
        const Matrix ordered(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        ctx.Check(ordered.M11 == 1 && ordered.M12 == 2 && ordered.M13 == 3 && ordered.M14 == 4 &&
                      ordered.M21 == 5 && ordered.M22 == 6 && ordered.M23 == 7 && ordered.M24 == 8 &&
                      ordered.M31 == 9 && ordered.M32 == 10 && ordered.M33 == 11 && ordered.M34 == 12 &&
                      ordered.M41 == 13 && ordered.M42 == 14 && ordered.M43 == 15 && ordered.M44 == 16,
                  "the 16-value constructor maps row-major arguments to all public fields");
        // A default-constructed Matrix is all zeros, not the identity.
        ctx.Check(Matrix() != identity, "Matrix() is the zero matrix");
    }
}

CNA_DEMOS_CONSOLE(MatrixIdentity, {
    .id = "math.matrix.identity",
    .title = "The identity matrix",
    .description = "Gets the identity matrix and shows that multiplying or transforming by it changes nothing, "
                   "while a default-constructed Matrix is all zeros.",
    .apis = {"Matrix::Matrix", "Matrix::Identity", "Matrix::op_Multiply", "Matrix::ToString",
             "Matrix::M11", "Matrix::M12", "Matrix::M13", "Matrix::M14",
             "Matrix::M21", "Matrix::M22", "Matrix::M23", "Matrix::M24",
             "Matrix::M31", "Matrix::M32", "Matrix::M33", "Matrix::M34",
             "Matrix::M41", "Matrix::M42", "Matrix::M43", "Matrix::M44"},
    .tags = {"matrix", "identity"},
})
