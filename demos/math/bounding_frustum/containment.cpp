// SPDX-License-Identifier: MS-PL
// BoundingFrustum containment for points, boxes, spheres and other frusta.
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
    Matrix InnerMatrix()
    {
        Matrix value = Matrix::getIdentityProperty();
        value.M11 = 2.0f;
        value.M22 = 2.0f;
        value.M33 = 2.0f;
        value.M43 = -0.5f;
        return value;
    }

    Matrix LargerMatrix()
    {
        Matrix value = Matrix::getIdentityProperty();
        value.M11 = 0.5f;
        value.M22 = 0.5f;
        value.M33 = 0.5f;
        value.M43 = 0.25f;
        return value;
    }

    void BoundingFrustumContainment(CnaDemos::DemoContext& ctx)
    {
        const BoundingFrustum identity(Matrix::getIdentityProperty());
        const Vector3 insidePoint(0.0f, 0.0f, 0.5f);
        const Vector3 outsidePoint(0.0f, 0.0f, 2.0f);
        ContainmentType insideOutput = ContainmentType::Disjoint;
        ContainmentType outsideOutput = ContainmentType::Contains;
        identity.Contains(insidePoint, insideOutput);
        identity.Contains(outsidePoint, outsideOutput);
        ctx.Check(identity.Contains(insidePoint) == ContainmentType::Contains &&
                      identity.Contains(outsidePoint) == ContainmentType::Disjoint &&
                      insideOutput == ContainmentType::Contains &&
                      outsideOutput == ContainmentType::Disjoint,
                  "point value and output forms distinguish the interior and exterior");

        const Vector3 boundaryPoint = Vector3::Zero;
        const Vector3 boundaryOutsidePoint(2.0f, 0.0f, 0.0f);
        ContainmentType boundaryOutput = ContainmentType::Disjoint;
        ContainmentType boundaryOutsideOutput = ContainmentType::Contains;
        identity.Contains(boundaryPoint, boundaryOutput);
        identity.Contains(boundaryOutsidePoint, boundaryOutsideOutput);
        ctx.Check(identity.Contains(boundaryPoint) == ContainmentType::Contains &&
                      boundaryOutput == ContainmentType::Contains &&
                      identity.Contains(boundaryOutsidePoint) == ContainmentType::Disjoint &&
                      boundaryOutsideOutput == ContainmentType::Disjoint,
                  "XNA includes a boundary point unless another plane places it outside");

        const BoundingBox innerBox(Vector3(-0.5f, -0.5f, 0.25f), Vector3(0.5f, 0.5f, 0.75f));
        const BoundingBox crossingBox(Vector3(-0.5f, -0.5f, -0.25f), Vector3(0.5f, 0.5f, 0.25f));
        const BoundingBox distantBox(Vector3(3.0f, -0.5f, 0.25f), Vector3(4.0f, 0.5f, 0.75f));
        const BoundingBox tangentBox(Vector3(-0.5f, -0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.5f));
        ContainmentType innerBoxOutput = ContainmentType::Disjoint;
        ContainmentType crossingBoxOutput = ContainmentType::Contains;
        ContainmentType distantBoxOutput = ContainmentType::Contains;
        identity.Contains(innerBox, innerBoxOutput);
        identity.Contains(crossingBox, crossingBoxOutput);
        identity.Contains(distantBox, distantBoxOutput);
        ctx.Check(identity.Contains(innerBox) == ContainmentType::Contains &&
                      identity.Contains(crossingBox) == ContainmentType::Intersects &&
                      identity.Contains(distantBox) == ContainmentType::Disjoint &&
                      innerBoxOutput == ContainmentType::Contains &&
                      crossingBoxOutput == ContainmentType::Intersects &&
                      distantBoxOutput == ContainmentType::Disjoint,
                  "box value and output forms produce all three containment classes");
        ContainmentType tangentBoxOutput = ContainmentType::Contains;
        identity.Contains(tangentBox, tangentBoxOutput);
        ctx.Check(identity.Contains(tangentBox) == ContainmentType::Intersects &&
                      tangentBoxOutput == ContainmentType::Intersects,
                  "a box face exactly on the frustum boundary is Intersects");

        const BoundingSphere innerSphere(Vector3(0.0f, 0.0f, 0.5f), 0.1f);
        const BoundingSphere crossingSphere(Vector3::Zero, 0.25f);
        const BoundingSphere distantSphere(Vector3(3.0f, 0.0f, 0.5f), 0.25f);
        const BoundingSphere tangentSphere(Vector3(0.0f, 0.0f, 0.25f), 0.25f);
        ContainmentType innerSphereOutput = ContainmentType::Disjoint;
        ContainmentType crossingSphereOutput = ContainmentType::Contains;
        ContainmentType distantSphereOutput = ContainmentType::Contains;
        identity.Contains(innerSphere, innerSphereOutput);
        identity.Contains(crossingSphere, crossingSphereOutput);
        identity.Contains(distantSphere, distantSphereOutput);
        ctx.Check(identity.Contains(innerSphere) == ContainmentType::Contains &&
                      identity.Contains(crossingSphere) == ContainmentType::Intersects &&
                      identity.Contains(distantSphere) == ContainmentType::Disjoint &&
                      innerSphereOutput == ContainmentType::Contains &&
                      crossingSphereOutput == ContainmentType::Intersects &&
                      distantSphereOutput == ContainmentType::Disjoint,
                  "sphere value and output forms produce all three containment classes");
        ContainmentType tangentSphereOutput = ContainmentType::Contains;
        identity.Contains(tangentSphere, tangentSphereOutput);
        ctx.Check(identity.Contains(tangentSphere) == ContainmentType::Intersects &&
                      tangentSphereOutput == ContainmentType::Intersects,
                  "a sphere surface exactly on the frustum boundary is Intersects");

        const BoundingFrustum equal(Matrix::getIdentityProperty());
        ctx.Check(identity.Contains(identity) == ContainmentType::Contains &&
                      identity.Contains(equal) == ContainmentType::Contains,
                  "self and a separate equal-matrix frustum are both geometrically contained");

        const BoundingFrustum inner(InnerMatrix());
        const BoundingFrustum larger(LargerMatrix());
        const BoundingFrustum distant(Matrix::CreateTranslation(-10.0f, 0.0f, 0.0f));
        ctx.Check(identity.Contains(inner) == ContainmentType::Contains &&
                      identity.Contains(larger) == ContainmentType::Intersects &&
                      identity.Contains(distant) == ContainmentType::Disjoint,
                  "strictly inner, overlapping larger and distant frusta produce all three classes");
    }
}

CNA_DEMOS_CONSOLE(BoundingFrustumContainment, {
    .id = "math.bounding_frustum.containment",
    .title = "Bounding-frustum containment",
    .description = "Classifies points, boxes, spheres and frusta through value/output paths, covering interiors, "
                   "crossings, exact boundaries, equal geometry and distant values.",
    .apis = {"BoundingFrustum::Contains", "ContainmentType::Contains",
             "ContainmentType::Intersects", "ContainmentType::Disjoint"},
    .tags = {"frustum", "containment", "bounding-volume", "culling"},
    .restrictions = "CNA-DEMOS-ISSUE-133 records point-boundary and equal-instance containment differences.",
    .knownIssue = "CNA-DEMOS-ISSUE-133",
})
