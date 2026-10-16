// SPDX-License-Identifier: MS-PL
// Matrices that turn a quad towards the camera: CreateBillboard and CreateConstrainedBillboard.
#include <cmath>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixBillboard(CnaDemos::DemoContext& ctx)
    {
        const Vector3 object(0.0f, 0.0f, 0.0f);
        const Vector3 camera(4.0f, 3.0f, 0.0f);
        const Vector3 toCamera = Vector3::Normalize(camera - object);

        // A free billboard (particle, sprite in 3D) turns fully towards the camera.
        const Matrix billboard = Matrix::CreateBillboard(object, camera, Vector3::Up, std::nullopt);
        ctx.Print("billboard Backward = {}", billboard.getBackwardProperty().ToString());
        ctx.Check(billboard.getTranslationProperty() == object, "the billboard stays at the object's position");
        ctx.CheckNear(std::abs(Vector3::Dot(billboard.getBackwardProperty(), toCamera)), 1.0, 1e-5,
                      "its Z axis lies on the line to the camera");

        // A constrained billboard (tree, flame) only turns about one axis.
        const Matrix tree = Matrix::CreateConstrainedBillboard(object, camera, Vector3::Up, std::nullopt, std::nullopt);
        ctx.Print("constrained Up     = {}", tree.getUpProperty().ToString());
        ctx.CheckNear(Vector3::Distance(tree.getUpProperty(), Vector3::Up), 0.0, 1e-5, "it stays upright");
        ctx.CheckNear(tree.getBackwardProperty().Y, 0.0, 1e-5, "it only turns in the horizontal plane");

        // When camera and object coincide, XNA cannot derive a viewing direction from positions.
        // It uses Forward by default or the negated optional camera-forward direction.
        const Vector3 coincident(2.0f, 3.0f, 4.0f);
        const Matrix defaultFacing = Matrix::CreateBillboard(
            coincident, coincident, Vector3::Up, std::nullopt);
        ctx.Check(defaultFacing.getRightProperty() == Vector3::Left &&
                      defaultFacing.getUpProperty() == Vector3::Up &&
                      defaultFacing.getBackwardProperty() == Vector3::Forward &&
                      defaultFacing.getTranslationProperty() == coincident,
                  "a coincident camera uses XNA's default Forward fallback");

        const Matrix cameraFacing = Matrix::CreateBillboard(
            coincident, coincident, Vector3::Up, Vector3::Right);
        ctx.Check(cameraFacing.getRightProperty() == Vector3::Backward &&
                      cameraFacing.getUpProperty() == Vector3::Up &&
                      cameraFacing.getBackwardProperty() == Vector3::Left,
                  "an explicit camera-forward vector controls coincident fallback orientation");

        Matrix byReference;
        Matrix::CreateBillboard(coincident, coincident, Vector3::Up, std::nullopt, byReference);
        ctx.Check(byReference == defaultFacing,
                  "the by-reference billboard overload preserves the coincident fallback");

        // A constrained billboard looking exactly along its rotation axis instead falls back to
        // objectForward. A missing or still-parallel object direction selects Forward.
        const Matrix parallelDefault = Matrix::CreateConstrainedBillboard(
            Vector3::Zero, Vector3(0.0f, 5.0f, 0.0f), Vector3::Up, std::nullopt, std::nullopt);
        const Matrix parallelObjectUp = Matrix::CreateConstrainedBillboard(
            Vector3::Zero, Vector3(0.0f, 5.0f, 0.0f), Vector3::Up, std::nullopt, Vector3::Up);
        ctx.Check(parallelDefault.getRightProperty() == Vector3::Left &&
                      parallelDefault.getUpProperty() == Vector3::Up &&
                      parallelDefault.getBackwardProperty() == Vector3::Forward &&
                      parallelObjectUp == parallelDefault,
                  "parallel constrained views use Forward when objectForward is absent or parallel");

        const Matrix parallelObjectRight = Matrix::CreateConstrainedBillboard(
            Vector3::Zero, Vector3(0.0f, 5.0f, 0.0f), Vector3::Up, std::nullopt, Vector3::Right);
        ctx.Check(parallelObjectRight.getRightProperty() == Vector3::Forward &&
                      parallelObjectRight.getUpProperty() == Vector3::Up &&
                      parallelObjectRight.getBackwardProperty() == Vector3::Right,
                  "a usable objectForward vector controls the parallel constrained fallback");
    }
}

CNA_DEMOS_CONSOLE(MatrixBillboard, {
    .id = "math.matrix.billboard",
    .title = "Billboard matrices",
    .description = "Turns an object towards the camera with Matrix::CreateBillboard, constrains rotation to one "
                   "axis and demonstrates XNA's coincident and parallel-direction fallbacks.",
    .apis = {"Matrix::CreateBillboard", "Matrix::CreateConstrainedBillboard"},
    .tags = {"matrix", "billboard", "particles"},
})
