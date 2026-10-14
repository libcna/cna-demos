// SPDX-License-Identifier: MS-PL
// Planes: construction, signed distance and which side things are on.
#include <any>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/PlaneIntersectionType.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    std::string_view Name(PlaneIntersectionType side)
    {
        switch (side)
        {
            case PlaneIntersectionType::Front: return "Front";
            case PlaneIntersectionType::Back: return "Back";
            case PlaneIntersectionType::Intersecting: return "Intersecting";
        }
        return "?";
    }

    void PlaneClassify(CnaDemos::DemoContext& ctx)
    {
        // A floor at height 2, given by normal and distance: points p with Dot(Normal, p) + D = 0.
        const Plane floor(Vector3::Up, -2.0f);
        // The same plane from three points on it (counter-clockwise seen from above).
        const Plane fromPoints(Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 2.0f, 1.0f), Vector3(1.0f, 2.0f, 0.0f));
        ctx.Print("Plane(Up, -2)        = {}", floor.ToString());
        ctx.Print("Plane(three points)  = {}", fromPoints.ToString());
        ctx.CheckNear(Vector3::Distance(fromPoints.Normal, floor.Normal), 0.0, 1e-6, "both describe the same normal");
        ctx.CheckNear(fromPoints.D, floor.D, 1e-6, "and the same distance");

        // DotCoordinate is the signed distance of a point (for a unit normal).
        ctx.Check(floor.DotCoordinate(Vector3(5.0f, 7.0f, 1.0f)) == 5.0f, "a point at height 7 is 5 above the floor");
        ctx.Check(floor.DotCoordinate(Vector3(0.0f, 0.0f, 0.0f)) == -2.0f, "the origin is 2 below it");
        // DotNormal ignores D: the projection of a direction onto the normal.
        ctx.Check(floor.DotNormal(Vector3(3.0f, -1.0f, 0.0f)) == -1.0f, "DotNormal measures a direction against the normal");
        // Dot uses W to include (or scale) the plane's distance term.
        ctx.Check(floor.Dot(Vector4(0.0f, 3.0f, 0.0f, 2.0f)) == -1.0f,
                  "Dot evaluates a homogeneous Vector4 including D times W");

        const Plane sameFloor(Vector3::Up, -2.0f);
        const Plane changedFloor(Vector3::Up, -3.0f);
        ctx.Check(floor.Equals(sameFloor) && floor.Equals(std::any{sameFloor}) &&
                      !floor.Equals(std::any{42}) && !floor.Equals(std::any{}),
                  "typed and boxed equality accept only the same normal and distance");
        ctx.Check(floor == sameFloor && floor != changedFloor &&
                      floor.GetHashCode() == sameFloor.GetHashCode(),
                  "plane operators and hashes follow all four coefficients");

        // Side tests for volumes.
        const auto box = floor.Intersects(BoundingBox(Vector3(0.0f, 3.0f, 0.0f), Vector3(1.0f, 4.0f, 1.0f)));
        const auto sphere = floor.Intersects(BoundingSphere(Vector3(0.0f, 2.5f, 0.0f), 1.0f));
        ctx.Print("box above the floor: {}, sphere through it: {}", Name(box), Name(sphere));
        ctx.Check(box == PlaneIntersectionType::Front, "a box above the floor is in Front (the normal side)");
        ctx.Check(sphere == PlaneIntersectionType::Intersecting, "a sphere crossing it is Intersecting");
    }
}

CNA_DEMOS_CONSOLE(PlaneClassify, {
    .id = "math.plane.classify_points",
    .title = "Planes and signed distance",
    .description = "Builds a Plane from a normal and distance and from three points, measures signed distances "
                   "with DotCoordinate and DotNormal, and classifies volumes as Front, Back or Intersecting.",
    .apis = {"Plane::Plane", "Plane::Normal", "Plane::D", "Plane::Dot", "Plane::DotCoordinate",
             "Plane::DotNormal", "Plane::Equals", "Plane::GetHashCode", "Plane::op_Equality",
             "Plane::op_Inequality", "Plane::Intersects", "PlaneIntersectionType::Front",
             "PlaneIntersectionType::Intersecting"},
    .tags = {"plane", "distance", "classification"},
})
