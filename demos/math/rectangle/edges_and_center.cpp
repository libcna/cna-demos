// SPDX-License-Identifier: MS-PL
// Reading a Rectangle: position, size, edges, centre and emptiness.
#include <any>
#include <limits>

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
        ctx.Check(button.X == 100 && button.Y == 50 && button.Width == 200 && button.Height == 40,
                  "the constructor preserves the public position and size fields");
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

        const Rectangle signedBounds(-9, -7, 5, 3);
        ctx.Check(signedBounds.getLeftProperty() == -9 && signedBounds.getTopProperty() == -7 &&
                      signedBounds.getRightProperty() == -4 && signedBounds.getBottomProperty() == -4 &&
                      signedBounds.getCenterProperty() == Point(-7, -6),
                  "signed edges and the center use the stored origin plus integer size arithmetic");

        const Rectangle negativeSize(10, 20, -5, -7);
        ctx.Check(negativeSize.getRightProperty() == 5 && negativeSize.getBottomProperty() == 13 &&
                      negativeSize.getCenterProperty() == Point(8, 17),
                  "negative odd sizes retain XNA's truncation-toward-zero center behavior");
        ctx.Check(Rectangle().getIsEmptyProperty() && Rectangle(0, 0, 0, 0).getIsEmptyProperty() &&
                      !Rectangle(1, 0, 0, 0).getIsEmptyProperty() &&
                      !Rectangle(0, 0, 0, 1).getIsEmptyProperty() && !negativeSize.getIsEmptyProperty(),
                  "IsEmpty is true only when all four fields are zero");
        ctx.Check(signedBounds.ToString() == "{X:-9 Y:-7 Width:5 Height:3}",
                  "ToString preserves signed positions and sizes");

        const int maximum = std::numeric_limits<int>::max();
        const int minimum = std::numeric_limits<int>::min();
        ctx.Check(Rectangle(1, 2, 3, 4).GetHashCode() == 10 &&
                      Rectangle(3, 7, 11, 13).GetHashCode() == 34 &&
                      Rectangle(5, -2, 9, -4).GetHashCode() == 8 &&
                      Rectangle(maximum, 1, 0, 0).GetHashCode() == minimum &&
                      Rectangle(minimum, -1, 0, 0).GetHashCode() == maximum,
                  "XNA hashes a Rectangle by unchecked addition of all four fields");
    }
}

CNA_DEMOS_CONSOLE(RectangleEdges, {
    .id = "math.rectangle.edges_and_center",
    .title = "Rectangle edges and centre",
    .description = "Reads Rectangle fields, edges, Location, Center and exact emptiness across ordinary and signed "
                   "sizes, then checks value identity, text and XNA hash behavior.",
    .apis = {"Rectangle::Rectangle", "Rectangle::Left", "Rectangle::Top", "Rectangle::Right", "Rectangle::Bottom",
             "Rectangle::Location", "Rectangle::Center", "Rectangle::IsEmpty", "Rectangle::Empty",
             "Rectangle::ToString", "Rectangle::Equals", "Rectangle::GetHashCode", "Rectangle::op_Equality",
             "Rectangle::op_Inequality", "Rectangle::X", "Rectangle::Y", "Rectangle::Width", "Rectangle::Height"},
    .tags = {"rectangle", "bounds", "ui"},
    .restrictions = "CNA-DEMOS-ISSUE-124 records Rectangle hash values that differ from XNA.",
    .knownIssue = "CNA-DEMOS-ISSUE-124",
})
