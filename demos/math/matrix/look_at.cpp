// SPDX-License-Identifier: MS-PL
// A camera view matrix with Matrix::CreateLookAt.
#include <cmath>

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

        Matrix byReference;
        Matrix::CreateLookAt(cameraPosition, target, Vector3::Up, byReference);
        ctx.Check(byReference == view, "the by-reference overload matches the value result");

        // XNA does not repair degenerate camera bases. Normalizing a zero direction or a zero
        // cross product exposes NaNs, while the unaffected affine/depth entries remain visible.
        const Vector3 coincident(2.0f, 3.0f, 4.0f);
        const Matrix samePosition = Matrix::CreateLookAt(coincident, coincident, Vector3::Up);
        ctx.Check(std::isnan(samePosition.M11) && std::isnan(samePosition.M12) &&
                      std::isnan(samePosition.M13) && std::isnan(samePosition.M21) &&
                      std::isnan(samePosition.M22) && std::isnan(samePosition.M23) &&
                      std::isnan(samePosition.M31) && std::isnan(samePosition.M32) &&
                      std::isnan(samePosition.M33) && std::isnan(samePosition.M41) &&
                      std::isnan(samePosition.M42) && std::isnan(samePosition.M43) &&
                      samePosition.M14 == 0.0f && samePosition.M24 == 0.0f &&
                      samePosition.M34 == 0.0f && samePosition.M44 == 1.0f,
                  "coincident camera and target publish a NaN basis and translation");

        const Matrix zeroUp = Matrix::CreateLookAt(
            Vector3(0.0f, 0.0f, 10.0f), Vector3::Zero, Vector3::Zero);
        ctx.Check(std::isnan(zeroUp.M11) && std::isnan(zeroUp.M21) && std::isnan(zeroUp.M31) &&
                      std::isnan(zeroUp.M12) && std::isnan(zeroUp.M22) && std::isnan(zeroUp.M32) &&
                      zeroUp.M13 == 0.0f && zeroUp.M23 == 0.0f && zeroUp.M33 == 1.0f &&
                      std::isnan(zeroUp.M41) && std::isnan(zeroUp.M42) && zeroUp.M43 == -10.0f,
                  "a zero up vector preserves only the normalized viewing column and depth translation");

        const Matrix parallelUp = Matrix::CreateLookAt(
            Vector3(0.0f, 5.0f, 0.0f), Vector3::Zero, Vector3::Up);
        const Matrix antiparallelUp = Matrix::CreateLookAt(
            Vector3(0.0f, -5.0f, 0.0f), Vector3::Zero, Vector3::Up);
        ctx.Check(parallelUp.M13 == 0.0f && parallelUp.M23 == 1.0f && parallelUp.M33 == 0.0f &&
                      antiparallelUp.M13 == 0.0f && antiparallelUp.M23 == -1.0f && antiparallelUp.M33 == 0.0f &&
                      parallelUp.M43 == -5.0f && antiparallelUp.M43 == -5.0f &&
                      std::isnan(parallelUp.M11) && std::isnan(antiparallelUp.M11),
                  "parallel and antiparallel up vectors retain viewing direction but leave a NaN lateral basis");
    }
}

CNA_DEMOS_CONSOLE(MatrixLookAt, {
    .id = "math.matrix.look_at",
    .title = "Look-at view matrix",
    .description = "Builds a view matrix, verifies value/output parity and demonstrates XNA's NaN basis behavior "
                   "for coincident positions and zero or parallel up vectors.",
    .apis = {"Matrix::CreateLookAt"},
    .tags = {"matrix", "camera", "view"},
})
