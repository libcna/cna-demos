// SPDX-License-Identifier: MS-PL
// BoundingSphere containment through value/output forms and exact boundaries.
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
    std::string_view Name(ContainmentType value)
    {
        switch (value)
        {
            case ContainmentType::Disjoint: return "Disjoint";
            case ContainmentType::Contains: return "Contains";
            case ContainmentType::Intersects: return "Intersects";
        }
        return "?";
    }

    void BoundingSphereContainmentBoundaries(CnaDemos::DemoContext& ctx)
    {
        const BoundingSphere outer(Vector3::Zero, 5.0f);

        const Vector3 insidePoint = Vector3::Zero;
        const Vector3 outsidePoint(6.0f, 0.0f, 0.0f);
        ContainmentType insidePointOutput = ContainmentType::Disjoint;
        ContainmentType outsidePointOutput = ContainmentType::Contains;
        outer.Contains(insidePoint, insidePointOutput);
        outer.Contains(outsidePoint, outsidePointOutput);
        ctx.Check(outer.Contains(insidePoint) == ContainmentType::Contains &&
                      insidePointOutput == ContainmentType::Contains &&
                      outer.Contains(outsidePoint) == ContainmentType::Disjoint &&
                      outsidePointOutput == ContainmentType::Disjoint,
                  "point value/output forms distinguish the interior and exterior");

        const Vector3 boundaryPoint(5.0f, 0.0f, 0.0f);
        ContainmentType boundaryPointOutput = ContainmentType::Contains;
        outer.Contains(boundaryPoint, boundaryPointOutput);
        ctx.Print("point on sphere surface: {} / {}", Name(outer.Contains(boundaryPoint)),
                  Name(boundaryPointOutput));
        ctx.Check(outer.Contains(boundaryPoint) == ContainmentType::Disjoint &&
                      boundaryPointOutput == ContainmentType::Disjoint,
                  "XNA classifies a point exactly on the sphere surface as Disjoint");

        const BoundingSphere insideSphere(Vector3::Zero, 1.0f);
        const BoundingSphere crossingSphere(Vector3(4.5f, 0.0f, 0.0f), 1.0f);
        const BoundingSphere disjointSphere(Vector3(7.0f, 0.0f, 0.0f), 1.0f);
        ContainmentType insideSphereOutput = ContainmentType::Disjoint;
        ContainmentType crossingSphereOutput = ContainmentType::Disjoint;
        ContainmentType disjointSphereOutput = ContainmentType::Contains;
        outer.Contains(insideSphere, insideSphereOutput);
        outer.Contains(crossingSphere, crossingSphereOutput);
        outer.Contains(disjointSphere, disjointSphereOutput);
        ctx.Check(insideSphereOutput == ContainmentType::Contains &&
                      crossingSphereOutput == ContainmentType::Intersects &&
                      disjointSphereOutput == ContainmentType::Disjoint,
                  "sphere output containment distinguishes inside, crossing and disjoint values");

        const BoundingSphere largerSphere(Vector3::Zero, 6.0f);
        ContainmentType largerSphereOutput = ContainmentType::Contains;
        outer.Contains(largerSphere, largerSphereOutput);
        ctx.Print("larger concentric sphere: {} / {}", Name(outer.Contains(largerSphere)),
                  Name(largerSphereOutput));
        ctx.Check(outer.Contains(largerSphere) == ContainmentType::Intersects &&
                      largerSphereOutput == ContainmentType::Intersects,
                  "a larger concentric sphere intersects rather than being contained");

        const BoundingBox insideBox(-Vector3::One, Vector3::One);
        const BoundingBox crossingBox(Vector3(4.0f, -1.0f, -1.0f), Vector3(6.0f, 1.0f, 1.0f));
        const BoundingBox disjointBox(Vector3(6.0f), Vector3(7.0f));
        ContainmentType insideBoxOutput = ContainmentType::Disjoint;
        ContainmentType crossingBoxOutput = ContainmentType::Disjoint;
        ContainmentType disjointBoxOutput = ContainmentType::Contains;
        outer.Contains(insideBox, insideBoxOutput);
        outer.Contains(crossingBox, crossingBoxOutput);
        outer.Contains(disjointBox, disjointBoxOutput);
        ctx.Check(insideBoxOutput == ContainmentType::Contains &&
                      crossingBoxOutput == ContainmentType::Intersects &&
                      disjointBoxOutput == ContainmentType::Disjoint,
                  "box output containment distinguishes inside, crossing and disjoint values");

        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());
        const BoundingSphere huge(Vector3::Zero, 5.0f);
        const BoundingSphere small(Vector3::Zero, 1.0f);
        const BoundingSphere far(Vector3(20.0f), 1.0f);
        const ContainmentType hugeResult = huge.Contains(identityFrustum);
        const ContainmentType smallResult = small.Contains(identityFrustum);
        const ContainmentType farResult = far.Contains(identityFrustum);
        ctx.Print("identity frustum in huge/small/far spheres: {} / {} / {}",
                  Name(hugeResult), Name(smallResult), Name(farResult));
        ctx.Check(hugeResult == ContainmentType::Contains &&
                      smallResult == ContainmentType::Intersects &&
                      farResult == ContainmentType::Disjoint,
                  "frustum containment distinguishes enclosing, clipped and distant spheres");
    }
}

CNA_DEMOS_CONSOLE(BoundingSphereContainmentBoundaries, {
    .id = "math.bounding_sphere.containment_boundaries",
    .title = "Bounding sphere containment boundaries",
    .description = "Classifies points, spheres, boxes and a frustum through BoundingSphere::Contains value/output "
                   "forms, including exact surfaces, larger concentric values and distant bounds.",
    .apis = {"BoundingSphere::Contains", "ContainmentType::Contains", "ContainmentType::Intersects",
             "ContainmentType::Disjoint"},
    .tags = {"bounds", "sphere", "containment", "collision"},
})
