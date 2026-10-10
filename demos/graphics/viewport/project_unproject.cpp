// SPDX-License-Identifier: MS-PL
// Viewport::Project and Viewport::Unproject: world to screen and back, and a mouse-picking ray.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using Microsoft::Xna::Framework::Graphics::Viewport;

namespace
{
    void ProjectUnproject(CnaDemos::DemoContext& ctx)
    {
        const Viewport viewport(0, 0, 800, 480);
        const Matrix world = Matrix::getIdentityProperty();
        const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 10.0f), Vector3::Zero, Vector3::Up);
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                                                                       viewport.getAspectRatioProperty(), 1.0f, 100.0f);

        // Project: world position -> pixel position (X, Y) plus depth (Z, 0 = near plane, 1 = far).
        const Vector3 centre = viewport.Project(Vector3::Zero, projection, view, world);
        ctx.Print("Project(origin)     = {}", centre.ToString());
        ctx.CheckNear(centre.X, 400.0, 1e-3, "the point the camera looks at lands in the middle (X)");
        ctx.CheckNear(centre.Y, 240.0, 1e-3, "the point the camera looks at lands in the middle (Y)");
        ctx.Check(centre.Z > 0.0f && centre.Z < 1.0f, "its depth lies between the near and far planes");

        const Vector3 above = viewport.Project(Vector3(0.0f, 1.0f, 0.0f), projection, view, world);
        ctx.Print("Project(0, 1, 0)    = {}", above.ToString());
        ctx.Check(above.Y < centre.Y, "world +Y points up the screen, where pixel Y gets smaller");

        // Unproject: a pixel at depth 0 and at depth 1 gives two world points; the line through them
        // is everything under that pixel. This is how a mouse click becomes a picking ray.
        const float mouseX = 600.0f;
        const float mouseY = 300.0f;
        const Vector3 nearPoint = viewport.Unproject(Vector3(mouseX, mouseY, 0.0f), projection, view, world);
        const Vector3 farPoint = viewport.Unproject(Vector3(mouseX, mouseY, 1.0f), projection, view, world);
        const Ray ray(nearPoint, Vector3::Normalize(farPoint - nearPoint));
        ctx.Print("Unproject(600, 300, 0) = {}", nearPoint.ToString());
        ctx.Print("Unproject(600, 300, 1) = {}", farPoint.ToString());
        ctx.CheckNear(nearPoint.Z, 9.0, 1e-3, "depth 0 unprojects onto the near plane (1 unit in front of the camera)");

        // Intersect the ray with the ground plane z = 0 and project the hit back: the round trip
        // returns the pixel we started from.
        const Plane ground(Vector3::Backward, 0.0f);
        const auto distance = ray.Intersects(ground);
        ctx.Check(distance.has_value(), "the picking ray hits the plane z = 0");
        if (distance.has_value())
        {
            const Vector3 hit = ray.Position + ray.Direction * *distance;
            const Vector3 back = viewport.Project(hit, projection, view, world);
            ctx.Print("ray hits z = 0 at {}, which projects to {}", hit.ToString(), back.ToString());
            ctx.CheckNear(back.X, mouseX, 1e-2, "projecting the hit point returns the mouse X");
            ctx.CheckNear(back.Y, mouseY, 1e-2, "projecting the hit point returns the mouse Y");
        }
    }
}

CNA_DEMOS_CONSOLE(ProjectUnproject, {
    .id = "graphics.viewport.project_unproject",
    .title = "Project and unproject",
    .description = "Projects world points to pixels with Viewport::Project and turns a pixel back into a picking "
                   "ray with Viewport::Unproject at depths 0 and 1.",
    .apis = {"Viewport::Project", "Viewport::Unproject", "Ray::Intersects"},
    .tags = {"viewport", "picking", "mouse", "3d", "screen-space"},
})
