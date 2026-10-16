// SPDX-License-Identifier: MS-PL
// Orthographic projections, including the pixel-space projection 2D rendering uses.
#include <cmath>

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

        // Unlike the perspective constructors, XNA performs no argument rejection here. Degenerate
        // spans and depth ranges expose the formula's IEEE-754 results directly.
        const Matrix zeroWidth = Matrix::CreateOrthographic(0.0f, 2.0f, 1.0f, 10.0f);
        const Matrix zeroHeight = Matrix::CreateOrthographic(2.0f, 0.0f, 1.0f, 10.0f);
        Matrix zeroWidthByReference;
        Matrix::CreateOrthographic(0.0f, 2.0f, 1.0f, 10.0f, zeroWidthByReference);
        ctx.Check(std::isinf(zeroWidth.M11) && zeroWidth.M11 > 0.0f &&
                      std::isinf(zeroHeight.M22) && zeroHeight.M22 > 0.0f &&
                      zeroWidthByReference == zeroWidth,
                  "zero dimensions produce positive infinity in both overloads");

        const Matrix equalZeroDepth = Matrix::CreateOrthographic(2.0f, 2.0f, 0.0f, 0.0f);
        const Matrix equalNonzeroDepth = Matrix::CreateOrthographic(2.0f, 2.0f, 5.0f, 5.0f);
        ctx.Check(std::isinf(equalZeroDepth.M33) && std::isnan(equalZeroDepth.M43) &&
                      std::isinf(equalNonzeroDepth.M33) && std::isinf(equalNonzeroDepth.M43),
                  "equal depth planes distinguish zero-over-zero NaN from nonzero infinity");

        const Matrix reversed = Matrix::CreateOrthographic(-2.0f, -4.0f, 10.0f, 1.0f);
        ctx.Check(reversed.M11 == -1.0f && reversed.M22 == -0.5f && reversed.M33 > 0.0f &&
                      reversed.M43 > 1.0f,
                  "negative dimensions and reversed depth retain their formula signs");

        const Matrix collapsedZero = Matrix::CreateOrthographicOffCenter(
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        ctx.Check(std::isinf(collapsedZero.M11) && std::isinf(collapsedZero.M22) &&
                      std::isinf(collapsedZero.M33) && std::isnan(collapsedZero.M41) &&
                      std::isnan(collapsedZero.M42) && std::isnan(collapsedZero.M43),
                  "zero collapsed off-centre bounds produce infinity and zero-over-zero NaN");

        const Matrix collapsedNonzero = Matrix::CreateOrthographicOffCenter(
            5.0f, 5.0f, 7.0f, 7.0f, 3.0f, 3.0f);
        ctx.Check(std::isinf(collapsedNonzero.M11) && std::isinf(collapsedNonzero.M22) &&
                      std::isinf(collapsedNonzero.M33) && std::isinf(collapsedNonzero.M41) &&
                      std::isinf(collapsedNonzero.M42) && std::isinf(collapsedNonzero.M43),
                  "nonzero collapsed off-centre bounds produce positive infinity");
    }
}

CNA_DEMOS_CONSOLE(MatrixOrthographic, {
    .id = "math.matrix.orthographic",
    .title = "Orthographic projection",
    .description = "Maps pixel coordinates to clip space, builds a centred volume and demonstrates XNA's raw "
                   "IEEE-754 behavior for collapsed orthographic spans and depth ranges.",
    .apis = {"Matrix::CreateOrthographicOffCenter", "Matrix::CreateOrthographic"},
    .tags = {"matrix", "projection", "2d", "pixel"},
})
