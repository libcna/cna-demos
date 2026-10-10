// SPDX-License-Identifier: MS-PL
// Viewport's derived properties: Bounds, AspectRatio, TitleSafeArea and the depth range.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using Microsoft::Xna::Framework::Rectangle;
using Microsoft::Xna::Framework::Graphics::Viewport;

namespace
{
    void ViewportProperties(CnaDemos::DemoContext& ctx)
    {
        // The usual way to build one: position and size in back-buffer pixels.
        Viewport viewport(0, 0, 800, 480);
        ctx.Print("Viewport(0, 0, 800, 480)  {}", viewport.ToString());
        ctx.Check(viewport.getMinDepthProperty() == 0.0f && viewport.getMaxDepthProperty() == 1.0f,
                  "the (x, y, width, height) constructor sets the depth range to [0, 1]");

        // AspectRatio is Width / Height -- the value a perspective projection needs.
        ctx.Print("AspectRatio = {}", viewport.getAspectRatioProperty());
        ctx.CheckNear(viewport.getAspectRatioProperty(), 800.0 / 480.0, 1e-6, "AspectRatio is Width / Height");

        // Bounds is the same rectangle as a Rectangle, and can also be assigned.
        ctx.Check(viewport.getBoundsProperty() == Rectangle(0, 0, 800, 480), "Bounds is (X, Y, Width, Height)");
        viewport.setBoundsProperty(Rectangle(400, 0, 400, 480));
        ctx.Print("after Bounds = (400, 0, 400, 480)  {}", viewport.ToString());
        ctx.Check(viewport.getXProperty() == 400 && viewport.getWidthProperty() == 400,
                  "setting Bounds moves and resizes the viewport");

        // TitleSafeArea is the part guaranteed to be visible on a television. On a PC monitor
        // nothing is cut off, so it equals Bounds (as in XNA on Windows).
        const Rectangle safe = viewport.getTitleSafeAreaProperty();
        ctx.Print("TitleSafeArea = {{X:{} Y:{} Width:{} Height:{}}}", safe.X, safe.Y, safe.Width, safe.Height);
        ctx.Check(safe == viewport.getBoundsProperty(), "TitleSafeArea equals Bounds on a PC");

        // A default-constructed Viewport is all zeros, including the depth range, and its
        // AspectRatio is 0 rather than a division by zero.
        const Viewport empty;
        ctx.Print("Viewport()  {}", empty.ToString());
        ctx.Check(empty.getWidthProperty() == 0 && empty.getMaxDepthProperty() == 0.0f,
                  "the default constructor leaves every field zero");
        ctx.Check(empty.getAspectRatioProperty() == 0.0f, "AspectRatio of an empty viewport is 0");
    }
}

CNA_DEMOS_CONSOLE(ViewportProperties, {
    .id = "graphics.viewport.properties",
    .title = "Viewport properties",
    .description = "Builds a Viewport and reads its Bounds, AspectRatio, TitleSafeArea and depth range, including "
                   "what a default-constructed Viewport holds.",
    .apis = {"Viewport::Viewport", "Viewport::Bounds", "Viewport::AspectRatio", "Viewport::TitleSafeArea",
             "Viewport::MinDepth", "Viewport::MaxDepth", "Viewport::ToString"},
    .tags = {"viewport", "aspect-ratio", "safe-area"},
})
