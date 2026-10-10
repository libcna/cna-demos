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
        box.setLocationProperty(Point(0, 0));
        ctx.Print("after Location = 0,0  = {}", box.ToString());
        ctx.Check(box == Rectangle(0, 0, 50, 24), "setting Location keeps the size");
    }
}

CNA_DEMOS_CONSOLE(RectangleInflateAndOffset, {
    .id = "math.rectangle.inflate_and_offset",
    .title = "Grow and move a rectangle",
    .description = "Grows a Rectangle on every side with Inflate, moves it with both Offset overloads and "
                   "repositions it through the Location property.",
    .apis = {"Rectangle::Inflate", "Rectangle::Offset", "Rectangle::Location"},
    .tags = {"rectangle", "layout"},
})
