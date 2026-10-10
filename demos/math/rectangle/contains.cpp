// SPDX-License-Identifier: MS-PL
// Hit-testing with Rectangle::Contains (points, coordinates, other rectangles).
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RectangleContains(CnaDemos::DemoContext& ctx)
    {
        const Rectangle button(100, 50, 200, 40);   // covers x 100..299, y 50..89

        // A mouse click at (150, 60) hits the button.
        ctx.Check(button.Contains(Point(150, 60)), "a point inside is contained");
        ctx.Check(button.Contains(150, 60), "the (x, y) overload agrees");

        // Left/Top are inclusive, Right/Bottom are exclusive.
        ctx.Check(button.Contains(100, 50), "the top-left corner is inside");
        ctx.Check(!button.Contains(300, 60), "x = Right is outside");
        ctx.Check(!button.Contains(150, 90), "y = Bottom is outside");

        // A rectangle is contained only when it lies completely inside.
        ctx.Check(button.Contains(Rectangle(110, 55, 50, 20)), "a smaller rectangle inside is contained");
        ctx.Check(!button.Contains(Rectangle(250, 55, 100, 20)), "a rectangle sticking out is not contained");

        bool result = false;
        button.Contains(Point(120, 70), result);
        ctx.Check(result, "the by-reference overload reports the same");
        ctx.Print("Contains checks done for {}", button.ToString());
    }
}

CNA_DEMOS_CONSOLE(RectangleContains, {
    .id = "math.rectangle.contains",
    .title = "Point and rectangle containment",
    .description = "Hit-tests points and rectangles with Rectangle::Contains, showing that Left/Top are inclusive "
                   "and Right/Bottom exclusive.",
    .apis = {"Rectangle::Contains"},
    .tags = {"rectangle", "hit-test", "mouse"},
})
