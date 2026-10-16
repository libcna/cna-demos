// SPDX-License-Identifier: MS-PL
// BoundingFrustum intersections with volumes, planes and rays.
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/PlaneIntersectionType.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    Matrix LargerMatrix()
    {
        Matrix value = Matrix::getIdentityProperty();
        value.M11 = 0.5f;
        value.M22 = 0.5f;
        value.M33 = 0.5f;
        value.M43 = 0.25f;
        return value;
    }

    void BoundingFrustumIntersections(CnaDemos::DemoContext& ctx)
    {
        const BoundingFrustum identity(Matrix::getIdentityProperty());
        const BoundingBox innerBox(Vector3(-0.5f, -0.5f, 0.25f), Vector3(0.5f, 0.5f, 0.75f));
        const BoundingBox distantBox(Vector3(3.0f, -0.5f, 0.25f), Vector3(4.0f, 0.5f, 0.75f));
        bool innerOutput = false;
        bool distantOutput = true;
        identity.Intersects(innerBox, innerOutput);
        identity.Intersects(distantBox, distantOutput);
        ctx.Check(identity.Intersects(innerBox) && !identity.Intersects(distantBox) &&
                      innerOutput && !distantOutput,
                  "box value and output forms distinguish a contained hit from a distant miss");

        const BoundingSphere innerSphere(Vector3(0.0f, 0.0f, 0.5f), 0.1f);
        const BoundingSphere distantSphere(Vector3(3.0f, 0.0f, 0.5f), 0.25f);
        innerOutput = false;
        distantOutput = true;
        identity.Intersects(innerSphere, innerOutput);
        identity.Intersects(distantSphere, distantOutput);
        ctx.Check(identity.Intersects(innerSphere) && !identity.Intersects(distantSphere) &&
                      innerOutput && !distantOutput,
                  "sphere value and output forms distinguish a contained hit from a distant miss");

        const BoundingFrustum equal(Matrix::getIdentityProperty());
        const BoundingFrustum larger(LargerMatrix());
        const BoundingFrustum distant(Matrix::CreateTranslation(-10.0f, 0.0f, 0.0f));
        ctx.Check(identity.Intersects(equal) && identity.Intersects(larger) &&
                      !identity.Intersects(distant),
                  "equal, overlapping larger and distant frusta produce hit, hit and miss");

        const Plane crossing(Vector3::UnitY, 0.0f);
        const Plane front(Vector3::UnitX, 2.0f);
        const Plane back(Vector3::UnitX, -2.0f);
        PlaneIntersectionType crossingOutput = PlaneIntersectionType::Front;
        PlaneIntersectionType frontOutput = PlaneIntersectionType::Back;
        PlaneIntersectionType backOutput = PlaneIntersectionType::Front;
        identity.Intersects(crossing, crossingOutput);
        identity.Intersects(front, frontOutput);
        identity.Intersects(back, backOutput);
        ctx.Check(identity.Intersects(crossing) == PlaneIntersectionType::Intersecting &&
                      identity.Intersects(front) == PlaneIntersectionType::Front &&
                      identity.Intersects(back) == PlaneIntersectionType::Back &&
                      crossingOutput == PlaneIntersectionType::Intersecting &&
                      frontOutput == PlaneIntersectionType::Front &&
                      backOutput == PlaneIntersectionType::Back,
                  "plane value and output forms distinguish crossing, Front and Back");

        const Ray inside(Vector3(0.0f, 0.0f, 0.5f), Vector3::UnitZ);
        const Ray miss(Vector3(3.0f, 0.0f, -3.0f), Vector3::UnitZ);
        std::optional<float> insideOutput = 99.0f;
        std::optional<float> missOutput = 99.0f;
        identity.Intersects(inside, insideOutput);
        identity.Intersects(miss, missOutput);
        ctx.Check(identity.Intersects(inside) == 0.0f && insideOutput == 0.0f &&
                      !identity.Intersects(miss).has_value() && !missOutput.has_value(),
                  "ray value and output forms report zero for an interior start and no distant miss");

        const Ray entering(Vector3(0.0f, 0.0f, -3.0f), Vector3::UnitZ);
        const Ray boundary(Vector3::Zero, Vector3::UnitZ);
        std::optional<float> entryOutput = 99.0f;
        std::optional<float> boundaryValue = 99.0f;
        std::optional<float> boundaryOutput = 99.0f;
        bool boundaryValueThrew = false;
        bool boundaryOutputThrew = false;
        identity.Intersects(entering, entryOutput);
        try
        {
            boundaryValue = identity.Intersects(boundary);
        }
        catch (...)
        {
            boundaryValueThrew = true;
        }
        try
        {
            identity.Intersects(boundary, boundaryOutput);
        }
        catch (...)
        {
            boundaryOutputThrew = true;
        }
        ctx.Check(identity.Intersects(entering) == 3.0f && entryOutput == 3.0f &&
                      !boundaryValueThrew && !boundaryOutputThrew &&
                      boundaryValue == 0.0f && boundaryOutput == 0.0f,
                  "XNA ray value/output forms calculate outside entry and accept a boundary start at zero");
    }
}

CNA_DEMOS_CONSOLE(BoundingFrustumIntersections, {
    .id = "math.bounding_frustum.intersections",
    .title = "Bounding-frustum intersections",
    .description = "Intersects an identity BoundingFrustum with boxes, spheres, frusta, planes and Rays through "
                   "value/output paths, including outside entry and boundary-start ray behavior.",
    .apis = {"BoundingFrustum::Intersects"},
    .tags = {"frustum", "intersection", "bounding-volume", "ray"},
    .restrictions = "CNA-DEMOS-ISSUE-131 records incomplete outside-entry and boundary-start ray intersection.",
    .knownIssue = "CNA-DEMOS-ISSUE-131",
})
