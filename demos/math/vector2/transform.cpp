// SPDX-License-Identifier: MS-PL
// 2D rotation about a pivot with Vector2::Transform.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector2TransformDemo(CnaDemos::DemoContext& ctx)
    {
        // Rotate a point 90 degrees about the pivot (100, 100) in screen space. In 2D the
        // rotation is about the Z axis. The pivot is moved to the origin, rotated and moved back.
        const Vector2 pivot(100.0f, 100.0f);
        const Vector2 point(150.0f, 100.0f);
        const Matrix rotateAboutPivot = Matrix::CreateTranslation(-pivot.X, -pivot.Y, 0.0f)
                                        * Matrix::CreateRotationZ(MathHelper::PiOver2)
                                        * Matrix::CreateTranslation(pivot.X, pivot.Y, 0.0f);

        const Vector2 rotated = Vector2::Transform(point, rotateAboutPivot);
        ctx.Print("Transform({}, rotate 90 about {}) = {}", point.ToString(), pivot.ToString(), rotated.ToString());

        // With +Y pointing down the screen, a positive Z rotation turns +X towards +Y (clockwise
        // on screen).
        ctx.CheckNear(Vector2::Distance(rotated, Vector2(100.0f, 150.0f)), 0.0, 1e-4,
                      "the point 50 px right of the pivot ends 50 px below it");

        const Vector2 direction = Vector2::TransformNormal(Vector2::UnitX, rotateAboutPivot);
        ctx.CheckNear(Vector2::Distance(direction, Vector2::UnitY), 0.0, 1e-6,
                      "TransformNormal rotates a direction without applying the pivot translations");
    }
}

CNA_DEMOS_CONSOLE(Vector2TransformDemo, {
    .id = "math.vector2.transform",
    .title = "Rotate a 2D point about a pivot",
    .description = "Builds translate-rotate-translate matrices and applies them to a screen-space point with "
                   "Vector2::Transform.",
    .apis = {"Vector2::Transform", "Vector2::TransformNormal", "Matrix::CreateRotationZ",
             "Matrix::CreateTranslation", "Vector2::Distance"},
    .tags = {"2d", "rotation", "pivot"},
})
