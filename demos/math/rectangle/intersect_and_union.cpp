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
    }
}

CNA_DEMOS_CONSOLE(RectangleIntersectAndUnion, {
    .id = "math.rectangle.intersect_and_union",
    .title = "Rectangle overlap and union",
    .description = "Tests rectangles for overlap with Rectangle::Intersects, computes the overlap with "
                   "Rectangle::Intersect and the combined bounds with Rectangle::Union.",
    .apis = {"Rectangle::Intersects", "Rectangle::Intersect", "Rectangle::Union"},
    .tags = {"rectangle", "collision", "2d"},
})
