// SPDX-License-Identifier: MS-PL
// Transforming many points at once with the array overloads of Vector3::Transform.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector3TransformArray(CnaDemos::DemoContext& ctx)
    {
        // The corners of a unit square in the XY plane.
        const std::vector<Vector3> corners = {
            {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
        std::vector<Vector3> placed(corners.size());

        // Scale by 2 and move to (10, 0, 0) -- one call for the whole array.
        const Matrix world = Matrix::CreateScale(2.0f) * Matrix::CreateTranslation(10.0f, 0.0f, 0.0f);
        Vector3::Transform(corners, world, placed);

        for (std::size_t i = 0; i < corners.size(); ++i)
        {
            ctx.Print("{} -> {}", corners[i].ToString(), placed[i].ToString());
        }
        ctx.Check(placed[0] == Vector3(10.0f, 0.0f, 0.0f), "the first corner moved to (10, 0, 0)");
        ctx.Check(placed[2] == Vector3(12.0f, 2.0f, 0.0f), "the opposite corner moved to (12, 2, 0)");

        const Matrix scaleAndTranslate = Matrix::CreateScale(2.0f, 3.0f, 4.0f) *
                                         Matrix::CreateTranslation(5.0f, -7.0f, 11.0f);
        const Quaternion quarterTurn =
            Quaternion::CreateFromAxisAngle(Vector3::UnitZ, MathHelper::PiOver2);
        const std::vector<Vector3> source = {
            Vector3(1.0f, 2.0f, 3.0f), Vector3(3.0f, 4.0f, 5.0f), Vector3(-1.0f, 5.0f, -2.0f)};

        std::vector<Vector3> matrixFull(3);
        Vector3::Transform(source, scaleAndTranslate, matrixFull);
        const std::vector<Vector3> matrixGuards = {
            Vector3(90.0f), Vector3(91.0f), Vector3(92.0f), Vector3(93.0f), Vector3(94.0f)};
        std::vector<Vector3> matrixRange = matrixGuards;
        Vector3::Transform(source, 1, scaleAndTranslate, matrixRange, 2, 2);
        ctx.Check(matrixFull == std::vector<Vector3>{Vector3(7.0f, -1.0f, 23.0f),
                                                      Vector3(11.0f, 5.0f, 31.0f),
                                                      Vector3(3.0f, 8.0f, 3.0f)} &&
                      matrixRange == std::vector<Vector3>{Vector3(90.0f), Vector3(91.0f),
                                                          Vector3(11.0f, 5.0f, 31.0f),
                                                          Vector3(3.0f, 8.0f, 3.0f), Vector3(94.0f)},
                  "full and ranged matrix overloads transform 3D values and preserve guards");

        std::vector<Vector3> quaternionFull(3);
        Vector3::Transform(source, quarterTurn, quaternionFull);
        const std::vector<Vector3> quaternionGuards = {
            Vector3(80.0f), Vector3(81.0f), Vector3(82.0f), Vector3(83.0f), Vector3(84.0f)};
        std::vector<Vector3> quaternionRange = quaternionGuards;
        Vector3::Transform(source, 0, quarterTurn, quaternionRange, 1, 2);
        ctx.Check(Vector3::Distance(quaternionFull[0], Vector3(-2.0f, 1.0f, 3.0f)) < 1e-5f &&
                      Vector3::Distance(quaternionFull[1], Vector3(-4.0f, 3.0f, 5.0f)) < 1e-5f &&
                      Vector3::Distance(quaternionFull[2], Vector3(-5.0f, -1.0f, -2.0f)) < 1e-5f &&
                      quaternionRange[0] == quaternionGuards[0] &&
                      Vector3::Distance(quaternionRange[1], quaternionFull[0]) < 1e-5f &&
                      Vector3::Distance(quaternionRange[2], quaternionFull[1]) < 1e-5f &&
                      quaternionRange[3] == quaternionGuards[3] &&
                      quaternionRange[4] == quaternionGuards[4],
                  "full and ranged quaternion overloads rotate 3D values and preserve guards");

        std::vector<Vector3> normalFull(3);
        Vector3::TransformNormal(source, scaleAndTranslate, normalFull);
        std::vector<Vector3> normalRange = matrixGuards;
        Vector3::TransformNormal(source, 1, scaleAndTranslate, normalRange, 2, 2);
        ctx.Check(normalFull == std::vector<Vector3>{Vector3(2.0f, 6.0f, 12.0f),
                                                      Vector3(6.0f, 12.0f, 20.0f),
                                                      Vector3(-2.0f, 15.0f, -8.0f)} &&
                      normalRange == std::vector<Vector3>{Vector3(90.0f), Vector3(91.0f),
                                                          Vector3(6.0f, 12.0f, 20.0f),
                                                          Vector3(-2.0f, 15.0f, -8.0f), Vector3(94.0f)},
                  "both normal-array overloads omit translation and preserve range guards");

        std::vector<Vector3> overlapRight = {
            Vector3(1.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f),
            Vector3(3.0f, 0.0f, 0.0f), Vector3(4.0f, 0.0f, 0.0f)};
        Vector3::Transform(overlapRight, 0, scaleAndTranslate, overlapRight, 1, 3);
        ctx.Check(overlapRight == std::vector<Vector3>{Vector3(1.0f, 0.0f, 0.0f),
                                                        Vector3(7.0f, -7.0f, 11.0f),
                                                        Vector3(19.0f, -28.0f, 55.0f),
                                                        Vector3(43.0f, -91.0f, 231.0f)},
                  "right-overlapping transforms follow XNA's forward cascading writes");

        std::vector<Vector3> overlapLeft = {
            Vector3(1.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f),
            Vector3(3.0f, 0.0f, 0.0f), Vector3(4.0f, 0.0f, 0.0f)};
        Vector3::Transform(overlapLeft, 1, scaleAndTranslate, overlapLeft, 0, 3);
        ctx.Check(overlapLeft == std::vector<Vector3>{Vector3(9.0f, -7.0f, 11.0f),
                                                       Vector3(11.0f, -7.0f, 11.0f),
                                                       Vector3(13.0f, -7.0f, 11.0f),
                                                       Vector3(4.0f, 0.0f, 0.0f)},
                  "left-overlapping transforms read each untouched source before writing it");

        std::vector<Vector3> invalidDestination = {
            Vector3(9.0f), Vector3(8.0f), Vector3(7.0f)};
        const std::vector<Vector3> invalidGuards = invalidDestination;
        bool negativeSourceRejected = false;
        bool shortSourceRejected = false;
        bool shortDestinationRejected = false;
        try { Vector3::Transform(source, -1, scaleAndTranslate, invalidDestination, 0, 1); }
        catch (...) { negativeSourceRejected = true; }
        try { Vector3::Transform(source, 2, scaleAndTranslate, invalidDestination, 0, 2); }
        catch (...) { shortSourceRejected = true; }
        try { Vector3::Transform(source, 0, scaleAndTranslate, invalidDestination, 2, 2); }
        catch (...) { shortDestinationRejected = true; }
        ctx.Check(negativeSourceRejected && shortSourceRejected && shortDestinationRejected &&
                      invalidDestination == invalidGuards,
                  "invalid source and destination windows are rejected before any write");

        bool negativeLengthAccepted = true;
        try { Vector3::Transform(source, 0, scaleAndTranslate, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        try { Vector3::Transform(source, 0, quarterTurn, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        try { Vector3::TransformNormal(source, 0, scaleAndTranslate, invalidDestination, 0, -1); }
        catch (...) { negativeLengthAccepted = false; }
        ctx.Check(negativeLengthAccepted && invalidDestination == invalidGuards,
                  "XNA accepts negative matrix, quaternion and normal lengths as empty ranges");

        bool zeroAtEndAccepted = true;
        try
        {
            Vector3::Transform(source, static_cast<int>(source.size()), scaleAndTranslate,
                               invalidDestination, static_cast<int>(invalidDestination.size()), 0);
            Vector3::Transform(source, static_cast<int>(source.size()), quarterTurn,
                               invalidDestination, static_cast<int>(invalidDestination.size()), 0);
            Vector3::TransformNormal(source, static_cast<int>(source.size()), scaleAndTranslate,
                                     invalidDestination, static_cast<int>(invalidDestination.size()), 0);
        }
        catch (...) { zeroAtEndAccepted = false; }
        ctx.Check(zeroAtEndAccepted && invalidDestination == invalidGuards,
                  "a zero-length transform accepts both one-past-end indices without writing");
    }
}

CNA_DEMOS_CONSOLE(Vector3TransformArray, {
    .id = "math.vector3.transform_array",
    .title = "Transform an array of points",
    .description = "Transforms whole and ranged Vector3 arrays with matrices and quaternions, including normals, "
                   "overlap and invalid-window behavior.",
    .apis = {"Vector3::Transform", "Vector3::TransformNormal", "Matrix::CreateScale",
             "Matrix::CreateTranslation", "Quaternion::CreateFromAxisAngle"},
    .tags = {"matrix", "quaternion", "batch", "arrays"},
})
