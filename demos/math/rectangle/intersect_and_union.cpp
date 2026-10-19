// SPDX-License-Identifier: MS-PL
// Overlap tests and combined bounds: Rectangle::Intersects, Intersect and Union.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RectangleIntersectAndUnion(CnaDemos::DemoContext& ctx)
    {
        const Rectangle player(10, 10, 32, 32);
        const Rectangle coin(30, 30, 16, 16);
        const Rectangle wall(100, 0, 20, 200);

        // Intersects: a yes/no collision test.
        ctx.Check(player.Intersects(coin), "the player overlaps the coin");
        ctx.Check(!player.Intersects(wall), "the player does not touch the wall");
        ctx.Check(!player.Intersects(Rectangle(42, 10, 10, 10)), "touching edges do not count as overlap");

        // Intersect: the overlapping area (empty when there is none).
        const Rectangle overlap = Rectangle::Intersect(player, coin);
        ctx.Print("Intersect(player, coin) = {}", overlap.ToString());
        ctx.Check(overlap == Rectangle(30, 30, 12, 12), "the overlap is 12x12 at (30, 30)");
        ctx.Check(Rectangle::Intersect(player, wall) == Rectangle::Empty, "no overlap gives Rectangle::Empty");

        // Union: the smallest rectangle containing both (e.g. a dirty region).
        const Rectangle both = Rectangle::Union(player, wall);
        ctx.Print("Union(player, wall)     = {}", both.ToString());
        ctx.Check(both == Rectangle(10, 0, 110, 200), "the union spans from (10, 0) to (120, 200)");

        const Rectangle left(0, 0, 12, 12);
        const Rectangle right(10, 10, 5, 5);
        bool intersectsOutput = false;
        left.Intersects(right, intersectsOutput);
        const Rectangle touching(12, 0, 3, 3);
        bool touchingOutput = true;
        left.Intersects(touching, touchingOutput);
        ctx.Check(intersectsOutput && !touchingOutput,
                  "the Intersects output form preserves overlap and edge-touch behavior");

        Rectangle intersectionOutput;
        Rectangle::Intersect(left, right, intersectionOutput);
        Rectangle intersectionAliasLeft = left;
        Rectangle intersectionRight = right;
        Rectangle::Intersect(intersectionAliasLeft, intersectionRight, intersectionAliasLeft);
        Rectangle intersectionLeft = left;
        Rectangle intersectionAliasRight = right;
        Rectangle::Intersect(intersectionLeft, intersectionAliasRight, intersectionAliasRight);
        const Rectangle disjoint(30, 40, 2, 3);
        Rectangle disjointOutput(9, 8, 7, 6);
        Rectangle::Intersect(left, disjoint, disjointOutput);
        ctx.Check(intersectionOutput == Rectangle(10, 10, 2, 2) &&
                      intersectionAliasLeft == intersectionOutput &&
                      intersectionAliasRight == intersectionOutput && disjointOutput == Rectangle::Empty,
                  "Intersect output and input-alias forms agree, with Empty for a disjoint pair");

        Rectangle unionOutput;
        Rectangle::Union(left, right, unionOutput);
        Rectangle unionAliasLeft = right;
        Rectangle unionOtherLeft = left;
        Rectangle::Union(unionAliasLeft, unionOtherLeft, unionAliasLeft);
        Rectangle unionOtherRight = left;
        Rectangle unionAliasRight = right;
        Rectangle::Union(unionOtherRight, unionAliasRight, unionAliasRight);
        ctx.Check(unionOutput == Rectangle(0, 0, 15, 15) && unionAliasLeft == unionOutput &&
                      unionAliasRight == unionOutput,
                  "XNA Union output may alias either input without changing the combined bounds");

        ctx.Check(Rectangle::Union(Rectangle::Empty, right) == Rectangle(0, 0, 15, 15),
                  "Union includes Empty's origin when the other rectangle starts away from zero");
    }
}

CNA_DEMOS_CONSOLE(RectangleIntersectAndUnion, {
    .id = "math.rectangle.intersect_and_union",
    .title = "Rectangle overlap and union",
    .description = "Tests rectangles for overlap and computes intersections/unions through value, output and "
                   "input-alias forms, including touching, disjoint and Empty boundaries.",
    .apis = {"Rectangle::Intersects", "Rectangle::Intersect", "Rectangle::Union"},
    .tags = {"rectangle", "collision", "2d"},
})
