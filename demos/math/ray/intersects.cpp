// SPDX-License-Identifier: MS-PL
// Ray casting: Ray::Intersects returns the distance to a box, sphere or plane (or nothing).
#include <any>
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

        const Ray shotCopy(Vector3::Zero, Vector3::UnitX);
        ctx.Check(shot.Equals(shotCopy) && shot.Equals(std::any(shotCopy)) && shot == shotCopy &&
                      !(shot != shotCopy) && shot.GetHashCode() == shotCopy.GetHashCode() &&
                      !shot.Equals(std::any(17)) && !shot.Equals(std::any{}),
                  "equal rays agree through typed and boxed equality, operators and hashing");
        const Ray turned(Vector3::Zero, Vector3::UnitY);
        ctx.Check(turned != shot && !turned.Equals(shot),
                  "changing the direction makes two rays unequal");

        // The hit point is Position + Direction * distance (Direction should be unit length).
        const Vector3 hit = shot.Position + shot.Direction * *toBall;
        ctx.Check(hit == Vector3(8.0f, 0.0f, 0.0f), "hit point on the ball's surface");

        // Starting inside a volume reports distance 0.
        const Ray inside(Vector3(5.0f, 0.0f, 0.0f), Vector3::UnitX);
        ctx.Check(inside.Intersects(BoundingBox(Vector3(4.0f, -1.0f, -1.0f), Vector3(6.0f, 1.0f, 1.0f))) == 0.0f,
                  "a ray starting inside the box hits at distance 0");
    }
}

CNA_DEMOS_CONSOLE(RayIntersects, {
    .id = "math.ray.intersects",
    .title = "Ray casting",
    .description = "Casts a Ray against a box, spheres and a plane with Ray::Intersects, which returns the "
                   "distance to the first hit or no value, and computes the hit point.",
    .apis = {"Ray::Ray", "Ray::Intersects", "Ray::Position", "Ray::Direction", "Ray::Equals",
             "Ray::GetHashCode", "Ray::op_Equality", "Ray::op_Inequality"},
    .tags = {"ray", "picking", "collision"},
})
