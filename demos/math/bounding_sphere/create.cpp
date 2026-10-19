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
#include "System/ArgumentException.hpp"

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

        ctx.Check(BoundingSphere().Center == Vector3::Zero && BoundingSphere().Radius == 0.0f &&
                      BoundingSphere(Vector3(1.0f, 2.0f, 3.0f), 0.0f).Radius == 0.0f &&
                      BoundingSphere(Vector3(-1.0f, 2.0f, -3.0f), 4.0f).ToString() ==
                          "{Center:{X:-1 Y:2 Z:-3} Radius:4}",
                  "default and zero-radius construction plus signed text match XNA");

        bool negativeRadiusRejected = false;
        try { (void)BoundingSphere(Vector3(1.0f, 2.0f, 3.0f), -1.0f); }
        catch (const System::ArgumentException&) { negativeRadiusRejected = true; }
        bool emptyPointsUseXnaException = false;
        try { (void)BoundingSphere::CreateFromPoints({}); }
        catch (const System::ArgumentException&) { emptyPointsUseXnaException = true; }
        catch (...) {}
        ctx.Check(negativeRadiusRejected && emptyPointsUseXnaException,
                  "XNA rejects a negative radius and an empty point set with ArgumentException");

        const Vector3 lonePoint(-2.0f, 3.0f, -4.0f);
        ctx.Check(BoundingSphere::CreateFromPoints({lonePoint}) == BoundingSphere(lonePoint, 0.0f),
                  "one point produces a zero-radius sphere at that point");

        BoundingSphere fromBoxOutput;
        BoundingSphere::CreateFromBoundingBox(box, fromBoxOutput);
        ctx.Check(fromBoxOutput == fromBox,
                  "the CreateFromBoundingBox output form agrees with the value form");

        BoundingSphere mergedOutput;
        const BoundingSphere left(Vector3(-5.0f, 0.0f, 0.0f), 1.0f);
        const BoundingSphere right(Vector3(5.0f, 0.0f, 0.0f), 1.0f);
        BoundingSphere::CreateMerged(left, right, mergedOutput);
        BoundingSphere aliasLeft = left;
        BoundingSphere otherLeft = right;
        BoundingSphere::CreateMerged(aliasLeft, otherLeft, aliasLeft);
        BoundingSphere otherRight = left;
        BoundingSphere aliasRight = right;
        BoundingSphere::CreateMerged(otherRight, aliasRight, aliasRight);
        ctx.Check(mergedOutput == merged && aliasLeft == merged && aliasRight == merged,
                  "CreateMerged output may safely alias either input");

        const BoundingSphere enclosing(Vector3::Zero, 10.0f);
        const BoundingSphere enclosed(Vector3::One, 1.0f);
        ctx.Check(BoundingSphere::CreateMerged(enclosing, enclosed) == enclosing &&
                      BoundingSphere::CreateMerged(enclosed, enclosing) == enclosing,
                  "merging returns an enclosing sphere in either argument order");
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereCreate, {
    .id = "math.bounding_sphere.create",
    .title = "Create bounding spheres",
    .description = "Builds BoundingSphere values from points, boxes, a frustum and merged spheres through value, "
                   "output and alias forms, including constructor and collection refusal boundaries.",
    .apis = {"BoundingSphere::CreateFromPoints", "BoundingSphere::CreateFromBoundingBox",
             "BoundingSphere::CreateMerged", "BoundingSphere::Center", "BoundingSphere::Radius",
             "BoundingSphere::CreateFromFrustum", "BoundingSphere::BoundingSphere", "BoundingSphere::ToString",
             "BoundingSphere::Equals", "BoundingSphere::GetHashCode", "BoundingSphere::op_Equality",
             "BoundingSphere::op_Inequality"},
    .tags = {"bounds", "sphere"},
})
