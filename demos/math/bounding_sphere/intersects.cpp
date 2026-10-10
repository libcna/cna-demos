// SPDX-License-Identifier: MS-PL
// Collision tests between spheres and boxes: BoundingSphere::Intersects.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
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

        ctx.Print("collision tests done for {}", ball.ToString());
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereIntersects, {
    .id = "math.bounding_sphere.intersects",
    .title = "Sphere collision tests",
    .description = "Tests a BoundingSphere against other spheres and against boxes with "
                   "BoundingSphere::Intersects.",
    .apis = {"BoundingSphere::Intersects", "BoundingSphere::BoundingSphere"},
    .tags = {"collision", "sphere"},
})
