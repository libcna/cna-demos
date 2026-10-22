// SPDX-License-Identifier: MS-PL
// Ray entry, miss and interior intersections with a BoundingFrustum.
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RayFrustumIntersection(CnaDemos::DemoContext& ctx)
    {
        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());

        const Ray entering(Vector3(0.0f, 0.0f, -3.0f), Vector3::UnitZ);
        const std::optional<float> entry = entering.Intersects(identityFrustum);
        ctx.Print("entry from z=-3: {}", entry.has_value() ? std::to_string(*entry) : "miss");
        ctx.Check(entry == 3.0f,
                  "XNA computes the first frustum entry for a ray starting outside");

        const Ray missed(Vector3(3.0f, 0.0f, -3.0f), Vector3::UnitZ);
        ctx.Check(!missed.Intersects(identityFrustum).has_value(),
                  "a parallel offset ray misses the identity frustum");

        const Ray inside(Vector3(0.0f, 0.0f, 0.5f), Vector3::UnitZ);
        ctx.Check(inside.Intersects(identityFrustum) == 0.0f,
                  "a ray starting inside the frustum reports parameter zero");
    }
}

CNA_DEMOS_CONSOLE(RayFrustumIntersection, {
    .id = "math.ray.frustum_intersection",
    .title = "Ray and frustum intersection",
    .description = "Casts Rays from outside, beside and inside an identity BoundingFrustum and observes the first "
                   "entry parameter or a miss.",
    .apis = {"Ray::Intersects", "BoundingFrustum::BoundingFrustum"},
    .tags = {"ray", "frustum", "picking", "collision"},
})
