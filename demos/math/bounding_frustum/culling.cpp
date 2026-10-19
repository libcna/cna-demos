// SPDX-License-Identifier: MS-PL
// BoundingFrustum construction, camera culling, planes, matrix mutation and value identity.
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingFrustumCulling(CnaDemos::DemoContext& ctx)
    {
        const Matrix view =
            Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 10.0f), Vector3::Zero, Vector3::Up);
        const Matrix projection =
            Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 4.0f / 3.0f, 1.0f, 100.0f);
        const BoundingFrustum camera(view * projection);

        const BoundingSphere ahead(Vector3::Zero, 1.0f);
        const BoundingSphere behind(Vector3(0.0f, 0.0f, 20.0f), 1.0f);
        const BoundingSphere onNearPlane(Vector3(0.0f, 0.0f, 9.0f), 0.5f);
        ctx.Check(camera.Contains(ahead) == ContainmentType::Contains &&
                      camera.Contains(behind) == ContainmentType::Disjoint &&
                      camera.Contains(onNearPlane) == ContainmentType::Intersects,
                  "camera containment distinguishes visible, culled and near-plane-crossing spheres");
        ctx.Check(camera.Intersects(ahead) && !camera.Intersects(behind) &&
                      camera.Intersects(onNearPlane),
                  "camera intersection is the matching boolean culling view");

        const BoundingFrustum identity(Matrix::getIdentityProperty());
        ctx.Check(identity.getMatrixProperty() == Matrix::getIdentityProperty(),
                  "Matrix retains the exact combined matrix supplied at construction");
        ctx.Check(identity.getNearProperty() == Plane(0.0f, 0.0f, -1.0f, 0.0f) &&
                      identity.getFarProperty() == Plane(0.0f, 0.0f, 1.0f, -1.0f),
                  "identity near and far planes bound Z from zero through one");
        ctx.Check(identity.getLeftProperty() == Plane(-1.0f, 0.0f, 0.0f, -1.0f) &&
                      identity.getRightProperty() == Plane(1.0f, 0.0f, 0.0f, -1.0f),
                  "identity left and right planes bound X from minus one through one");
        ctx.Check(identity.getTopProperty() == Plane(0.0f, 1.0f, 0.0f, -1.0f) &&
                      identity.getBottomProperty() == Plane(0.0f, -1.0f, 0.0f, -1.0f),
                  "identity top and bottom planes bound Y from minus one through one");

        BoundingFrustum mutableFrustum(Matrix::getIdentityProperty());
        const Matrix shiftedMatrix = Matrix::CreateTranslation(-10.0f, 0.0f, 0.0f);
        mutableFrustum.setMatrixProperty(shiftedMatrix);
        ctx.Check(mutableFrustum.getMatrixProperty() == shiftedMatrix &&
                      mutableFrustum.Contains(Vector3(0.0f, 0.0f, 0.5f)) == ContainmentType::Disjoint &&
                      mutableFrustum.Contains(Vector3(10.0f, 0.0f, 0.5f)) == ContainmentType::Contains,
                  "setting Matrix recomputes all planes and corners for the translated frustum");

        const BoundingFrustum equal(Matrix::getIdentityProperty());
        const BoundingFrustum changed(shiftedMatrix);
        ctx.Check(identity.Equals(equal) && identity.Equals(std::any{equal}) &&
                      !identity.Equals(std::any{42}) && !identity.Equals(std::any{}),
                  "typed and boxed equality accept only the same frustum matrix");
        ctx.Check(identity == equal && identity != changed &&
                      identity.GetHashCode() == equal.GetHashCode(),
                  "frustum operators and equal hashing follow matrix identity");

        ctx.Print("identity frustum = {}", identity.ToString());
        ctx.Check(identity.ToString() ==
                      "{Near:{Normal:{X:0 Y:0 Z:-1} D:0} Far:{Normal:{X:0 Y:0 Z:1} D:-1} "
                      "Left:{Normal:{X:-1 Y:0 Z:0} D:-1} Right:{Normal:{X:1 Y:0 Z:0} D:-1} "
                      "Top:{Normal:{X:0 Y:1 Z:0} D:-1} Bottom:{Normal:{X:0 Y:-1 Z:0} D:-1}}",
                  "ToString lists the six clipping planes in XNA's canonical signed-zero form");
    }
}

CNA_DEMOS_CONSOLE(BoundingFrustumCulling, {
    .id = "math.bounding_frustum.culling",
    .title = "View-frustum construction and culling",
    .description = "Builds camera and identity BoundingFrustums, observes all six planes and matrix mutation, performs "
                   "sphere culling, and checks matrix-based identity, hashing and exact XNA text.",
    .apis = {"BoundingFrustum::BoundingFrustum", "BoundingFrustum::Contains", "BoundingFrustum::Intersects",
             "BoundingFrustum::Near", "BoundingFrustum::Far", "BoundingFrustum::Left",
             "BoundingFrustum::Right", "BoundingFrustum::Top", "BoundingFrustum::Bottom",
             "BoundingFrustum::Matrix", "BoundingFrustum::Equals", "BoundingFrustum::GetHashCode",
             "BoundingFrustum::ToString", "BoundingFrustum::op_Equality", "BoundingFrustum::op_Inequality"},
    .tags = {"culling", "camera", "frustum", "value-semantics"},
})
