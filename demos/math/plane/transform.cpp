// SPDX-License-Identifier: MS-PL
// Moving and rotating a plane with Plane::Transform; normalizing with Plane::Normalize.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void PlaneTransform(CnaDemos::DemoContext& ctx)
    {
        const Plane ground(Vector3::Up, 0.0f);   // y = 0

        // Raise the plane by 5 units.
        const Plane raised = Plane::Transform(ground, Matrix::CreateTranslation(0.0f, 5.0f, 0.0f));
        ctx.Print("ground raised by 5 = {}", raised.ToString());
        ctx.CheckNear(raised.DotCoordinate(Vector3(3.0f, 5.0f, -2.0f)), 0.0, 1e-5, "points at height 5 lie on the moved plane");

        // Tilt it a quarter turn about Z with a quaternion: the floor becomes a wall.
        const Plane wall = Plane::Transform(ground, Quaternion::CreateFromAxisAngle(Vector3::UnitZ, MathHelper::PiOver2));
        ctx.Print("ground rotated 90 about Z = {}", wall.ToString());
        ctx.CheckNear(Vector3::Distance(wall.Normal, -Vector3::UnitX), 0.0, 1e-6, "Up rotated about Z points to -X");

        // A plane built from an unnormalized normal measures scaled distances until normalized.
        const Plane scaled(0.0f, 2.0f, 0.0f, -4.0f);   // 2y - 4 = 0, i.e. y = 2
        const Plane unit = Plane::Normalize(scaled);
        ctx.Print("Normalize({}) = {}", scaled.ToString(), unit.ToString());
        ctx.CheckNear(unit.Normal.Length(), 1.0, 1e-6, "the normal becomes unit length");
        ctx.CheckNear(unit.DotCoordinate(Vector3(0.0f, 5.0f, 0.0f)), 3.0, 1e-6, "DotCoordinate is then a true distance");
    }
}

CNA_DEMOS_CONSOLE(PlaneTransform, {
    .id = "math.plane.transform",
    .title = "Transform and normalize a plane",
    .description = "Moves a Plane with a matrix and rotates it with a quaternion using Plane::Transform, and fixes "
                   "an unnormalized plane with Plane::Normalize.",
    .apis = {"Plane::Transform", "Plane::Normalize", "Plane::Plane"},
    .tags = {"plane", "transform"},
})
