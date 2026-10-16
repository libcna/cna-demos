// SPDX-License-Identifier: MS-PL
// Collision tests of an axis-aligned box: BoundingBox::Intersects against boxes, spheres, rays and
// planes.
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/PlaneIntersectionType.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingBoxIntersects(CnaDemos::DemoContext& ctx)
    {
        // A crate from (0, 0, 0) to (2, 2, 2).
        const BoundingBox crate(Vector3::Zero, Vector3(2.0f, 2.0f, 2.0f));

        // Box-box: overlapping on all three axes. Boxes that only touch count as intersecting.
        ctx.Check(crate.Intersects(BoundingBox(Vector3(1.0f, 1.0f, 1.0f), Vector3(3.0f, 3.0f, 3.0f))),
                  "an overlapping crate intersects");
        ctx.Check(crate.Intersects(BoundingBox(Vector3(2.0f, 0.0f, 0.0f), Vector3(4.0f, 2.0f, 2.0f))),
                  "a crate touching the face intersects");
        ctx.Check(!crate.Intersects(BoundingBox(Vector3(1.0f, 3.0f, 1.0f), Vector3(2.0f, 4.0f, 2.0f))),
                  "a crate above it does not (separated on Y)");

        // Box-sphere: a ball near the corner.
        ctx.Check(crate.Intersects(BoundingSphere(Vector3(2.5f, 2.5f, 1.0f), 0.8f)), "a ball over the edge intersects");
        ctx.Check(!crate.Intersects(BoundingSphere(Vector3(2.6f, 2.6f, 2.6f), 0.8f)),
                  "a ball off the corner does not (1.04 away)");

        // Box-ray: the distance along the ray to the entry point, or nothing -- a bullet or a
        // mouse pick.
        const std::optional<float> hit = crate.Intersects(Ray(Vector3(-3.0f, 1.0f, 1.0f), Vector3::UnitX));
        const std::optional<float> miss = crate.Intersects(Ray(Vector3(-3.0f, 3.0f, 1.0f), Vector3::UnitX));
        ctx.Print("ray along +X at y=1: hit at distance {}", hit.has_value() ? *hit : -1.0f);
        ctx.Check(hit.has_value() && *hit == 3.0f, "a ray towards the crate hits it 3 units away");
        ctx.Check(!miss.has_value(), "a ray passing above it misses");
        const std::optional<float> inside = crate.Intersects(Ray(Vector3(1.0f, 1.0f, 1.0f), Vector3::UnitY));
        ctx.Check(inside.has_value() && *inside == 0.0f, "a ray starting inside hits at distance 0");

        // Box-plane: which side of the floor (y = 0.5, normal up) the crate is on.
        const Plane floorAt(Vector3::Up, -0.5f);
        const Plane floorBelow(Vector3::Up, 1.0f);   // y = -1
        const Plane ceiling(Vector3::Up, -5.0f);     // y = 5
        ctx.Check(crate.Intersects(floorAt) == PlaneIntersectionType::Intersecting, "a plane through it intersects");
        ctx.Check(crate.Intersects(floorBelow) == PlaneIntersectionType::Front,
                  "the crate is in front of a plane below it");
        ctx.Check(crate.Intersects(ceiling) == PlaneIntersectionType::Back, "and behind a plane above it");

        const BoundingBox overlapBox(Vector3(1.0f), Vector3(3.0f));
        const BoundingBox touchingBox(Vector3(2.0f, 0.0f, 0.0f), Vector3(4.0f, 2.0f, 2.0f));
        const BoundingBox missedBox(Vector3(3.0f), Vector3(4.0f));
        bool overlapOutput = false;
        bool touchingOutput = false;
        bool missedOutput = true;
        crate.Intersects(overlapBox, overlapOutput);
        crate.Intersects(touchingBox, touchingOutput);
        crate.Intersects(missedBox, missedOutput);
        ctx.Check(overlapOutput && touchingOutput && !missedOutput,
                  "box output intersections preserve overlap, contact and separation");

        const BoundingSphere tangentSphere(Vector3(3.0f, 1.0f, 1.0f), 1.0f);
        const BoundingSphere missedSphere(Vector3(3.01f, 1.0f, 1.0f), 1.0f);
        bool tangentSphereOutput = false;
        bool missedSphereOutput = true;
        crate.Intersects(tangentSphere, tangentSphereOutput);
        crate.Intersects(missedSphere, missedSphereOutput);
        ctx.Check(tangentSphereOutput && !missedSphereOutput,
                  "sphere output intersections include exact tangency and exclude a gap");

        std::optional<float> rayHitOutput;
        std::optional<float> rayMissOutput = 99.0f;
        const Ray outputHitRay(Vector3(-3.0f, 1.0f, 1.0f), Vector3::UnitX);
        const Ray outputMissRay(Vector3(-3.0f, 3.0f, 1.0f), Vector3::UnitX);
        crate.Intersects(outputHitRay, rayHitOutput);
        crate.Intersects(outputMissRay, rayMissOutput);
        ctx.Check(rayHitOutput == 3.0f && !rayMissOutput.has_value(),
                  "ray output intersections overwrite hit distance and miss state");

        PlaneIntersectionType frontOutput = PlaneIntersectionType::Back;
        PlaneIntersectionType crossingOutput = PlaneIntersectionType::Back;
        PlaneIntersectionType backOutput = PlaneIntersectionType::Front;
        crate.Intersects(floorBelow, frontOutput);
        crate.Intersects(floorAt, crossingOutput);
        crate.Intersects(ceiling, backOutput);
        ctx.Check(frontOutput == PlaneIntersectionType::Front &&
                      crossingOutput == PlaneIntersectionType::Intersecting &&
                      backOutput == PlaneIntersectionType::Back,
                  "plane output intersections overwrite all three classifications");

        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());
        const BoundingBox huge(Vector3(-2.0f), Vector3(2.0f));
        const BoundingBox half(Vector3(-2.0f), Vector3(0.0f, 2.0f, 2.0f));
        const BoundingBox far(Vector3(10.0f), Vector3(11.0f));
        ctx.Check(huge.Intersects(identityFrustum) && half.Intersects(identityFrustum) &&
                      !far.Intersects(identityFrustum),
                  "frustum intersection accepts enclosing and clipped boxes but rejects a distant box");
    }
}

CNA_DEMOS_CONSOLE(BoundingBoxIntersects, {
    .id = "math.bounding_box.intersects",
    .title = "Box collision tests",
    .description = "Tests BoundingBox intersections with boxes, spheres, rays, planes and a frustum through value "
                   "and output forms, including contact, miss and all plane classifications.",
    .apis = {"BoundingBox::Intersects", "BoundingBox::BoundingBox", "PlaneIntersectionType", "PlaneIntersectionType::Back", "Ray::Ray",
             "Plane::Plane"},
    .tags = {"collision", "box", "aabb", "ray", "picking"},
})
