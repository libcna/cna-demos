// SPDX-License-Identifier: MS-PL
// 2D rotation about a pivot with Vector2::Transform.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector2TransformDemo(CnaDemos::DemoContext& ctx)
    {
        // Rotate a point 90 degrees about the pivot (100, 100) in screen space. In 2D the
        // rotation is about the Z axis. The pivot is moved to the origin, rotated and moved back.
        const Vector2 pivot(100.0f, 100.0f);
        const Vector2 point(150.0f, 100.0f);
        const Matrix rotateAboutPivot = Matrix::CreateTranslation(-pivot.X, -pivot.Y, 0.0f)
                                        * Matrix::CreateRotationZ(MathHelper::PiOver2)
                                        * Matrix::CreateTranslation(pivot.X, pivot.Y, 0.0f);

        const Vector2 rotated = Vector2::Transform(point, rotateAboutPivot);
        ctx.Print("Transform({}, rotate 90 about {}) = {}", point.ToString(), pivot.ToString(), rotated.ToString());

        // With +Y pointing down the screen, a positive Z rotation turns +X towards +Y (clockwise
        // on screen).
        ctx.CheckNear(Vector2::Distance(rotated, Vector2(100.0f, 150.0f)), 0.0, 1e-4,
                      "the point 50 px right of the pivot ends 50 px below it");

        const Vector2 direction = Vector2::TransformNormal(Vector2::UnitX, rotateAboutPivot);
        ctx.CheckNear(Vector2::Distance(direction, Vector2::UnitY), 0.0, 1e-6,
                      "TransformNormal rotates a direction without applying the pivot translations");

        const Matrix scaleAndTranslate = Matrix::CreateScale(2.0f, 3.0f, 1.0f) *
                                         Matrix::CreateTranslation(5.0f, -7.0f, 0.0f);
        const Vector2 authored(2.0f, -3.0f);
        Vector2 matrixOutput;
        Vector2::Transform(authored, scaleAndTranslate, matrixOutput);
        Vector2 matrixAlias = authored;
        Vector2::Transform(matrixAlias, scaleAndTranslate, matrixAlias);
        ctx.Check(matrixOutput == Vector2(9.0f, -16.0f) && matrixAlias == matrixOutput,
                  "matrix output transforms may alias their input safely");

        Vector2 normalOutput;
        Vector2::TransformNormal(authored, scaleAndTranslate, normalOutput);
        ctx.Check(normalOutput == Vector2(4.0f, -9.0f),
                  "the TransformNormal output form excludes translation");

        const Quaternion quarterTurn =
            Quaternion::CreateFromAxisAngle(Vector3::UnitZ, MathHelper::PiOver2);
        const Vector2 quaternionValue = Vector2::Transform(authored, quarterTurn);
        Vector2 quaternionOutput;
        Vector2::Transform(authored, quarterTurn, quaternionOutput);
        Vector2 quaternionAlias = authored;
        Vector2::Transform(quaternionAlias, quarterTurn, quaternionAlias);
        ctx.CheckNear(Vector2::Distance(quaternionValue, Vector2(3.0f, 2.0f)), 0.0, 1e-5,
                      "quaternion Transform rotates a Vector2 in the XY plane");
        ctx.Check(quaternionOutput == quaternionValue && quaternionAlias == quaternionValue,
                  "quaternion output transforms match the value form and may alias their input");

        const std::vector<Vector2> source = {
            Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f), Vector2(-1.0f, 5.0f)};
        std::vector<Vector2> matrixFull(3);
        Vector2::Transform(source, scaleAndTranslate, matrixFull);
        ctx.Check(matrixFull == std::vector<Vector2>{Vector2(7.0f, -1.0f), Vector2(11.0f, 5.0f),
                                                      Vector2(3.0f, 8.0f)},
                  "the full matrix-array overload transforms every source element");

        const std::vector<Vector2> matrixGuards = {
            Vector2(90.0f), Vector2(91.0f), Vector2(92.0f), Vector2(93.0f), Vector2(94.0f)};
        std::vector<Vector2> matrixRange = matrixGuards;
        Vector2::Transform(source, 1, scaleAndTranslate, matrixRange, 2, 2);
        ctx.Check(matrixRange == std::vector<Vector2>{Vector2(90.0f), Vector2(91.0f),
                                                       Vector2(11.0f, 5.0f), Vector2(3.0f, 8.0f),
                                                       Vector2(94.0f)},
                  "the matrix range overload preserves both destination guards");

        std::vector<Vector2> quaternionFull(3);
        Vector2::Transform(source, quarterTurn, quaternionFull);
        ctx.Check(Vector2::Distance(quaternionFull[0], Vector2(-2.0f, 1.0f)) < 1e-5f &&
                      Vector2::Distance(quaternionFull[1], Vector2(-4.0f, 3.0f)) < 1e-5f &&
                      Vector2::Distance(quaternionFull[2], Vector2(-5.0f, -1.0f)) < 1e-5f,
                  "the full quaternion-array overload rotates every source element");

        const std::vector<Vector2> quaternionGuards = {
            Vector2(80.0f), Vector2(81.0f), Vector2(82.0f), Vector2(83.0f), Vector2(84.0f)};
        std::vector<Vector2> quaternionRange = quaternionGuards;
        Vector2::Transform(source, 0, quarterTurn, quaternionRange, 1, 2);
        ctx.Check(quaternionRange[0] == quaternionGuards[0] &&
                      Vector2::Distance(quaternionRange[1], Vector2(-2.0f, 1.0f)) < 1e-5f &&
                      Vector2::Distance(quaternionRange[2], Vector2(-4.0f, 3.0f)) < 1e-5f &&
                      quaternionRange[3] == quaternionGuards[3] &&
                      quaternionRange[4] == quaternionGuards[4],
                  "the quaternion range overload writes only its selected destination window");

        std::vector<Vector2> normalFull(3);
        Vector2::TransformNormal(source, scaleAndTranslate, normalFull);
        std::vector<Vector2> normalRange = matrixGuards;
        Vector2::TransformNormal(source, 1, scaleAndTranslate, normalRange, 2, 2);
        ctx.Check(normalFull == std::vector<Vector2>{Vector2(2.0f, 6.0f), Vector2(6.0f, 12.0f),
                                                      Vector2(-2.0f, 15.0f)} &&
                      normalRange == std::vector<Vector2>{Vector2(90.0f), Vector2(91.0f),
                                                          Vector2(6.0f, 12.0f), Vector2(-2.0f, 15.0f),
                                                          Vector2(94.0f)},
                  "both normal-array overloads omit translation and preserve range guards");

        std::vector<Vector2> overlapRight = {
            Vector2(1.0f, 0.0f), Vector2(2.0f, 0.0f), Vector2(3.0f, 0.0f), Vector2(4.0f, 0.0f)};
        Vector2::Transform(overlapRight, 0, scaleAndTranslate, overlapRight, 1, 3);
        ctx.Check(overlapRight == std::vector<Vector2>{Vector2(1.0f, 0.0f), Vector2(7.0f, -7.0f),
                                                        Vector2(19.0f, -28.0f), Vector2(43.0f, -91.0f)},
                  "right-overlapping transforms follow XNA's forward cascading writes");

        std::vector<Vector2> overlapLeft = {
            Vector2(1.0f, 0.0f), Vector2(2.0f, 0.0f), Vector2(3.0f, 0.0f), Vector2(4.0f, 0.0f)};
        Vector2::Transform(overlapLeft, 1, scaleAndTranslate, overlapLeft, 0, 3);
        ctx.Check(overlapLeft == std::vector<Vector2>{Vector2(9.0f, -7.0f), Vector2(11.0f, -7.0f),
                                                       Vector2(13.0f, -7.0f), Vector2(4.0f, 0.0f)},
                  "left-overlapping transforms read each untouched source before writing it");

        std::vector<Vector2> invalidDestination = {
            Vector2(9.0f), Vector2(8.0f), Vector2(7.0f)};
        const std::vector<Vector2> invalidGuards = invalidDestination;
        bool negativeSourceRejected = false;
        bool shortSourceRejected = false;
        bool shortDestinationRejected = false;
        try { Vector2::Transform(source, -1, scaleAndTranslate, invalidDestination, 0, 1); }
        catch (...) { negativeSourceRejected = true; }
        try { Vector2::Transform(source, 2, scaleAndTranslate, invalidDestination, 0, 2); }
        catch (...) { shortSourceRejected = true; }
        try { Vector2::Transform(source, 0, scaleAndTranslate, invalidDestination, 2, 2); }
        catch (...) { shortDestinationRejected = true; }
        ctx.Check(negativeSourceRejected && shortSourceRejected && shortDestinationRejected &&
                      invalidDestination == invalidGuards,
                  "invalid source and destination windows are rejected before any write");

        bool negativeLengthAccepted = true;
        try { Vector2::Transform(source, 0, scaleAndTranslate, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        try { Vector2::Transform(source, 0, quarterTurn, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        try { Vector2::TransformNormal(source, 0, scaleAndTranslate, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        ctx.Check(negativeLengthAccepted && invalidDestination == invalidGuards,
                  "XNA accepts negative matrix, quaternion and normal lengths as empty ranges");

        bool zeroAtEndAccepted = true;
        try
        {
            Vector2::Transform(source, static_cast<int>(source.size()), scaleAndTranslate,
                               invalidDestination, static_cast<int>(invalidDestination.size()), 0);
            Vector2::Transform(source, static_cast<int>(source.size()), quarterTurn,
                               invalidDestination, static_cast<int>(invalidDestination.size()), 0);
            Vector2::TransformNormal(source, static_cast<int>(source.size()), scaleAndTranslate,
                                     invalidDestination, static_cast<int>(invalidDestination.size()), 0);
        }
        catch (...) { zeroAtEndAccepted = false; }
        ctx.Check(zeroAtEndAccepted && invalidDestination == invalidGuards,
                  "a zero-length transform accepts both one-past-end indices without writing");
    }
}

CNA_DEMOS_CONSOLE(Vector2TransformDemo, {
    .id = "math.vector2.transform",
    .title = "Rotate a 2D point about a pivot",
    .description = "Transforms Vector2 positions and normals with matrices and quaternions through value, output, "
                   "array, overlapping-range and refusal paths.",
    .apis = {"Vector2::Transform", "Vector2::TransformNormal", "Matrix::CreateRotationZ",
             "Matrix::CreateTranslation", "Matrix::CreateScale", "Quaternion::CreateFromAxisAngle",
             "Vector2::Distance"},
    .tags = {"2d", "rotation", "pivot", "arrays"},
    .restrictions = "CNA-DEMOS-ISSUE-121 records rejection of XNA's negative-length no-op.",
    .knownIssue = "CNA-DEMOS-ISSUE-121",
})
