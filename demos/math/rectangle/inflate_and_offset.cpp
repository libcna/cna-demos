// SPDX-License-Identifier: MS-PL
// Changing a Rectangle in place: Inflate, Offset and Location.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RectangleInflateAndOffset(CnaDemos::DemoContext& ctx)
    {
        Rectangle box(100, 100, 40, 20);

        // Inflate grows every side: here 5 pixels left and right, 2 above and below.
        box.Inflate(5, 2);
        ctx.Print("after Inflate(5, 2)   = {}", box.ToString());
        ctx.Check(box == Rectangle(95, 98, 50, 24), "Inflate moves the corner and grows the size by twice the amount");

        // Offset moves without resizing.
        box.Offset(10, -8);
        ctx.Print("after Offset(10, -8)  = {}", box.ToString());
        ctx.Check(box == Rectangle(105, 90, 50, 24), "Offset(x, y) moves the rectangle");
        box.Offset(Point(-5, 0));
        ctx.Check(box.X == 100, "Offset(Point) moves it too");

        // Location replaces the corner directly.
        box.setLocationProperty(Point(-20, 30));
        ctx.Print("after Location = -20,30 = {}", box.ToString());
        ctx.Check(box == Rectangle(-20, 30, 50, 24), "setting a signed Location keeps the size");

        Rectangle shrink(10, 20, 10, 8);
        shrink.Inflate(-2, -3);
        ctx.Check(shrink == Rectangle(12, 23, 6, 2),
                  "negative Inflate values shrink symmetrically");

        Rectangle overShrink(10, 20, 10, 8);
        overShrink.Inflate(-6, -5);
        ctx.Check(overShrink == Rectangle(16, 25, -2, -2),
                  "Inflate permits shrinking past zero into negative sizes");

        Rectangle byPoint(-10, 8, 7, 9);
        Rectangle byCoordinates = byPoint;
        byPoint.Offset(Point(4, -11));
        byCoordinates.Offset(4, -11);
        ctx.Check(byPoint == Rectangle(-6, -3, 7, 9) && byCoordinates == byPoint,
                  "both Offset overloads preserve size and agree for signed movement");
    }
}

CNA_DEMOS_CONSOLE(RectangleInflateAndOffset, {
    .id = "math.rectangle.inflate_and_offset",
    .title = "Grow and move a rectangle",
    .description = "Grows and shrinks a Rectangle with Inflate, including negative sizes, then moves it with both "
                   "Offset overloads and the Location property.",
    .apis = {"Rectangle::Inflate", "Rectangle::Offset", "Rectangle::Location"},
    .tags = {"rectangle", "layout"},
})
