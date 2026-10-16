// SPDX-License-Identifier: MS-PL
// Growing bounds: BoundingBox::CreateMerged and BoundingBox::CreateFromSphere.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingBoxMerge(CnaDemos::DemoContext& ctx)
    {
        // The bounds of a model made of two meshes is the merge of their bounds.
        const BoundingBox body(Vector3(-1.0f, 0.0f, -1.0f), Vector3(1.0f, 2.0f, 1.0f));
        const BoundingBox arm(Vector3(1.0f, 1.0f, -0.2f), Vector3(3.0f, 1.4f, 0.2f));
        const BoundingBox model = BoundingBox::CreateMerged(body, arm);
        ctx.Print("CreateMerged(body, arm) = {}", model.ToString());
        ctx.Check(model.Min == Vector3(-1.0f, 0.0f, -1.0f) && model.Max == Vector3(3.0f, 2.0f, 1.0f),
                  "the merged box encloses both");

        // The box that tightly encloses a sphere.
        const BoundingBox aroundBall = BoundingBox::CreateFromSphere(BoundingSphere(Vector3(5.0f, 5.0f, 5.0f), 2.0f));
        ctx.Print("CreateFromSphere(centre 5, radius 2) = {}", aroundBall.ToString());
        ctx.Check(aroundBall.Min == Vector3(3.0f, 3.0f, 3.0f) && aroundBall.Max == Vector3(7.0f, 7.0f, 7.0f),
                  "the box extends one radius from the centre on every axis");

        BoundingBox aroundBallOutput;
        const BoundingSphere ball(Vector3(5.0f, 5.0f, 5.0f), 2.0f);
        BoundingBox::CreateFromSphere(ball, aroundBallOutput);
        ctx.Check(aroundBallOutput == aroundBall,
                  "the CreateFromSphere output form agrees with the value form");

        BoundingBox mergedOutput;
        BoundingBox::CreateMerged(body, arm, mergedOutput);
        BoundingBox aliasLeft = body;
        BoundingBox otherLeft = arm;
        BoundingBox::CreateMerged(aliasLeft, otherLeft, aliasLeft);
        BoundingBox otherRight = body;
        BoundingBox aliasRight = arm;
        BoundingBox::CreateMerged(otherRight, aliasRight, aliasRight);
        ctx.Check(mergedOutput == model && aliasLeft == model && aliasRight == model,
                  "CreateMerged output may safely alias either input");
    }
}

CNA_DEMOS_CONSOLE(BoundingBoxMerge, {
    .id = "math.bounding_box.merge",
    .title = "Merge bounding boxes",
    .description = "Combines two boxes and encloses a sphere through value and output forms, including both "
                   "CreateMerged input-alias directions.",
    .apis = {"BoundingBox::CreateMerged", "BoundingBox::CreateFromSphere"},
    .tags = {"bounds", "aabb"},
})
