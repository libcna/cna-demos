// SPDX-License-Identifier: MS-PL
// Vector4 matrix/quaternion transforms across value, output, array, overlap and range boundaries.
#include <cmath>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    bool Near(const Vector4& value, const Vector4& expected, float tolerance = 1e-5f)
    {
        return std::abs(value.X - expected.X) < tolerance &&
               std::abs(value.Y - expected.Y) < tolerance &&
               std::abs(value.Z - expected.Z) < tolerance &&
               std::abs(value.W - expected.W) < tolerance;
    }

    void Vector4Transform(CnaDemos::DemoContext& ctx)
    {
        const Matrix matrix = Matrix::CreateScale(2.0f, 3.0f, 4.0f) *
                              Matrix::CreateTranslation(5.0f, -7.0f, 11.0f);
        const Quaternion rotation =
            Quaternion::CreateFromAxisAngle(Vector3::UnitZ, MathHelper::PiOver2);
        const Vector2 value2(2.0f, -3.0f);
        const Vector3 value3(2.0f, -3.0f, 4.0f);
        const Vector4 value4(2.0f, -3.0f, 4.0f, 2.0f);
        Vector4 output;

        Vector4::Transform(value2, matrix, output);
        ctx.Check(Vector4::Transform(value2, matrix) == Vector4(9.0f, -16.0f, 11.0f, 1.0f) &&
                      output == Vector4(9.0f, -16.0f, 11.0f, 1.0f),
                  "a matrix treats Vector2 as XY with implicit Z zero and W one");

        Vector4::Transform(value3, matrix, output);
        ctx.Check(Vector4::Transform(value3, matrix) == Vector4(9.0f, -16.0f, 27.0f, 1.0f) &&
                      output == Vector4(9.0f, -16.0f, 27.0f, 1.0f),
                  "a matrix treats Vector3 as XYZ with implicit W one");

        Vector4 matrixAlias = value4;
        Vector4::Transform(matrixAlias, matrix, matrixAlias);
        Vector4::Transform(value4, matrix, output);
        ctx.Check(Vector4::Transform(value4, matrix) == Vector4(14.0f, -23.0f, 38.0f, 2.0f) &&
                      output == Vector4(14.0f, -23.0f, 38.0f, 2.0f) && matrixAlias == output,
                  "authored W scales matrix translation and the output may alias a Vector4 input");

        Vector4::Transform(value2, rotation, output);
        ctx.Check(Near(Vector4::Transform(value2, rotation), Vector4(3.0f, 2.0f, 0.0f, 1.0f)) &&
                      output == Vector4::Transform(value2, rotation),
                  "a quaternion rotates Vector2 XY and supplies Z zero and W one");

        Vector4::Transform(value3, rotation, output);
        ctx.Check(Near(Vector4::Transform(value3, rotation), Vector4(3.0f, 2.0f, 4.0f, 1.0f)) &&
                      output == Vector4::Transform(value3, rotation),
                  "a quaternion rotates Vector3 XYZ and supplies W one");

        Vector4 quaternionAlias = value4;
        Vector4::Transform(quaternionAlias, rotation, quaternionAlias);
        Vector4::Transform(value4, rotation, output);
        ctx.Check(Near(Vector4::Transform(value4, rotation), Vector4(3.0f, 2.0f, 4.0f, 2.0f)) &&
                      output == Vector4::Transform(value4, rotation) && quaternionAlias == output,
                  "a quaternion preserves authored W and the output may alias a Vector4 input");

        const std::vector<Vector4> source = {
            Vector4(1.0f, 2.0f, 3.0f, 1.0f), Vector4(3.0f, 4.0f, 5.0f, 2.0f),
            Vector4(-1.0f, 5.0f, -2.0f, -1.0f)};
        std::vector<Vector4> matrixFull(3);
        Vector4::Transform(source, matrix, matrixFull);
        const std::vector<Vector4> matrixGuards = {
            Vector4(90.0f), Vector4(91.0f), Vector4(92.0f), Vector4(93.0f), Vector4(94.0f)};
        std::vector<Vector4> matrixRange = matrixGuards;
        Vector4::Transform(source, 1, matrix, matrixRange, 2, 2);
        ctx.Check(matrixFull == std::vector<Vector4>{Vector4(7.0f, -1.0f, 23.0f, 1.0f),
                                                      Vector4(16.0f, -2.0f, 42.0f, 2.0f),
                                                      Vector4(-7.0f, 22.0f, -19.0f, -1.0f)} &&
                      matrixRange == std::vector<Vector4>{Vector4(90.0f), Vector4(91.0f),
                                                          Vector4(16.0f, -2.0f, 42.0f, 2.0f),
                                                          Vector4(-7.0f, 22.0f, -19.0f, -1.0f),
                                                          Vector4(94.0f)},
                  "full and ranged matrix arrays preserve authored W and destination guards");

        std::vector<Vector4> quaternionFull(3);
        Vector4::Transform(source, rotation, quaternionFull);
        const std::vector<Vector4> quaternionGuards = {
            Vector4(80.0f), Vector4(81.0f), Vector4(82.0f), Vector4(83.0f), Vector4(84.0f)};
        std::vector<Vector4> quaternionRange = quaternionGuards;
        Vector4::Transform(source, 0, rotation, quaternionRange, 1, 2);
        ctx.Check(Near(quaternionFull[0], Vector4(-2.0f, 1.0f, 3.0f, 1.0f)) &&
                      Near(quaternionFull[1], Vector4(-4.0f, 3.0f, 5.0f, 2.0f)) &&
                      Near(quaternionFull[2], Vector4(-5.0f, -1.0f, -2.0f, -1.0f)) &&
                      quaternionRange[0] == quaternionGuards[0] &&
                      quaternionRange[1] == quaternionFull[0] &&
                      quaternionRange[2] == quaternionFull[1] &&
                      quaternionRange[3] == quaternionGuards[3] &&
                      quaternionRange[4] == quaternionGuards[4],
                  "full and ranged quaternion arrays rotate XYZ, preserve W and keep guards");

        std::vector<Vector4> overlapRight = {
            Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector4(2.0f, 0.0f, 0.0f, 1.0f),
            Vector4(3.0f, 0.0f, 0.0f, 1.0f), Vector4(4.0f, 0.0f, 0.0f, 1.0f)};
        Vector4::Transform(overlapRight, 0, matrix, overlapRight, 1, 3);
        std::vector<Vector4> overlapLeft = {
            Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector4(2.0f, 0.0f, 0.0f, 1.0f),
            Vector4(3.0f, 0.0f, 0.0f, 1.0f), Vector4(4.0f, 0.0f, 0.0f, 1.0f)};
        Vector4::Transform(overlapLeft, 1, matrix, overlapLeft, 0, 3);
        ctx.Check(overlapRight == std::vector<Vector4>{Vector4(1.0f, 0.0f, 0.0f, 1.0f),
                                                        Vector4(7.0f, -7.0f, 11.0f, 1.0f),
                                                        Vector4(19.0f, -28.0f, 55.0f, 1.0f),
                                                        Vector4(43.0f, -91.0f, 231.0f, 1.0f)} &&
                      overlapLeft == std::vector<Vector4>{Vector4(9.0f, -7.0f, 11.0f, 1.0f),
                                                          Vector4(11.0f, -7.0f, 11.0f, 1.0f),
                                                          Vector4(13.0f, -7.0f, 11.0f, 1.0f),
                                                          Vector4(4.0f, 0.0f, 0.0f, 1.0f)},
                  "same-array ranges reproduce XNA's forward overlap semantics in both directions");

        std::vector<Vector4> invalidDestination = {
            Vector4(9.0f), Vector4(8.0f), Vector4(7.0f)};
        const std::vector<Vector4> invalidGuards = invalidDestination;
        bool negativeSourceRejected = false;
        bool shortSourceRejected = false;
        bool shortDestinationRejected = false;
        try { Vector4::Transform(source, -1, matrix, invalidDestination, 0, 1); }
        catch (...) { negativeSourceRejected = true; }
        try { Vector4::Transform(source, 2, matrix, invalidDestination, 0, 2); }
        catch (...) { shortSourceRejected = true; }
        try { Vector4::Transform(source, 0, matrix, invalidDestination, 2, 2); }
        catch (...) { shortDestinationRejected = true; }
        ctx.Check(negativeSourceRejected && shortSourceRejected && shortDestinationRejected &&
                      invalidDestination == invalidGuards,
                  "invalid source and destination windows are rejected before any write");

        bool negativeLengthAccepted = true;
        try { Vector4::Transform(source, 0, matrix, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        try { Vector4::Transform(source, 0, rotation, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        ctx.Check(negativeLengthAccepted && invalidDestination == invalidGuards,
                  "XNA accepts negative matrix and quaternion lengths as empty ranges");

        bool zeroAtEndAccepted = true;
        try
        {
            Vector4::Transform(source, static_cast<int>(source.size()), matrix, invalidDestination,
                               static_cast<int>(invalidDestination.size()), 0);
            Vector4::Transform(source, static_cast<int>(source.size()), rotation, invalidDestination,
                               static_cast<int>(invalidDestination.size()), 0);
        }
        catch (...) { zeroAtEndAccepted = false; }
        ctx.Check(zeroAtEndAccepted && invalidDestination == invalidGuards,
                  "zero-length transforms accept both one-past-end indices without writing");
    }
}

CNA_DEMOS_CONSOLE(Vector4Transform, {
    .id = "math.vector4.transform",
    .title = "Vector4 transforms and arrays",
    .description = "Transforms Vector2, Vector3 and Vector4 values plus whole, ranged and overlapping Vector4 "
                   "arrays with matrices and quaternions, including refusal boundaries.",
    .apis = {"Vector4::Transform", "Matrix::CreateScale", "Matrix::CreateTranslation",
             "Quaternion::CreateFromAxisAngle"},
    .tags = {"vector", "matrix", "quaternion", "arrays"},
    .restrictions = "CNA-DEMOS-ISSUE-121 records rejection of XNA's negative-length no-op.",
    .knownIssue = "CNA-DEMOS-ISSUE-121",
})
