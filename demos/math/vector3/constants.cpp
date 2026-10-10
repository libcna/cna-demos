// SPDX-License-Identifier: MS-PL
// The named Vector3 constants and XNA's right-handed coordinate convention.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Constants(CnaDemos::DemoContext& ctx)
    {
        ctx.Print("Zero     = {}", Vector3::Zero.ToString());
        ctx.Print("One      = {}", Vector3::One.ToString());
        ctx.Print("UnitX    = {}   UnitY = {}   UnitZ = {}", Vector3::UnitX.ToString(), Vector3::UnitY.ToString(),
                  Vector3::UnitZ.ToString());
        ctx.Print("Right    = {}   Left  = {}", Vector3::Right.ToString(), Vector3::Left.ToString());
        ctx.Print("Up       = {}   Down  = {}", Vector3::Up.ToString(), Vector3::Down.ToString());
        ctx.Print("Forward  = {}   Backward = {}", Vector3::Forward.ToString(), Vector3::Backward.ToString());

        // XNA is right-handed: +X right, +Y up, and "forward" looks down the NEGATIVE Z axis.
        ctx.Check(Vector3::Right == Vector3::UnitX, "Right is +X");
        ctx.Check(Vector3::Up == Vector3::UnitY, "Up is +Y");
        ctx.Check(Vector3::Forward == Vector3(0.0f, 0.0f, -1.0f), "Forward is -Z");
        ctx.Check(Vector3::Backward == Vector3::UnitZ, "Backward is +Z");
        ctx.Check(Vector3::Left == -Vector3::Right && Vector3::Down == -Vector3::Up, "Left/Down are the negations");
    }
}

CNA_DEMOS_CONSOLE(Vector3Constants, {
    .id = "math.vector3.constants",
    .title = "Vector3 constants and axes",
    .description = "Prints Vector3::Zero, One, the unit axes and the direction constants, showing that XNA is "
                   "right-handed with Forward pointing down -Z.",
    .apis = {"Vector3::Zero", "Vector3::One", "Vector3::UnitX", "Vector3::UnitY", "Vector3::UnitZ", "Vector3::Up",
             "Vector3::Down", "Vector3::Left", "Vector3::Right", "Vector3::Forward", "Vector3::Backward"},
    .tags = {"coordinate-system", "axes", "right-handed"},
})
