// SPDX-License-Identifier: MS-PL
// Transforming many points at once with the array overloads of Vector3::Transform.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
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
    }
}

CNA_DEMOS_CONSOLE(Vector3TransformArray, {
    .id = "math.vector3.transform_array",
    .title = "Transform an array of points",
    .description = "Transforms a whole std::vector of positions with one call to the array overload of "
                   "Vector3::Transform.",
    .apis = {"Vector3::Transform", "Matrix::CreateScale", "Matrix::CreateTranslation"},
    .tags = {"matrix", "batch"},
})
