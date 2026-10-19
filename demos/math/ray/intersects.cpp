// SPDX-License-Identifier: MS-PL
// Ray casting: Ray::Intersects returns the distance to a box, sphere or plane (or nothing).
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RayIntersects(CnaDemos::DemoContext& ctx)
    {
        // A shot fired from the origin along +X.
        const Ray shot(Vector3::Zero, Vector3::UnitX);

        const std::optional<float> toCrate = shot.Intersects(BoundingBox(Vector3(4.0f, -1.0f, -1.0f), Vector3(6.0f, 1.0f, 1.0f)));
        const std::optional<float> toBall = shot.Intersects(BoundingSphere(Vector3(10.0f, 0.0f, 0.0f), 2.0f));
        const std::optional<float> toWall = shot.Intersects(Plane(-Vector3::UnitX, 20.0f));   // x = 20
        const std::optional<float> missed = shot.Intersects(BoundingSphere(Vector3(0.0f, 5.0f, 0.0f), 1.0f));

        ctx.Print("crate: {}", toCrate ? std::to_string(*toCrate) : "miss");
        ctx.Print("ball : {}", toBall ? std::to_string(*toBall) : "miss");
        ctx.Print("wall : {}", toWall ? std::to_string(*toWall) : "miss");
        ctx.Print("off-axis ball: {}", missed ? std::to_string(*missed) : "miss");

        ctx.Check(toCrate && *toCrate == 4.0f, "the crate's near face is 4 units away");
        ctx.Check(toBall && *toBall == 8.0f, "the ball's surface is 8 units away");
        ctx.Check(toWall && *toWall == 20.0f, "the wall is 20 units away");
        ctx.Check(!missed, "a sphere off the ray's line gives no value");

        // The hit point is Position + Direction * distance (Direction should be unit length).
        const Vector3 hit = shot.Position + shot.Direction * *toBall;
        ctx.Check(hit == Vector3(8.0f, 0.0f, 0.0f), "hit point on the ball's surface");

        // Starting inside a volume reports distance 0.
        const Ray inside(Vector3(5.0f, 0.0f, 0.0f), Vector3::UnitX);
        ctx.Check(inside.Intersects(BoundingBox(Vector3(4.0f, -1.0f, -1.0f), Vector3(6.0f, 1.0f, 1.0f))) == 0.0f,
                  "a ray starting inside the box hits at distance 0");

        const BoundingBox box(Vector3(4.0f, -1.0f, -1.0f), Vector3(6.0f, 1.0f, 1.0f));
        const Ray boxMiss(Vector3(0.0f, 2.0f, 0.0f), Vector3::UnitX);
        std::optional<float> boxHitOutput;
        std::optional<float> boxMissOutput = 99.0f;
        std::optional<float> boxInsideOutput = 99.0f;
        shot.Intersects(box, boxHitOutput);
        boxMiss.Intersects(box, boxMissOutput);
        inside.Intersects(box, boxInsideOutput);
        ctx.Check(boxHitOutput == 4.0f && !boxMissOutput.has_value() && boxInsideOutput == 0.0f &&
                      Ray(Vector3(10.0f, 0.0f, 0.0f), -Vector3::UnitX).Intersects(box) == 4.0f &&
                      !Ray(Vector3::Zero, -Vector3::UnitX).Intersects(box).has_value(),
                  "box output, reverse-direction and away-facing paths preserve hit parameters");

        const Ray zeroInsideBox(Vector3(5.0f, 0.0f, 0.0f), Vector3::Zero);
        const Ray zeroOutsideBox(Vector3::Zero, Vector3::Zero);
        ctx.Check(zeroInsideBox.Intersects(box) == 0.0f &&
                      !zeroOutsideBox.Intersects(box).has_value(),
                  "a zero-direction ray reports zero only when its position is inside the box");
        ctx.Check(Ray(Vector3::Zero, Vector3(2.0f, 0.0f, 0.0f)).Intersects(box) == 2.0f,
                  "box intersection returns the authored-direction parameter without normalization");

        const BoundingSphere sphere(Vector3(10.0f, 0.0f, 0.0f), 2.0f);
        const Ray sphereMiss(Vector3::Zero, Vector3::UnitY);
        std::optional<float> sphereHitOutput;
        std::optional<float> sphereMissOutput = 99.0f;
        shot.Intersects(sphere, sphereHitOutput);
        sphereMiss.Intersects(sphere, sphereMissOutput);
        ctx.Check(sphereHitOutput == 8.0f && !sphereMissOutput.has_value() &&
                      Ray(Vector3(10.0f, 0.0f, 0.0f), Vector3::UnitX).Intersects(sphere) == 0.0f &&
                      shot.Intersects(BoundingSphere(Vector3(10.0f, 1.0f, 0.0f), 1.0f)) == 10.0f &&
                      !shot.Intersects(BoundingSphere(Vector3(-10.0f, 0.0f, 0.0f), 2.0f)).has_value(),
                  "sphere output handles hits, misses, interior starts, tangency and behind-ray bounds");
        ctx.Check(Ray(Vector3(12.0f, 0.0f, 0.0f), Vector3::Zero).Intersects(sphere) == 0.0f,
                  "a zero-direction ray on the sphere surface reports parameter zero");
        ctx.CheckNear(*Ray(Vector3::Zero, Vector3(2.0f, 0.0f, 0.0f)).Intersects(sphere),
                      2.56440353f, 1e-6,
                      "sphere intersection consumes a nonunit direction exactly as authored");

        const Plane wall(-Vector3::UnitX, 20.0f);
        const Plane parallel(Vector3::UnitY, -2.0f);
        std::optional<float> planeHitOutput;
        std::optional<float> parallelOutput = 99.0f;
        shot.Intersects(wall, planeHitOutput);
        shot.Intersects(parallel, parallelOutput);
        ctx.Check(planeHitOutput == 20.0f && !parallelOutput.has_value() &&
                      !shot.Intersects(Plane(Vector3::UnitX, 1.0f)).has_value() &&
                      shot.Intersects(Plane(Vector3::UnitX, 0.0f)) == 0.0f,
                  "plane output distinguishes forward, parallel, behind and on-plane starts");
        ctx.Check(Ray(Vector3(0.000001f, 0.0f, 0.0f), Vector3::UnitX)
                          .Intersects(Plane(Vector3::UnitX, 0.0f)) == 0.0f,
                  "a tiny negative plane parameter clamps to zero within XNA's tolerance");
        ctx.Check(Ray(Vector3::Zero, Vector3(2.0f, 0.0f, 0.0f)).Intersects(wall) == 10.0f,
                  "plane intersection scales its parameter for a nonunit direction");

    }
}

CNA_DEMOS_CONSOLE(RayIntersects, {
    .id = "math.ray.intersects",
    .title = "Ray casting",
    .description = "Casts unit, nonunit and zero-direction Rays against boxes, spheres and planes through value/output "
                   "forms, including inside, tangent, behind, tolerance and miss boundaries.",
    .apis = {"Ray::Intersects"},
    .tags = {"ray", "picking", "collision"},
})
