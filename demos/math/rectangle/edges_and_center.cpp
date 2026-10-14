// SPDX-License-Identifier: MS-PL
// Reading a Rectangle: position, size, edges, centre and emptiness.
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RectangleEdges(CnaDemos::DemoContext& ctx)
    {
        // Integer rectangle: top-left corner (X, Y), Width, Height. Y grows downwards on screen.
        const Rectangle button(100, 50, 200, 40);
        ctx.Print("button = {}", button.ToString());
        ctx.Print("Left {} Top {} Right {} Bottom {}", button.getLeftProperty(), button.getTopProperty(),
                  button.getRightProperty(), button.getBottomProperty());
        ctx.Print("Location {}  Center {}", button.getLocationProperty().ToString(), button.getCenterProperty().ToString());

        ctx.Check(button.getRightProperty() == 300 && button.getBottomProperty() == 90, "Right = X + Width, Bottom = Y + Height");
        ctx.Check(button.getLocationProperty() == Point(100, 50), "Location is the top-left corner");
        ctx.Check(button.getCenterProperty() == Point(200, 70), "Center is the middle (integer division)");
        ctx.Check(!button.getIsEmptyProperty(), "a sized rectangle is not empty");
        ctx.Check(Rectangle::Empty.getIsEmptyProperty() && Rectangle() == Rectangle::Empty, "Rectangle::Empty is all zeros");

        const Rectangle same(100, 50, 200, 40);
        const Rectangle changed(100, 50, 201, 40);
        ctx.Check(button.Equals(same) && button.Equals(std::any{same}) &&
                      !button.Equals(std::any{42}) && !button.Equals(std::any{}),
                  "typed and boxed equality accept only the same position and size");
        ctx.Check(button == same && button != changed && button.GetHashCode() == same.GetHashCode(),
                  "rectangle operators and hashes follow all four fields");
    }
}

CNA_DEMOS_CONSOLE(RectangleEdges, {
    .id = "math.rectangle.edges_and_center",
    .title = "Rectangle edges and centre",
    .description = "Reads the Left, Top, Right, Bottom, Location and Center properties of a Rectangle and checks "
                   "for Rectangle::Empty.",
    .apis = {"Rectangle::Rectangle", "Rectangle::Left", "Rectangle::Top", "Rectangle::Right", "Rectangle::Bottom",
             "Rectangle::Location", "Rectangle::Center", "Rectangle::IsEmpty", "Rectangle::Empty",
             "Rectangle::ToString", "Rectangle::Equals", "Rectangle::GetHashCode", "Rectangle::op_Equality",
             "Rectangle::op_Inequality"},
    .tags = {"rectangle", "bounds", "ui"},
})
