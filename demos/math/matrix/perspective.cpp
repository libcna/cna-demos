// SPDX-License-Identifier: MS-PL
// Perspective projection matrices: field of view, width/height, off-centre.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    Vector3 Project(Vector3 point, const Matrix& projection)
    {
        const Vector4 clip = Vector4::Transform(Vector4(point, 1.0f), projection);
        return Vector3(clip.X / clip.W, clip.Y / clip.W, clip.Z / clip.W);
    }

    void MatrixPerspective(CnaDemos::DemoContext& ctx)
    {
        const float nearPlane = 1.0f;
        const float farPlane = 100.0f;
        const float aspect = 16.0f / 9.0f;

        // The usual form: vertical field of view and aspect ratio.
        const Matrix byFieldOfView = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, nearPlane, farPlane);

        // Depth maps the near plane to 0 and the far plane to 1.
        ctx.CheckNear(Project(Vector3(0.0f, 0.0f, -nearPlane), byFieldOfView).Z, 0.0, 1e-5, "the near plane maps to depth 0");
        ctx.CheckNear(Project(Vector3(0.0f, 0.0f, -farPlane), byFieldOfView).Z, 1.0, 1e-5, "the far plane maps to depth 1");
        ctx.Print("depth of a point 10 units away = {:.4f} (non-linear)", Project(Vector3(0.0f, 0.0f, -10.0f), byFieldOfView).Z);

        // The same frustum given as the size of the view volume at the near plane.
        const float height = 2.0f * nearPlane * std::tan(MathHelper::PiOver4 / 2.0f);
        const Matrix bySize = Matrix::CreatePerspective(height * aspect, height, nearPlane, farPlane);
        ctx.CheckNear(CnaDemos::MaxDifference(byFieldOfView, bySize), 0.0, 1e-5, "CreatePerspective describes the same frustum");

        // Off-centre: an asymmetric frustum (used for split screens, stereo, tiled rendering).
        const Matrix offCentre = Matrix::CreatePerspectiveOffCenter(-height * aspect / 2.0f, height * aspect / 2.0f,
                                                                    -height / 2.0f, height / 2.0f, nearPlane, farPlane);
        ctx.CheckNear(CnaDemos::MaxDifference(byFieldOfView, offCentre), 0.0, 1e-5,
                      "a symmetric off-centre frustum equals the field-of-view one");
    }
}

CNA_DEMOS_CONSOLE(MatrixPerspective, {
    .id = "math.matrix.perspective",
    .title = "Perspective projection",
    .description = "Builds the same perspective frustum with CreatePerspectiveFieldOfView, CreatePerspective and "
                   "CreatePerspectiveOffCenter, and shows depth mapping the near plane to 0 and the far plane to 1.",
    .apis = {"Matrix::CreatePerspectiveFieldOfView", "Matrix::CreatePerspective", "Matrix::CreatePerspectiveOffCenter"},
    .tags = {"matrix", "projection", "camera", "frustum"},
})
