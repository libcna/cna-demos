// SPDX-License-Identifier: MS-PL
// Planar effects: mirror with Matrix::CreateReflection, flatten into a shadow with CreateShadow.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixReflectionAndShadow(CnaDemos::DemoContext& ctx)
    {
        const Plane ground(Vector3::Up, 0.0f);   // the plane y = 0

        // Mirror image below the ground (a water reflection).
        const Vector3 point(2.0f, 5.0f, -1.0f);
        const Vector3 mirrored = Vector3::Transform(point, Matrix::CreateReflection(ground));
        ctx.Print("reflection of {} in y = 0: {}", point.ToString(), mirrored.ToString());
        ctx.Check(mirrored == Vector3(2.0f, -5.0f, -1.0f), "the mirror flips the height");

        // Planar shadow: project points along the light direction onto the plane. The result is
        // homogeneous, so divide by W.
        const Vector3 lightDirection = Vector3::Normalize(Vector3(-1.0f, -1.0f, 0.0f));
        const Matrix shadow = Matrix::CreateShadow(lightDirection, ground);
        const Vector4 h = Vector4::Transform(Vector4(point, 1.0f), shadow);
        const Vector3 onGround(h.X / h.W, h.Y / h.W, h.Z / h.W);
        ctx.Print("shadow of {} with light {}: {}", point.ToString(), lightDirection.ToString(), onGround.ToString());
        ctx.CheckNear(onGround.Y, 0.0, 1e-5, "the shadow lies on the ground plane");
        ctx.CheckNear(std::abs(onGround.X - point.X), point.Y, 1e-4, "a 45-degree light offsets it by the height");

        // XNA normalizes either plane internally. Multiplying every coefficient by two still
        // describes y = 2 and must produce the exact same reflection or shadow matrix.
        const Plane elevated(Vector3::Up, -2.0f);
        const Plane elevatedScaled(Vector3(0.0f, 2.0f, 0.0f), -4.0f);
        const Matrix reflection = Matrix::CreateReflection(elevated);
        Matrix reflectionOutput;
        Matrix::CreateReflection(elevatedScaled, reflectionOutput);
        ctx.Check(Matrix::CreateReflection(elevatedScaled) == reflection && reflectionOutput == reflection &&
                      Vector3::Transform(point, reflection) == Vector3(2.0f, -1.0f, -1.0f),
                  "reflection normalizes a scaled offset plane in value and output forms");

        const Matrix elevatedShadow = Matrix::CreateShadow(lightDirection, elevated);
        Matrix shadowOutput;
        Matrix::CreateShadow(lightDirection, elevatedScaled, shadowOutput);
        ctx.Check(Matrix::CreateShadow(lightDirection, elevatedScaled) == elevatedShadow &&
                      shadowOutput == elevatedShadow,
                  "shadow normalizes a scaled offset plane in value and output forms");
    }
}

CNA_DEMOS_CONSOLE(MatrixReflectionAndShadow, {
    .id = "math.matrix.reflection_and_shadow",
    .title = "Reflection and planar shadow matrices",
    .description = "Mirrors and projects points with reflection and shadow matrices, including XNA's normalization "
                   "of scaled offset-plane coefficients and both output forms.",
    .apis = {"Matrix::CreateReflection", "Matrix::CreateShadow", "Plane::Plane"},
    .tags = {"matrix", "plane", "mirror", "shadow"},
    .restrictions = "CNA-DEMOS-ISSUE-140 records missing plane normalization in CreateShadow.",
    .knownIssue = "CNA-DEMOS-ISSUE-140",
})
