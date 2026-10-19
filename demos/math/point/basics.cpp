// SPDX-License-Identifier: MS-PL
// Point: an integer X/Y pair for pixel and grid coordinates.
#include <any>
#include <limits>

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
        const Point equal(3, 7);
        ctx.Check(tile == equal && tile.Equals(equal) && tile.Equals(std::any{equal}) &&
                      !(tile != equal),
                  "typed, boxed and operator equality agree for equal coordinates");
        ctx.Check(tile != Point(7, 3), "coordinates are not interchangeable");
        ctx.Check(Point() == Point::Zero && Point::Zero == Point(0, 0) &&
                      Point::getZeroProperty() == Point::Zero,
                  "the default, Zero field and Zero property are all (0, 0)");
        ctx.Check(!tile.Equals(std::any{42}) && !tile.Equals(std::any{}),
                  "boxed equality rejects a different type and an empty object");
        ctx.Check(tile.GetHashCode() == Point(3, 7).GetHashCode(), "equal points hash equally");

        Point changed = tile;
        changed.X = -9;
        ctx.Check(tile == Point(3, 7) && changed == Point(-9, 7),
                  "copying a Point preserves value semantics when public coordinates change");

        const int maximum = std::numeric_limits<int>::max();
        const int minimum = std::numeric_limits<int>::min();
        ctx.Check(Point(3, 7).GetHashCode() == 10 && Point(5, -2).GetHashCode() == 3 &&
                      Point(maximum, 1).GetHashCode() == minimum &&
                      Point(minimum, -1).GetHashCode() == maximum,
                  "XNA hashes a Point by unchecked addition of X and Y");
        ctx.Check(Point(minimum, maximum).ToString() == "{X:-2147483648 Y:2147483647}",
                  "ToString preserves both signed 32-bit coordinate limits");

        const Point left(8, -6);
        const Point right(2, 3);
        ctx.Check(left + right == Point(10, -3) && left - right == Point(6, -9) &&
                      left * right == Point(16, -18) && left / right == Point(4, -2),
                  "CNA's Point arithmetic extensions operate component-wise");
    }
}

CNA_DEMOS_CONSOLE(PointBasics, {
    .id = "math.point.basics",
    .title = "Point basics",
    .description = "Creates integer Point values for grid or pixel coordinates, checks value and boxed identity, "
                   "hashes and text at signed limits, and demonstrates CNA's component arithmetic extensions.",
    .apis = {"Point::Point", "Point::Zero", "Point::X", "Point::Y", "Point::Equals", "Point::ToString",
             "Point::op_Equality", "Point::op_Inequality", "Point::GetHashCode", "Point::op_Addition",
             "Point::op_Subtraction", "Point::op_Multiply", "Point::op_Division"},
    .tags = {"integer", "grid", "pixel", "value-semantics"},
})
