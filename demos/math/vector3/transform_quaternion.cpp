// SPDX-License-Identifier: MS-PL
// Rotating a vector by a Quaternion with Vector3::Transform.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector3TransformQuaternion(CnaDemos::DemoContext& ctx)
    {
        // A quarter turn about the Up axis.
        const Quaternion rotation = Quaternion::CreateFromAxisAngle(Vector3::Up, MathHelper::PiOver2);
        const Vector3 rotated = Vector3::Transform(Vector3::Forward, rotation);
        ctx.Print("Forward rotated 90 degrees about Up = {}", rotated.ToString());
        ctx.CheckNear(Vector3::Distance(rotated, Vector3::Left), 0.0, 1e-6, "a quarter turn about Up turns Forward into Left");

        // The same rotation as a matrix gives the same result.
        const Vector3 viaMatrix = Vector3::Transform(Vector3::Forward, Matrix::CreateFromQuaternion(rotation));
        ctx.CheckNear(Vector3::Distance(rotated, viaMatrix), 0.0, 1e-6, "quaternion and matrix rotations agree");
    }
}

CNA_DEMOS_CONSOLE(Vector3TransformQuaternion, {
    .id = "math.vector3.transform_quaternion",
    .title = "Rotate a vector by a quaternion",
    .description = "Rotates a direction with the Vector3::Transform overload that takes a Quaternion and checks "
                   "it against the equivalent rotation matrix.",
    .apis = {"Vector3::Transform", "Quaternion::CreateFromAxisAngle", "Matrix::CreateFromQuaternion"},
    .tags = {"quaternion", "rotation"},
})
