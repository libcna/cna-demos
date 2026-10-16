// SPDX-License-Identifier: MS-PL
// Plane classifications for boxes, spheres and frusta.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/PlaneIntersectionType.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void PlaneIntersections(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(static_cast<int>(PlaneIntersectionType::Front) == 0 &&
                      static_cast<int>(PlaneIntersectionType::Back) == 1 &&
                      static_cast<int>(PlaneIntersectionType::Intersecting) == 2,
                  "PlaneIntersectionType preserves XNA's three public enum values");

        const Plane split(Vector3::Up, 0.0f);
        const BoundingBox frontBox(Vector3(-1.0f, 1.0f, -1.0f), Vector3(1.0f, 3.0f, 1.0f));
        const BoundingBox backBox(Vector3(-1.0f, -3.0f, -1.0f), Vector3(1.0f, -1.0f, 1.0f));
        const BoundingBox crossingBox(-Vector3::One, Vector3::One);
        const BoundingBox tangentBox(Vector3(-1.0f, 0.0f, -1.0f), Vector3::One);
        PlaneIntersectionType frontOutput = PlaneIntersectionType::Back;
        PlaneIntersectionType backOutput = PlaneIntersectionType::Front;
        PlaneIntersectionType crossingOutput = PlaneIntersectionType::Front;
        PlaneIntersectionType tangentOutput = PlaneIntersectionType::Back;
        split.Intersects(frontBox, frontOutput);
        split.Intersects(backBox, backOutput);
        split.Intersects(crossingBox, crossingOutput);
        split.Intersects(tangentBox, tangentOutput);
        ctx.Check(split.Intersects(frontBox) == PlaneIntersectionType::Front &&
                      split.Intersects(backBox) == PlaneIntersectionType::Back &&
                      split.Intersects(crossingBox) == PlaneIntersectionType::Intersecting &&
                      frontOutput == PlaneIntersectionType::Front && backOutput == PlaneIntersectionType::Back &&
                      crossingOutput == PlaneIntersectionType::Intersecting,
                  "box value and output forms distinguish the two half-spaces and a crossing");
        ctx.Check(split.Intersects(tangentBox) == PlaneIntersectionType::Intersecting &&
                      tangentOutput == PlaneIntersectionType::Intersecting,
                  "a box face exactly on the plane is Intersecting");

        const BoundingSphere frontSphere(Vector3(0.0f, 2.0f, 0.0f), 1.0f);
        const BoundingSphere backSphere(Vector3(0.0f, -2.0f, 0.0f), 1.0f);
        const BoundingSphere crossingSphere(Vector3::Zero, 1.0f);
        const BoundingSphere tangentSphere(Vector3::Up, 1.0f);
        frontOutput = PlaneIntersectionType::Back;
        backOutput = PlaneIntersectionType::Front;
        crossingOutput = PlaneIntersectionType::Front;
        tangentOutput = PlaneIntersectionType::Back;
        split.Intersects(frontSphere, frontOutput);
        split.Intersects(backSphere, backOutput);
        split.Intersects(crossingSphere, crossingOutput);
        split.Intersects(tangentSphere, tangentOutput);
        ctx.Check(split.Intersects(frontSphere) == PlaneIntersectionType::Front &&
                      split.Intersects(backSphere) == PlaneIntersectionType::Back &&
                      split.Intersects(crossingSphere) == PlaneIntersectionType::Intersecting &&
                      frontOutput == PlaneIntersectionType::Front && backOutput == PlaneIntersectionType::Back &&
                      crossingOutput == PlaneIntersectionType::Intersecting,
                  "sphere value and output forms distinguish the two half-spaces and a crossing");
        ctx.Check(split.Intersects(tangentSphere) == PlaneIntersectionType::Intersecting &&
                      tangentOutput == PlaneIntersectionType::Intersecting,
                  "a sphere surface exactly on the plane is Intersecting");

        const BoundingFrustum identityFrustum(Matrix::getIdentityProperty());
        ctx.Check(split.Intersects(identityFrustum) == PlaneIntersectionType::Intersecting &&
                      Plane(Vector3::UnitX, 2.0f).Intersects(identityFrustum) == PlaneIntersectionType::Front &&
                      Plane(Vector3::UnitX, -2.0f).Intersects(identityFrustum) == PlaneIntersectionType::Back,
                  "frustum corners classify a crossing and both separated half-spaces");
    }
}

CNA_DEMOS_CONSOLE(PlaneIntersections, {
    .id = "math.plane.intersections",
    .title = "Plane and bounding-volume intersections",
    .description = "Classifies boxes, spheres and an identity frustum as Front, Back or Intersecting through Plane "
                   "value/output forms, including exact tangency.",
    .apis = {"Plane::Intersects", "PlaneIntersectionType", "PlaneIntersectionType::Front",
             "PlaneIntersectionType::Back", "PlaneIntersectionType::Intersecting"},
    .tags = {"plane", "bounding-volume", "classification", "collision"},
})
