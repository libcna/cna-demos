// SPDX-License-Identifier: MS-PL
// Perspective projection matrices: field of view, width/height, off-centre.
#include <cmath>
#include <limits>
#include <stdexcept>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    template<typename F>
    bool ThrowsInvalidArgument(F&& action)
    {
        try { action(); }
        catch (const std::invalid_argument&) { return true; }
        return false;
    }

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

        // XNA validates angular and depth ordering, while leaving width/aspect degeneracy to
        // ordinary IEEE-754 division. Keep both halves visible: they are easy to over-normalize.
        ctx.Check(ThrowsInvalidArgument([] { (void) Matrix::CreatePerspectiveFieldOfView(0.0f, 1.0f, 1.0f, 10.0f); }),
                  "a zero field of view is refused");
        ctx.Check(ThrowsInvalidArgument([] { (void) Matrix::CreatePerspectiveFieldOfView(MathHelper::Pi, 1.0f, 1.0f, 10.0f); }),
                  "a field of view equal to Pi is refused");
        ctx.Check(ThrowsInvalidArgument([] { (void) Matrix::CreatePerspectiveFieldOfView(1.0f, 1.0f, 0.0f, 10.0f); }) &&
                      ThrowsInvalidArgument([] { (void) Matrix::CreatePerspectiveFieldOfView(1.0f, 1.0f, 1.0f, 0.0f); }) &&
                      ThrowsInvalidArgument([] { (void) Matrix::CreatePerspectiveFieldOfView(1.0f, 1.0f, 10.0f, 10.0f); }),
                  "field-of-view projection refuses nonpositive or unordered depth planes");
        ctx.Check(ThrowsInvalidArgument([] { (void) Matrix::CreatePerspective(2.0f, 2.0f, -1.0f, 10.0f); }) &&
                      ThrowsInvalidArgument([] { (void) Matrix::CreatePerspectiveOffCenter(-1.0f, 1.0f, -1.0f, 1.0f, 10.0f, 1.0f); }),
                  "size and off-centre projections enforce the same depth-plane contract");

        const Matrix zeroAspect = Matrix::CreatePerspectiveFieldOfView(1.0f, 0.0f, 1.0f, 10.0f);
        const Matrix negativeAspect = Matrix::CreatePerspectiveFieldOfView(1.0f, -1.0f, 1.0f, 10.0f);
        ctx.Check(std::isinf(zeroAspect.M11) && zeroAspect.M11 > 0.0f && negativeAspect.M11 < 0.0f,
                  "zero and negative aspect ratios remain raw XNA floating-point inputs");

        const Matrix zeroWidth = Matrix::CreatePerspective(0.0f, 1.0f, 1.0f, 10.0f);
        const Matrix zeroHeight = Matrix::CreatePerspective(1.0f, 0.0f, 1.0f, 10.0f);
        ctx.Check(std::isinf(zeroWidth.M11) && std::isinf(zeroHeight.M22),
                  "zero perspective dimensions expose positive infinity");

        const Matrix zeroOffWidth = Matrix::CreatePerspectiveOffCenter(1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 10.0f);
        const Matrix zeroOffHeight = Matrix::CreatePerspectiveOffCenter(-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 10.0f);
        ctx.Check(std::isinf(zeroOffWidth.M11) && std::isinf(zeroOffHeight.M22),
                  "collapsed off-centre spans expose positive infinity");

        const Matrix nanFieldOfView = Matrix::CreatePerspectiveFieldOfView(
            std::numeric_limits<float>::quiet_NaN(), 1.0f, 1.0f, 10.0f);
        ctx.Check(std::isnan(nanFieldOfView.M11) && std::isnan(nanFieldOfView.M22),
                  "a NaN field of view propagates through the XNA formula");
    }
}

CNA_DEMOS_CONSOLE(MatrixPerspective, {
    .id = "math.matrix.perspective",
    .title = "Perspective projection",
    .description = "Builds equivalent perspective frustums, verifies depth mapping and demonstrates XNA's "
                   "validated depth/angle boundaries versus IEEE-754 dimension and aspect behavior.",
    .apis = {"Matrix::CreatePerspectiveFieldOfView", "Matrix::CreatePerspective", "Matrix::CreatePerspectiveOffCenter"},
    .tags = {"matrix", "projection", "camera", "frustum"},
    .restrictions = "CNA-DEMOS-ISSUE-117 records acceptance of MathHelper::Pi as a field of view.",
    .knownIssue = "CNA-DEMOS-ISSUE-117",
})
