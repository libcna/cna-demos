// SPDX-License-Identifier: MS-PL
// A camera view matrix with Matrix::CreateLookAt.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixLookAt(CnaDemos::DemoContext& ctx)
    {
        const Vector3 cameraPosition(0.0f, 5.0f, 10.0f);
        const Vector3 target = Vector3::Zero;
        const Matrix view = Matrix::CreateLookAt(cameraPosition, target, Vector3::Up);

        // The view matrix moves the world so the camera sits at the origin looking down -Z.
        const Vector3 cameraInView = Vector3::Transform(cameraPosition, view);
        const Vector3 targetInView = Vector3::Transform(target, view);
        ctx.Print("camera in view space = {}", cameraInView.ToString());
        ctx.Print("target in view space = {}", targetInView.ToString());

        ctx.CheckNear(cameraInView.Length(), 0.0, 1e-5, "the camera ends up at the origin");
        ctx.CheckNear(targetInView.X, 0.0, 1e-5, "the target is centred horizontally");
        ctx.CheckNear(targetInView.Y, 0.0, 1e-5, "the target is centred vertically");
        ctx.CheckNear(targetInView.Z, -Vector3::Distance(cameraPosition, target), 1e-4,
                      "the target is straight ahead (-Z) at its distance");
    }
}

CNA_DEMOS_CONSOLE(MatrixLookAt, {
    .id = "math.matrix.look_at",
    .title = "Look-at view matrix",
    .description = "Builds a view matrix with Matrix::CreateLookAt and shows that it moves the camera to the "
                   "origin and puts the target straight ahead on -Z.",
    .apis = {"Matrix::CreateLookAt"},
    .tags = {"matrix", "camera", "view"},
})
