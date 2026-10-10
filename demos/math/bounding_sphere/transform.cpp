// SPDX-License-Identifier: MS-PL
// Moving a model's bounds with its world matrix: BoundingSphere::Transform.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingSphereTransform(CnaDemos::DemoContext& ctx)
    {
        // A model's bounds in its own space.
        const BoundingSphere local(Vector3(0.0f, 1.0f, 0.0f), 1.5f);

        // Placed in the world at three times its size.
        const Matrix world = Matrix::CreateScale(3.0f) * Matrix::CreateRotationY(0.4f) * Matrix::CreateTranslation(10.0f, 0.0f, -4.0f);
        const BoundingSphere placed = local.Transform(world);
        ctx.Print("local  = {}", local.ToString());
        ctx.Print("world  = {}", placed.ToString());

        ctx.CheckNear(Vector3::Distance(placed.Center, Vector3::Transform(local.Center, world)), 0.0, 1e-5,
                      "the centre moves like a point");
        ctx.CheckNear(placed.Radius, 4.5, 1e-5, "the radius scales with the matrix");
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereTransform, {
    .id = "math.bounding_sphere.transform",
    .title = "Transform a bounding sphere",
    .description = "Moves a model-space BoundingSphere into world space with BoundingSphere::Transform; the "
                   "centre is transformed and the radius scaled.",
    .apis = {"BoundingSphere::Transform"},
    .tags = {"bounds", "sphere", "world"},
})
