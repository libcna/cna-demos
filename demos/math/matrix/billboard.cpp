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
    }
}

CNA_DEMOS_CONSOLE(MatrixBillboard, {
    .id = "math.matrix.billboard",
    .title = "Billboard matrices",
    .description = "Turns an object towards the camera with Matrix::CreateBillboard, and about the up axis only "
                   "with Matrix::CreateConstrainedBillboard.",
    .apis = {"Matrix::CreateBillboard", "Matrix::CreateConstrainedBillboard"},
    .tags = {"matrix", "billboard", "particles"},
})
