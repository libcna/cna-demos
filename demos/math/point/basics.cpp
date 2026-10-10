// SPDX-License-Identifier: MS-PL
// Point: an integer X/Y pair for pixel and grid coordinates.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"

using Microsoft::Xna::Framework::Point;

namespace
{
    void PointBasics(CnaDemos::DemoContext& ctx)
    {
        const Point tile(3, 7);   // e.g. a cell in a tile map
        ctx.Print("tile = {}, X = {}, Y = {}", tile.ToString(), tile.X, tile.Y);
        ctx.Print("Point::Zero = {}", Point::Zero.ToString());

        ctx.Check(tile.ToString() == "{X:3 Y:7}", "ToString uses the {X:.. Y:..} format");
        ctx.Check(tile == Point(3, 7) && tile.Equals(Point(3, 7)), "points with equal coordinates are equal");
        ctx.Check(tile != Point(7, 3), "coordinates are not interchangeable");
        ctx.Check(Point::Zero == Point(0, 0) && Point::getZeroProperty() == Point::Zero, "Zero is (0, 0)");
        ctx.Check(tile.GetHashCode() == Point(3, 7).GetHashCode(), "equal points hash equally");
    }
}

CNA_DEMOS_CONSOLE(PointBasics, {
    .id = "math.point.basics",
    .title = "Point basics",
    .description = "Creates integer Point values for grid or pixel coordinates and compares, hashes and prints "
                   "them.",
    .apis = {"Point::Point", "Point::Zero", "Point::X", "Point::Y", "Point::Equals", "Point::ToString",
             "Point::op_Equality", "Point::op_Inequality", "Point::GetHashCode"},
    .tags = {"integer", "grid", "pixel"},
})
