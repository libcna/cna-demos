// SPDX-License-Identifier: MS-PL
// Collision tests between spheres and boxes: BoundingSphere::Intersects.
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
    void BoundingSphereIntersects(CnaDemos::DemoContext& ctx)
    {
        const BoundingSphere ball(Vector3(0.0f, 0.0f, 0.0f), 1.0f);

        // Sphere-sphere: overlapping when the centres are closer than the sum of the radii.
        ctx.Check(ball.Intersects(BoundingSphere(Vector3(1.5f, 0.0f, 0.0f), 1.0f)), "spheres 1.5 apart overlap");
        ctx.Check(!ball.Intersects(BoundingSphere(Vector3(3.0f, 0.0f, 0.0f), 1.0f)), "spheres 3 apart do not");

        // Sphere-box: e.g. a ball against a crate.
        const BoundingBox crate(Vector3(0.5f, -1.0f, -1.0f), Vector3(2.5f, 1.0f, 1.0f));
        ctx.Check(ball.Intersects(crate), "the ball touches the crate");
        ctx.Check(!ball.Intersects(BoundingBox(Vector3(1.0f, 1.0f, 1.0f), Vector3(2.0f, 2.0f, 2.0f))),
                  "a box past the diagonal is missed (the sphere is round)");

        const BoundingSphere outer(Vector3::Zero, 5.0f);
        const BoundingSphere overlapping(Vector3(5.5f, 0.0f, 0.0f), 1.0f);
        const BoundingSphere missedSphere(Vector3(7.0f, 0.0f, 0.0f), 1.0f);
        bool overlappingOutput = false;
        bool missedSphereOutput = true;
        outer.Intersects(overlapping, overlappingOutput);
        outer.Intersects(missedSphere, missedSphereOutput);
        ctx.Check(overlappingOutput && !missedSphereOutput,
                  "sphere output intersections overwrite overlap and miss results");

        const BoundingSphere tangentSphere(Vector3(6.0f, 0.0f, 0.0f), 1.0f);
        bool tangentSphereOutput = true;
        outer.Intersects(tangentSphere, tangentSphereOutput);
        ctx.Check(!outer.Intersects(tangentSphere) && !tangentSphereOutput,
                  "XNA excludes exact external sphere tangency from Intersects");

        const BoundingBox tangentBox(Vector3(5.0f, -1.0f, -1.0f), Vector3(6.0f, 1.0f, 1.0f));
        const BoundingBox missedBox(Vector3(6.0f), Vector3(7.0f));
        bool tangentBoxOutput = false;
        bool missedBoxOutput = true;
        outer.Intersects(tangentBox, tangentBoxOutput);
        outer.Intersects(missedBox, missedBoxOutput);
        ctx.Check(tangentBoxOutput && !missedBoxOutput,
                  "box output intersections include exact contact and exclude a gap");

        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());
        const BoundingSphere huge(Vector3::Zero, 5.0f);
        const BoundingSphere small(Vector3::Zero, 1.0f);
        const BoundingSphere far(Vector3(20.0f), 1.0f);
        ctx.Check(huge.Intersects(identityFrustum) && small.Intersects(identityFrustum) &&
                      !far.Intersects(identityFrustum),
                  "frustum intersection accepts enclosing and clipped spheres but rejects a distant sphere");

        const Ray hitRay(Vector3(-10.0f, 0.0f, 0.0f), Vector3::UnitX);
        const Ray missRay(Vector3(-10.0f, 6.0f, 0.0f), Vector3::UnitX);
        std::optional<float> hitOutput;
        std::optional<float> missOutput = 99.0f;
        outer.Intersects(hitRay, hitOutput);
        outer.Intersects(missRay, missOutput);
        ctx.Check(hitOutput == 5.0f && !missOutput.has_value(),
                  "ray output intersections overwrite hit distance and miss state");

        const Plane frontPlane(Vector3::Up, 6.0f);
        const Plane crossingPlane(Vector3::Up, 0.0f);
        const Plane backPlane(Vector3::Up, -6.0f);
        PlaneIntersectionType frontOutput = PlaneIntersectionType::Back;
        PlaneIntersectionType crossingOutput = PlaneIntersectionType::Back;
        PlaneIntersectionType backOutput = PlaneIntersectionType::Front;
        outer.Intersects(frontPlane, frontOutput);
        outer.Intersects(crossingPlane, crossingOutput);
        outer.Intersects(backPlane, backOutput);
        ctx.Check(frontOutput == PlaneIntersectionType::Front &&
                      crossingOutput == PlaneIntersectionType::Intersecting &&
                      backOutput == PlaneIntersectionType::Back,
                  "plane output intersections overwrite Front, Intersecting and Back");

        ctx.Print("collision tests done for {}", ball.ToString());
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereIntersects, {
    .id = "math.bounding_sphere.intersects",
    .title = "Sphere collision tests",
    .description = "Tests BoundingSphere intersections with spheres, boxes, a frustum, rays and planes through "
                   "value and output forms, including exact contact and miss boundaries.",
    .apis = {"BoundingSphere::Intersects", "BoundingSphere::BoundingSphere"},
    .tags = {"collision", "sphere"},
    .restrictions = "CNA-DEMOS-ISSUE-129 records exact sphere-tangency classification.",
    .knownIssue = "CNA-DEMOS-ISSUE-129",
})
