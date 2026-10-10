// SPDX-License-Identifier: MS-PL
// BoundingBox::Contains and the three ContainmentType answers.
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    std::string_view Name(ContainmentType type)
    {
        switch (type)
        {
            case ContainmentType::Disjoint: return "Disjoint";
            case ContainmentType::Contains: return "Contains";
            case ContainmentType::Intersects: return "Intersects";
        }
        return "?";
    }

    void BoundingBoxContains(CnaDemos::DemoContext& ctx)
    {
        const BoundingBox room(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 3.0f, 10.0f));

        const auto point = room.Contains(Vector3(5.0f, 1.0f, 5.0f));
        const auto inner = room.Contains(BoundingBox(Vector3(1.0f, 0.0f, 1.0f), Vector3(2.0f, 1.0f, 2.0f)));
        const auto crossing = room.Contains(BoundingSphere(Vector3(10.0f, 1.0f, 5.0f), 1.0f));
        const auto outside = room.Contains(BoundingSphere(Vector3(20.0f, 1.0f, 5.0f), 1.0f));
        ctx.Print("point in the middle     : {}", Name(point));
        ctx.Print("small box in a corner   : {}", Name(inner));
        ctx.Print("sphere through the wall : {}", Name(crossing));
        ctx.Print("sphere outside          : {}", Name(outside));

        ctx.Check(point == ContainmentType::Contains, "a point inside is Contains");
        ctx.Check(inner == ContainmentType::Contains, "a box fully inside is Contains");
        ctx.Check(crossing == ContainmentType::Intersects, "a sphere crossing the boundary is Intersects");
        ctx.Check(outside == ContainmentType::Disjoint, "a sphere outside is Disjoint");
    }
}

CNA_DEMOS_CONSOLE(BoundingBoxContains, {
    .id = "math.bounding_box.contains",
    .title = "Containment tests",
    .description = "Asks a BoundingBox whether it contains a point, a box and spheres, and prints the "
                   "ContainmentType answer: Contains, Intersects or Disjoint.",
    .apis = {"BoundingBox::Contains", "ContainmentType::Contains", "ContainmentType::Intersects",
             "ContainmentType::Disjoint"},
    .tags = {"bounds", "containment", "collision"},
})
