// SPDX-License-Identifier: MS-PL
// View-frustum culling with BoundingFrustum.
#include <any>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingFrustumCulling(CnaDemos::DemoContext& ctx)
    {
        // The camera's frustum is built from view * projection.
        const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 10.0f), Vector3::Zero, Vector3::Up);
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 4.0f / 3.0f, 1.0f, 100.0f);
        const BoundingFrustum frustum(view * projection);

        const BoundingSphere ahead(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
        const BoundingSphere behind(Vector3(0.0f, 0.0f, 20.0f), 1.0f);
        const BoundingSphere onEdge(Vector3(0.0f, 0.0f, 9.0f), 0.5f);    // centred on the near plane (z = 10 - 1)

        // Draw only what the camera can see.
        ctx.Check(frustum.Contains(ahead) == ContainmentType::Contains, "an object in front is visible");
        ctx.Check(frustum.Contains(behind) == ContainmentType::Disjoint, "an object behind the camera is culled");
        ctx.Check(frustum.Contains(onEdge) == ContainmentType::Intersects, "an object crossing the near plane is partly visible");
        ctx.Check(frustum.Intersects(ahead) && !frustum.Intersects(behind), "Intersects gives the yes/no version");

        // The frustum's planes and corners.
        ctx.Print("near plane = {}", frustum.getNearProperty().ToString());
        ctx.Print("far plane  = {}", frustum.getFarProperty().ToString());
        const std::vector<Vector3> corners = frustum.GetCorners();
        ctx.Print("{} corners; first near corner {}", corners.size(), corners.front().ToString());
        ctx.Check(corners.size() == static_cast<std::size_t>(BoundingFrustum::CornerCount), "a frustum has 8 corners");
        ctx.Check(frustum.getMatrixProperty() == view * projection, "Matrix returns the matrix it was built from");

        // The identity matrix makes all six clipping planes exact and easy to inspect.
        const BoundingFrustum identity(Matrix::getIdentityProperty());
        ctx.Check(identity.getLeftProperty() == Plane(-1.0f, 0.0f, 0.0f, -1.0f) &&
                      identity.getRightProperty() == Plane(1.0f, 0.0f, 0.0f, -1.0f),
                  "identity left and right planes bound X from -1 through 1");
        ctx.Check(identity.getTopProperty() == Plane(0.0f, 1.0f, 0.0f, -1.0f) &&
                      identity.getBottomProperty() == Plane(0.0f, -1.0f, 0.0f, -1.0f),
                  "identity top and bottom planes bound Y from -1 through 1");

        const BoundingFrustum sameIdentity(Matrix::getIdentityProperty());
        const BoundingFrustum changed(Matrix::CreateTranslation(1.0f, 0.0f, 0.0f));
        ctx.Check(identity.Equals(sameIdentity) && identity.Equals(std::any{sameIdentity}) &&
                      !identity.Equals(std::any{42}) && !identity.Equals(std::any{}),
                  "typed and boxed equality accept only the same frustum matrix");
        ctx.Check(identity == sameIdentity && identity != changed &&
                      identity.GetHashCode() == sameIdentity.GetHashCode(),
                  "frustum operators and hashes follow matrix identity");
        ctx.Check(identity.ToString() ==
                      "{Near:{Normal:{X:-0 Y:-0 Z:-1} D:-0} Far:{Normal:{X:0 Y:0 Z:1} D:-1} "
                      "Left:{Normal:{X:-1 Y:-0 Z:-0} D:-1} Right:{Normal:{X:1 Y:0 Z:0} D:-1} "
                      "Top:{Normal:{X:0 Y:1 Z:0} D:-1} Bottom:{Normal:{X:-0 Y:-1 Z:-0} D:-1}}",
                  "ToString lists all six clipping planes in canonical order");
    }
}

CNA_DEMOS_CONSOLE(BoundingFrustumCulling, {
    .id = "math.bounding_frustum.culling",
    .title = "View-frustum culling",
    .description = "Builds a BoundingFrustum from a camera's view and projection matrices and classifies objects "
                   "as visible, culled or partly visible; prints its planes and corners.",
    .apis = {"BoundingFrustum::BoundingFrustum", "BoundingFrustum::Contains", "BoundingFrustum::Intersects",
             "BoundingFrustum::Near", "BoundingFrustum::Far", "BoundingFrustum::Left",
             "BoundingFrustum::Right", "BoundingFrustum::Top", "BoundingFrustum::Bottom",
             "BoundingFrustum::GetCorners", "BoundingFrustum::Matrix", "BoundingFrustum::Equals",
             "BoundingFrustum::GetHashCode", "BoundingFrustum::ToString", "BoundingFrustum::op_Equality",
             "BoundingFrustum::op_Inequality"},
    .tags = {"culling", "camera", "frustum"},
})
