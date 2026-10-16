// SPDX-License-Identifier: MS-PL
// BoundingBox::Contains and the three ContainmentType answers.
#include <string_view>

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
    std::string_view Name(ContainmentType type)
    {
        switch (type)
        {
            case ContainmentType::Disjoint: return "Disjoint";
            case ContainmentType::Contains: return "Contains";
            case ContainmentType::Intersects: return "Intersects";
        }
        return "?";
    }

    void BoundingBoxContains(CnaDemos::DemoContext& ctx)
    {
        const BoundingBox room(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 3.0f, 10.0f));

        const auto point = room.Contains(Vector3(5.0f, 1.0f, 5.0f));
        const auto inner = room.Contains(BoundingBox(Vector3(1.0f, 0.0f, 1.0f), Vector3(2.0f, 1.0f, 2.0f)));
        const auto crossing = room.Contains(BoundingSphere(Vector3(10.0f, 1.0f, 5.0f), 1.0f));
        const auto outside = room.Contains(BoundingSphere(Vector3(20.0f, 1.0f, 5.0f), 1.0f));
        ctx.Print("point in the middle     : {}", Name(point));
        ctx.Print("small box in a corner   : {}", Name(inner));
        ctx.Print("sphere through the wall : {}", Name(crossing));
        ctx.Print("sphere outside          : {}", Name(outside));

        ctx.Check(point == ContainmentType::Contains, "a point inside is Contains");
        ctx.Check(inner == ContainmentType::Contains, "a box fully inside is Contains");
        ctx.Check(crossing == ContainmentType::Intersects, "a sphere crossing the boundary is Intersects");
        ctx.Check(outside == ContainmentType::Disjoint, "a sphere outside is Disjoint");

        const BoundingBox unit(Vector3::Zero, Vector3(2.0f));
        const Vector3 boundary(2.0f, 1.0f, 1.0f);
        ContainmentType pointOutput = ContainmentType::Disjoint;
        unit.Contains(boundary, pointOutput);
        ctx.Check(unit.Contains(boundary) == ContainmentType::Contains &&
                      pointOutput == ContainmentType::Contains,
                  "a maximum-face point is contained through both point forms");

        const BoundingBox contained(Vector3(0.5f), Vector3(1.5f));
        const BoundingBox overlaps(Vector3(1.0f), Vector3(3.0f));
        const BoundingBox disjoint(Vector3(3.0f), Vector3(4.0f));
        ContainmentType boxContained = ContainmentType::Disjoint;
        ContainmentType boxOverlaps = ContainmentType::Disjoint;
        ContainmentType boxDisjoint = ContainmentType::Contains;
        unit.Contains(contained, boxContained);
        unit.Contains(overlaps, boxOverlaps);
        unit.Contains(disjoint, boxDisjoint);
        ctx.Check(boxContained == ContainmentType::Contains && boxOverlaps == ContainmentType::Intersects &&
                      boxDisjoint == ContainmentType::Disjoint,
                  "box output containment distinguishes inside, crossing and disjoint bounds");

        const BoundingSphere tangentInside(Vector3::One, 1.0f);
        const BoundingSphere tangentOutside(Vector3(3.0f, 1.0f, 1.0f), 1.0f);
        const BoundingSphere sphereDisjoint(Vector3(3.01f, 1.0f, 1.0f), 1.0f);
        ContainmentType sphereInsideOutput = ContainmentType::Disjoint;
        ContainmentType sphereTangentOutput = ContainmentType::Disjoint;
        ContainmentType sphereDisjointOutput = ContainmentType::Contains;
        unit.Contains(tangentInside, sphereInsideOutput);
        unit.Contains(tangentOutside, sphereTangentOutput);
        unit.Contains(sphereDisjoint, sphereDisjointOutput);
        ctx.Check(sphereInsideOutput == ContainmentType::Contains &&
                      sphereTangentOutput == ContainmentType::Intersects &&
                      sphereDisjointOutput == ContainmentType::Disjoint,
                  "sphere output containment keeps inside and outside tangency distinct");

        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());
        const BoundingBox huge(Vector3(-2.0f), Vector3(2.0f));
        const BoundingBox half(Vector3(-2.0f), Vector3(0.0f, 2.0f, 2.0f));
        const BoundingBox far(Vector3(10.0f), Vector3(11.0f));
        const ContainmentType hugeFrustum = huge.Contains(identityFrustum);
        const ContainmentType halfFrustum = half.Contains(identityFrustum);
        const ContainmentType farFrustum = far.Contains(identityFrustum);
        ctx.Print("identity frustum in huge/half/far boxes: {} / {} / {}",
                  Name(hugeFrustum), Name(halfFrustum), Name(farFrustum));
        ctx.Check(hugeFrustum == ContainmentType::Contains &&
                      halfFrustum == ContainmentType::Intersects &&
                      farFrustum == ContainmentType::Disjoint,
                  "frustum containment distinguishes enclosing, clipped and distant boxes");
    }
}

CNA_DEMOS_CONSOLE(BoundingBoxContains, {
    .id = "math.bounding_box.contains",
    .title = "Containment tests",
    .description = "Classifies points, boxes, spheres and a frustum through BoundingBox::Contains value/output "
                   "forms, including boundary tangency and all three ContainmentType results.",
    .apis = {"BoundingBox::Contains", "ContainmentType::Contains", "ContainmentType::Intersects",
             "ContainmentType::Disjoint"},
    .tags = {"bounds", "containment", "collision"},
    .restrictions = "CNA-DEMOS-ISSUE-127 records distant-frustum misclassification as Intersects.",
    .knownIssue = "CNA-DEMOS-ISSUE-127",
})
