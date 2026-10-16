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

        const BoundingSphere signedLocal(Vector3(1.0f, 2.0f, 3.0f), 2.0f);
        const Matrix signedNonuniform = Matrix::CreateScale(2.0f, -3.0f, 4.0f) *
                                        Matrix::CreateTranslation(5.0f, -7.0f, 11.0f);
        const BoundingSphere signedPlaced = signedLocal.Transform(signedNonuniform);
        ctx.Check(signedPlaced.Center == Vector3(7.0f, -13.0f, 23.0f) && signedPlaced.Radius == 8.0f,
                  "nonuniform signed scale uses the largest absolute axis for the radius");

        BoundingSphere output;
        signedLocal.Transform(signedNonuniform, output);
        BoundingSphere alias = signedLocal;
        alias.Transform(signedNonuniform, alias);
        ctx.Check(output == signedPlaced && alias == signedPlaced,
                  "the Transform output form agrees and may alias its source");

        const Matrix zeroScale = Matrix::CreateScale(0.0f) *
                                 Matrix::CreateTranslation(5.0f, -7.0f, 11.0f);
        const BoundingSphere collapsed = signedLocal.Transform(zeroScale);
        ctx.Check(collapsed.Center == Vector3(5.0f, -7.0f, 11.0f) && collapsed.Radius == 0.0f,
                  "zero scale collapses the radius while retaining translation");
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereTransform, {
    .id = "math.bounding_sphere.transform",
    .title = "Transform a bounding sphere",
    .description = "Transforms BoundingSphere values through uniform, signed nonuniform and zero scale using "
                   "value, output and source-alias forms.",
    .apis = {"BoundingSphere::Transform"},
    .tags = {"bounds", "sphere", "world"},
})
