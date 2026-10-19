// SPDX-License-Identifier: MS-PL
// An axis-aligned bounding box around a set of points, and its corners.
#include <any>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingBoxFromPoints(CnaDemos::DemoContext& ctx)
    {
        const std::vector<Vector3> vertices = {
            {1.0f, 0.0f, 2.0f}, {-3.0f, 4.0f, 0.5f}, {2.0f, -1.0f, -6.0f}, {0.0f, 2.0f, 1.0f}};

        const BoundingBox box = BoundingBox::CreateFromPoints(vertices);
        ctx.Print("CreateFromPoints -> {}", box.ToString());
        ctx.Check(box.Min == Vector3(-3.0f, -1.0f, -6.0f), "Min is the smallest coordinate on every axis");
        ctx.Check(box.Max == Vector3(2.0f, 4.0f, 2.0f), "Max is the largest coordinate on every axis");

        // The eight corners, e.g. for drawing the box or transforming it.
        const std::vector<Vector3> corners = box.GetCorners();
        ctx.Print("{} corners, first {} last {}", corners.size(), corners.front().ToString(), corners.back().ToString());
        ctx.Check(corners.size() == static_cast<std::size_t>(BoundingBox::CornerCount), "GetCorners returns CornerCount (8) points");

        const BoundingBox manual(Vector3(-3.0f, -1.0f, -6.0f), Vector3(2.0f, 4.0f, 2.0f));
        ctx.Check(manual == box, "the (min, max) constructor builds the same box");
        ctx.Check(box.Equals(manual) && box.Equals(std::any(manual)) && !box.Equals(std::any(17)) &&
                      !box.Equals(std::any{}) && box.GetHashCode() == manual.GetHashCode(),
                  "equal boxes agree through typed and boxed equality and hashing");
        const BoundingBox taller(manual.Min, Vector3(2.0f, 5.0f, 2.0f));
        ctx.Check(taller != box && !taller.Equals(box),
                  "changing one maximum coordinate makes two boxes unequal");

        ctx.Check(BoundingBox().Min == Vector3::Zero && BoundingBox().Max == Vector3::Zero,
                  "the default box has zero minimum and maximum corners");
        ctx.Check(manual.ToString() == "{Min:{X:-3 Y:-1 Z:-6} Max:{X:2 Y:4 Z:2}}",
                  "ToString uses XNA's single-braced Min/Max representation");

        const std::vector<Vector3> expectedCorners = {
            Vector3(-3.0f, 4.0f, 2.0f), Vector3(2.0f, 4.0f, 2.0f),
            Vector3(2.0f, -1.0f, 2.0f), Vector3(-3.0f, -1.0f, 2.0f),
            Vector3(-3.0f, 4.0f, -6.0f), Vector3(2.0f, 4.0f, -6.0f),
            Vector3(2.0f, -1.0f, -6.0f), Vector3(-3.0f, -1.0f, -6.0f)};
        ctx.Check(corners == expectedCorners, "GetCorners returns XNA's exact eight-corner order");

        std::vector<Vector3> cornerOutput(9, Vector3(90.0f));
        cornerOutput[8] = Vector3(90.0f, 91.0f, 92.0f);
        box.GetCorners(cornerOutput);
        bool outputMatches = cornerOutput[8] == Vector3(90.0f, 91.0f, 92.0f);
        for (std::size_t i = 0; i < expectedCorners.size(); ++i)
        {
            outputMatches = outputMatches && cornerOutput[i] == expectedCorners[i];
        }
        ctx.Check(outputMatches, "the corner output form writes eight values and preserves a trailing guard");

        std::vector<Vector3> shortOutput = {Vector3(7.0f), Vector3(8.0f)};
        const std::vector<Vector3> shortGuards = shortOutput;
        bool shortRejected = false;
        try { box.GetCorners(shortOutput); }
        catch (const System::ArgumentOutOfRangeException&) { shortRejected = true; }
        ctx.Check(shortRejected && shortOutput == shortGuards,
                  "a short corner output is rejected before any value changes");

        bool emptyRejected = false;
        try { (void)BoundingBox::CreateFromPoints({}); }
        catch (const System::ArgumentException&) { emptyRejected = true; }
        ctx.Check(emptyRejected, "CreateFromPoints rejects an empty collection");
        const Vector3 lonePoint(-2.0f, 3.0f, -4.0f);
        ctx.Check(BoundingBox::CreateFromPoints({lonePoint}) == BoundingBox(lonePoint, lonePoint),
                  "one point produces a degenerate box at that point");
    }
}

CNA_DEMOS_CONSOLE(BoundingBoxFromPoints, {
    .id = "math.bounding_box.create_from_points",
    .title = "Bounding box from points",
    .description = "Builds an axis-aligned BoundingBox from point sets, checks default/value identity and text, "
                   "and exercises both GetCorners forms with exact order and refusal boundaries.",
    .apis = {"BoundingBox::CreateFromPoints", "BoundingBox::GetCorners", "BoundingBox::CornerCount",
             "BoundingBox::Min", "BoundingBox::Max", "BoundingBox::BoundingBox", "BoundingBox::Equals",
             "BoundingBox::GetHashCode", "BoundingBox::ToString", "BoundingBox::op_Equality",
             "BoundingBox::op_Inequality"},
    .tags = {"bounds", "aabb", "collision"},
})
