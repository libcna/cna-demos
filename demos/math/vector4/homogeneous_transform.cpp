// SPDX-License-Identifier: MS-PL
// Projecting a 3D point with a perspective matrix: Vector4 and the divide by W.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector4Homogeneous(CnaDemos::DemoContext& ctx)
    {
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver2, 1.0f, 1.0f, 100.0f);

        // A point 10 units in front of the camera and 10 units to the right: on the edge of a
        // 90-degree field of view.
        const Vector3 point(10.0f, 0.0f, -10.0f);
        const Vector4 clip = Vector4::Transform(Vector4(point, 1.0f), projection);
        ctx.Print("clip-space position = {}", clip.ToString());

        // Perspective division turns clip space into normalized device coordinates.
        const Vector3 ndc(clip.X / clip.W, clip.Y / clip.W, clip.Z / clip.W);
        ctx.Print("after dividing by W = {}", ndc.ToString());

        ctx.CheckNear(clip.W, 10.0, 1e-4, "W is the distance in front of the camera");
        ctx.CheckNear(clip.Y, 0.0, 1e-6, "a point on the camera's horizontal axis keeps clip-space Y at zero");
        ctx.CheckNear(ndc.X, 1.0, 1e-5, "the point lands on the right edge (x = 1)");
        ctx.Check(ndc.Z > 0.0f && ndc.Z < 1.0f, "depth lies between the near and far planes (0..1)");
    }
}

CNA_DEMOS_CONSOLE(Vector4Homogeneous, {
    .id = "math.vector4.homogeneous_transform",
    .title = "Homogeneous coordinates",
    .description = "Projects a 3D point with Vector4::Transform and a perspective matrix, then divides by W to "
                   "get normalized device coordinates.",
    .apis = {"Vector4::Vector4", "Vector4::Transform", "Vector4::X", "Vector4::Y", "Vector4::Z", "Vector4::W",
             "Matrix::CreatePerspectiveFieldOfView"},
    .tags = {"projection", "clip-space", "perspective"},
})
