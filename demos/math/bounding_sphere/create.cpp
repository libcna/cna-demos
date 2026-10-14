// SPDX-License-Identifier: MS-PL
// Bounding spheres from points, from a box, and merged.
#include <algorithm>
#include <any>
#include <cmath>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingSphereCreate(CnaDemos::DemoContext& ctx)
    {
        const std::vector<Vector3> points = {{-2.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
        const BoundingSphere fromPoints = BoundingSphere::CreateFromPoints(points);
        ctx.Print("CreateFromPoints       = {}", fromPoints.ToString());
        for (const Vector3& p : points)
        {
            ctx.Check(fromPoints.Contains(p) != ContainmentType::Disjoint, "every input point is inside the sphere");
        }

        // A box's bounding sphere: centre of the box, radius = half the diagonal.
        const BoundingBox box(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f));
        const BoundingSphere fromBox = BoundingSphere::CreateFromBoundingBox(box);
        ctx.Print("CreateFromBoundingBox  = {}", fromBox.ToString());
        ctx.Check(fromBox.Center == Vector3(1.0f, 1.0f, 1.0f), "the centre is the box centre");
        ctx.CheckNear(fromBox.Radius, std::sqrt(3.0f), 1e-5, "the radius is half the box diagonal");
        const BoundingSphere fromBoxCopy(fromBox.Center, fromBox.Radius);
        ctx.Check(fromBox.Equals(fromBoxCopy) && fromBox.Equals(std::any(fromBoxCopy)) &&
                      fromBox == fromBoxCopy && !(fromBox != fromBoxCopy) &&
                      fromBox.GetHashCode() == fromBoxCopy.GetHashCode() &&
                      !fromBox.Equals(std::any(17)) && !fromBox.Equals(std::any{}),
                  "equal spheres agree through typed and boxed equality, operators and hashing");
        const BoundingSphere expanded(fromBox.Center, fromBox.Radius + 1.0f);
        ctx.Check(expanded != fromBox && !expanded.Equals(fromBox),
                  "changing the radius makes two bounding spheres unequal");

        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());
        const BoundingSphere outer(Vector3::Zero, 5.0f);
        ctx.Check(outer.Contains(Vector3::Zero) == ContainmentType::Contains &&
                      outer.Contains(Vector3(5.0f, 0.0f, 0.0f)) == ContainmentType::Intersects &&
                      outer.Contains(Vector3(6.0f, 0.0f, 0.0f)) == ContainmentType::Disjoint,
                  "point containment distinguishes the interior, boundary and exterior");
        ctx.Check(outer.Contains(BoundingSphere(Vector3::Zero, 1.0f)) == ContainmentType::Contains &&
                      outer.Contains(BoundingSphere(Vector3(4.5f, 0.0f, 0.0f), 1.0f)) ==
                          ContainmentType::Intersects &&
                      outer.Contains(BoundingSphere(Vector3(7.0f, 0.0f, 0.0f), 1.0f)) ==
                          ContainmentType::Disjoint &&
                      outer.Contains(BoundingBox(-Vector3::One, Vector3::One)) == ContainmentType::Contains &&
                      outer.Contains(identityFrustum) == ContainmentType::Contains,
                  "sphere, box and frustum containment preserve the expected classifications");

        const BoundingSphere fromFrustum = BoundingSphere::CreateFromFrustum(identityFrustum);
        float farthestCorner = 0.0f;
        for (const Vector3& corner : identityFrustum.GetCorners())
        {
            farthestCorner = std::max(
                farthestCorner, Vector3::Distance(fromFrustum.Center, corner));
        }
        ctx.Check(fromFrustum.Radius > 0.0f && farthestCorner <= fromFrustum.Radius + 1e-5f,
                  "CreateFromFrustum encloses all eight source corners without assuming a minimum sphere");

        // Merging two spheres encloses both.
        const BoundingSphere merged = BoundingSphere::CreateMerged(BoundingSphere(Vector3(-5.0f, 0.0f, 0.0f), 1.0f),
                                                                   BoundingSphere(Vector3(5.0f, 0.0f, 0.0f), 1.0f));
        ctx.Print("CreateMerged           = {}", merged.ToString());
        ctx.CheckNear(merged.Radius, 6.0, 1e-5, "two unit spheres 10 apart merge into radius 6");
        ctx.CheckNear(merged.Center.Length(), 0.0, 1e-5, "centred between them");
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereCreate, {
    .id = "math.bounding_sphere.create",
    .title = "Create bounding spheres",
    .description = "Builds BoundingSphere values with CreateFromPoints, CreateFromBoundingBox and CreateMerged.",
    .apis = {"BoundingSphere::CreateFromPoints", "BoundingSphere::CreateFromBoundingBox",
             "BoundingSphere::CreateMerged", "BoundingSphere::Center", "BoundingSphere::Radius",
             "BoundingSphere::CreateFromFrustum", "BoundingSphere::Contains",
             "BoundingSphere::Equals", "BoundingSphere::GetHashCode", "BoundingSphere::op_Equality",
             "BoundingSphere::op_Inequality"},
    .tags = {"bounds", "sphere"},
})
