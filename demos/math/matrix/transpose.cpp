// SPDX-License-Identifier: MS-PL
// Matrix::Transpose, and why it inverts a pure rotation.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixTranspose(CnaDemos::DemoContext& ctx)
    {
        Matrix m;
        m.M12 = 1.0f;
        m.M21 = 2.0f;
        m.M34 = 3.0f;
        const Matrix t = Matrix::Transpose(m);
        ctx.Print("M12 {} M21 {} M34 {}  ->  M12 {} M21 {} M43 {}", m.M12, m.M21, m.M34, t.M12, t.M21, t.M43);
        ctx.Check(t.M12 == m.M21 && t.M21 == m.M12 && t.M43 == m.M34, "rows become columns");

        Matrix output;
        Matrix::Transpose(m, output);
        ctx.Check(output == t, "the output overload matches the value result");

        Matrix aliased = m;
        Matrix::Transpose(aliased, aliased);
        ctx.Check(aliased == t, "the output overload supports the same input and output variable");

        // A rotation matrix is orthonormal: its transpose is its inverse, and cheaper to compute.
        const Matrix rotation = Matrix::CreateFromYawPitchRoll(0.4f, -0.3f, 0.2f);
        ctx.CheckNear(CnaDemos::MaxDifference(Matrix::Transpose(rotation), Matrix::Invert(rotation)), 0.0, 1e-5,
                      "for a rotation, Transpose equals Invert");
    }
}

CNA_DEMOS_CONSOLE(MatrixTranspose, {
    .id = "math.matrix.transpose",
    .title = "Transpose a matrix",
    .description = "Swaps rows and columns through value, output and aliased-output forms, and shows that the "
                   "transpose of a rotation is its inverse.",
    .apis = {"Matrix::Transpose", "Matrix::Invert", "Matrix::M12"},
    .tags = {"matrix", "transpose", "orthonormal"},
    .restrictions = "CNA-DEMOS-ISSUE-139 records corruption when Transpose uses the same input and output matrix.",
    .knownIssue = "CNA-DEMOS-ISSUE-139",
})
