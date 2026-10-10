// SPDX-License-Identifier: MS-PL
// Directions ignore translation: Vector3::TransformNormal versus Vector3::Transform.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector3TransformNormal(CnaDemos::DemoContext& ctx)
    {
        const Matrix world = Matrix::CreateRotationZ(MathHelper::PiOver2) * Matrix::CreateTranslation(100.0f, 0.0f, 0.0f);
        const Vector3 direction = Vector3::UnitX;

        // Transform treats the vector as a position: it is rotated AND moved.
        const Vector3 asPosition = Vector3::Transform(direction, world);
        // TransformNormal treats it as a direction: rotation and scale only.
        const Vector3 asDirection = Vector3::TransformNormal(direction, world);

        ctx.Print("Transform(UnitX, world)       = {}", asPosition.ToString());
        ctx.Print("TransformNormal(UnitX, world) = {}", asDirection.ToString());
        ctx.CheckNear(Vector3::Distance(asPosition, Vector3(100.0f, 1.0f, 0.0f)), 0.0, 1e-4, "Transform includes the translation");
        ctx.CheckNear(Vector3::Distance(asDirection, Vector3::UnitY), 0.0, 1e-6, "TransformNormal only rotates");
        // Note: for matrices with non-uniform scale, surface normals need the inverse transpose.
    }
}

CNA_DEMOS_CONSOLE(Vector3TransformNormal, {
    .id = "math.vector3.transform_normal",
    .title = "Transform a direction",
    .description = "Rotates a direction with Vector3::TransformNormal, which ignores the matrix translation, "
                   "and contrasts it with Vector3::Transform for positions.",
    .apis = {"Vector3::TransformNormal", "Vector3::Transform", "Matrix::CreateRotationZ"},
    .tags = {"matrix", "direction", "normal"},
})
