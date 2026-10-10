// SPDX-License-Identifier: MS-PL
// Orthographic projections, including the pixel-space projection 2D rendering uses.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixOrthographic(CnaDemos::DemoContext& ctx)
    {
        // Pixel coordinates of an 800x480 screen, Y down: left 0, right 800, bottom 480, top 0.
        const Matrix screen = Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f);

        const Vector3 topLeft = Vector3::Transform(Vector3(0.0f, 0.0f, 0.0f), screen);
        const Vector3 bottomRight = Vector3::Transform(Vector3(800.0f, 480.0f, 0.0f), screen);
        const Vector3 centre = Vector3::Transform(Vector3(400.0f, 240.0f, 0.0f), screen);
        ctx.Print("pixel (0, 0)     -> {}", topLeft.ToString());
        ctx.Print("pixel (800, 480) -> {}", bottomRight.ToString());
        ctx.Print("pixel (400, 240) -> {}", centre.ToString());
        // Floating-point: compare with a tolerance (ToString rounds to 7 significant digits).
        ctx.CheckNear(Vector3::Distance(topLeft, Vector3(-1.0f, 1.0f, 0.0f)), 0.0, 1e-6, "the top-left corner maps to (-1, 1)");
        ctx.CheckNear(Vector3::Distance(bottomRight, Vector3(1.0f, -1.0f, 0.0f)), 0.0, 1e-6,
                      "the bottom-right corner maps to (1, -1)");
        ctx.CheckNear(centre.Length(), 0.0, 1e-6, "the screen centre maps to the origin");

        // Centred form: width and height of the view volume, no perspective.
        const Matrix centred = Matrix::CreateOrthographic(20.0f, 10.0f, 0.0f, 100.0f);
        const Vector3 near = Vector3::Transform(Vector3(10.0f, 5.0f, 0.0f), centred);
        const Vector3 far = Vector3::Transform(Vector3(10.0f, 5.0f, -50.0f), centred);
        ctx.CheckNear(Vector3::Distance(Vector3(near.X, near.Y, 0.0f), Vector3(far.X, far.Y, 0.0f)), 0.0, 1e-6,
                      "distance does not change the projected position");
        ctx.CheckNear(near.X, 1.0, 1e-6, "half the width maps to the right edge");
    }
}

CNA_DEMOS_CONSOLE(MatrixOrthographic, {
    .id = "math.matrix.orthographic",
    .title = "Orthographic projection",
    .description = "Maps pixel coordinates to clip space with Matrix::CreateOrthographicOffCenter (the 2D "
                   "screen projection) and builds a centred volume with CreateOrthographic.",
    .apis = {"Matrix::CreateOrthographicOffCenter", "Matrix::CreateOrthographic"},
    .tags = {"matrix", "projection", "2d", "pixel"},
})
