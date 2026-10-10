// SPDX-License-Identifier: MS-PL
// An axis-aligned bounding box around a set of points, and its corners.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingBoxFromPoints(CnaDemos::DemoContext& ctx)
    {
        const std::vector<Vector3> vertices = {
            {1.0f, 0.0f, 2.0f}, {-3.0f, 4.0f, 0.5f}, {2.0f, -1.0f, -6.0f}, {0.0f, 2.0f, 1.0f}};

        const BoundingBox box = BoundingBox::CreateFromPoints(vertices);
        ctx.Print("CreateFromPoints -> {}", box.ToString());
        ctx.Check(box.Min == Vector3(-3.0f, -1.0f, -6.0f), "Min is the smallest coordinate on every axis");
        ctx.Check(box.Max == Vector3(2.0f, 4.0f, 2.0f), "Max is the largest coordinate on every axis");

        // The eight corners, e.g. for drawing the box or transforming it.
        const std::vector<Vector3> corners = box.GetCorners();
        ctx.Print("{} corners, first {} last {}", corners.size(), corners.front().ToString(), corners.back().ToString());
        ctx.Check(corners.size() == static_cast<std::size_t>(BoundingBox::CornerCount), "GetCorners returns CornerCount (8) points");

        const BoundingBox manual(Vector3(-3.0f, -1.0f, -6.0f), Vector3(2.0f, 4.0f, 2.0f));
        ctx.Check(manual == box, "the (min, max) constructor builds the same box");
    }
}

CNA_DEMOS_CONSOLE(BoundingBoxFromPoints, {
    .id = "math.bounding_box.create_from_points",
    .title = "Bounding box from points",
    .description = "Builds an axis-aligned BoundingBox around vertices with BoundingBox::CreateFromPoints and "
                   "lists its eight corners with GetCorners.",
    .apis = {"BoundingBox::CreateFromPoints", "BoundingBox::GetCorners", "BoundingBox::CornerCount",
             "BoundingBox::Min", "BoundingBox::Max", "BoundingBox::BoundingBox"},
    .tags = {"bounds", "aabb", "collision"},
})
