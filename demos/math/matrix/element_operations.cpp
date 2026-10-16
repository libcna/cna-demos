// SPDX-License-Identifier: MS-PL
// Element-wise matrix arithmetic and Matrix::Lerp.
#include <any>
#include <cmath>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixElementOperations(CnaDemos::DemoContext& ctx)
    {
        const Matrix a = Matrix::CreateScale(2.0f);
        const Matrix b = Matrix::CreateTranslation(1.0f, 2.0f, 3.0f);

        // These work element by element -- they are NOT transform composition (that is a * b).
        const Matrix sum = Matrix::Add(a, b);
        ctx.Check(sum.M11 == 3.0f && sum.M41 == 1.0f && sum.M44 == 2.0f, "Add sums corresponding elements");
        ctx.Check(Matrix::Subtract(sum, b) == a, "Subtract undoes Add");
        // Every element is scaled, including M44: identity * 2 has 2 on the whole diagonal.
        const Matrix twice = Matrix::Multiply(Matrix::getIdentityProperty(), 2.0f);
        ctx.Check(twice.M11 == 2.0f && twice.M44 == 2.0f && twice.M12 == 0.0f, "Multiply(Matrix, float) scales every element");
        ctx.Check(Matrix::Divide(twice, 2.0f) == Matrix::getIdentityProperty(), "Divide(Matrix, float) divides every element");
        ctx.Check(Matrix::Multiply(a, 0.5f).M44 == 0.5f, "CreateScale(2) has M44 = 1, so halving it gives M44 = 0.5");
        ctx.Check(Matrix::Negate(a).M11 == -2.0f, "Negate flips every sign");
        ctx.Check(a + b == sum && sum - b == a && -a == Matrix::Negate(a),
                  "addition, subtraction and negation operators match the named methods");
        ctx.Check(twice / 2.0f == Matrix::getIdentityProperty(),
                  "the division operator performs the same element-wise scalar division");
        ctx.Check(Matrix::getIdentityProperty() * 2.0f == twice &&
                      2.0f * Matrix::getIdentityProperty() == twice,
                  "both scalar multiplication operand orders match the named method");

        const Matrix divisors(
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12,
            13, 14, 15, 16);
        const Matrix dividends = Matrix::Multiply(divisors, 2.0f);
        const Matrix expectedTwos(
            2, 2, 2, 2,
            2, 2, 2, 2,
            2, 2, 2, 2,
            2, 2, 2, 2);
        Matrix dividedOutput;
        Matrix::Divide(dividends, divisors, dividedOutput);
        ctx.Check(Matrix::Divide(dividends, divisors) == expectedTwos &&
                      dividends / divisors == expectedTwos && dividedOutput == expectedTwos,
                  "matrix division is component-wise through value, output and operator forms");

        Matrix compoundMatrix = Matrix::CreateScale(2.0f);
        Matrix& matrixReference = (compoundMatrix *= b);
        Matrix compoundScalar = b;
        Matrix& scalarReference = (compoundScalar *= 2.0f);
        ctx.Check(&matrixReference == &compoundMatrix && compoundMatrix == Matrix::Multiply(a, b) &&
                      &scalarReference == &compoundScalar && compoundScalar == Matrix::Multiply(b, 2.0f),
                  "CNA compound multiplication mutates and returns the original matrix");

        const Matrix sameA = Matrix::CreateScale(2.0f);
        const Matrix changedA = Matrix::CreateScale(3.0f);
        ctx.Check(a.Equals(sameA) && a.Equals(std::any{sameA}) &&
                      !a.Equals(std::any{42}) && !a.Equals(std::any{}),
                  "typed and boxed equality accept only the same 16 elements");
        ctx.Check(a == sameA && a != changedA && a.GetHashCode() == sameA.GetHashCode(),
                  "matrix operators and hashes follow all 16 elements");

        // Lerp is element-wise too: fine for blending translations, but it does not keep a
        // rotation a rotation (use Quaternion::Slerp for that).
        const Matrix halfway = Matrix::Lerp(Matrix::getIdentityProperty(), b, 0.5f);
        ctx.Print("Lerp(identity, translate(1, 2, 3), 0.5) translation = {}", halfway.getTranslationProperty().ToString());
        ctx.Check(halfway.M41 == 0.5f && halfway.M42 == 1.0f && halfway.M43 == 1.5f, "Lerp halves the translation");

        const Matrix from = Matrix::CreateTranslation(1.0f, 2.0f, 3.0f);
        const Matrix to = Matrix::CreateTranslation(5.0f, 10.0f, 15.0f);
        ctx.Check(Matrix::Lerp(from, to, -1.0f) == Matrix::CreateTranslation(-3.0f, -6.0f, -9.0f) &&
                      Matrix::Lerp(from, to, 2.0f) == Matrix::CreateTranslation(9.0f, 18.0f, 27.0f),
                  "Lerp extrapolates below zero and above one");

        Matrix nanLerp;
        Matrix::Lerp(from, to, std::numeric_limits<float>::quiet_NaN(), nanLerp);
        ctx.Check(std::isnan(nanLerp.M11) && std::isnan(nanLerp.M12) &&
                      std::isnan(nanLerp.M13) && std::isnan(nanLerp.M14) &&
                      std::isnan(nanLerp.M21) && std::isnan(nanLerp.M22) &&
                      std::isnan(nanLerp.M23) && std::isnan(nanLerp.M24) &&
                      std::isnan(nanLerp.M31) && std::isnan(nanLerp.M32) &&
                      std::isnan(nanLerp.M33) && std::isnan(nanLerp.M34) &&
                      std::isnan(nanLerp.M41) && std::isnan(nanLerp.M42) &&
                      std::isnan(nanLerp.M43) && std::isnan(nanLerp.M44),
                  "the Lerp output overload propagates a NaN amount to all 16 elements");
    }
}

CNA_DEMOS_CONSOLE(MatrixElementOperations, {
    .id = "math.matrix.element_operations",
    .title = "Element-wise matrix operations",
    .description = "Adds, subtracts, scales, divides, negates and interpolates matrices element by element, "
                   "including matrix quotients, scalar operand order and CNA compound multiplication.",
    .apis = {"Matrix::Add", "Matrix::Subtract", "Matrix::Multiply", "Matrix::Divide", "Matrix::Negate",
             "Matrix::Lerp", "Matrix::Equals", "Matrix::GetHashCode", "Matrix::op_Addition",
             "Matrix::op_Division", "Matrix::op_Equality", "Matrix::op_Inequality",
             "Matrix::op_Multiply", "Matrix::op_MultiplyAssignment", "Matrix::op_Subtraction",
             "Matrix::op_UnaryNegation"},
    .tags = {"matrix", "arithmetic", "interpolation"},
})
