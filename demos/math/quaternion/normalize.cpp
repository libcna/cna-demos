// SPDX-License-Identifier: MS-PL
// Keeping an accumulated orientation unit length with Quaternion::Normalize.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionNormalize(CnaDemos::DemoContext& ctx)
    {
        // A spaceship turning a little every frame: thousands of multiplications accumulate
        // rounding error, and a non-unit quaternion starts to scale what it rotates.
        const Quaternion perFrame = Quaternion::CreateFromYawPitchRoll(0.013f, 0.007f, 0.011f);
        Quaternion orientation = Quaternion::Identity;
        for (int frame = 0; frame < 100000; ++frame)
        {
            orientation = orientation * perFrame;
        }
        ctx.Print("length after 100000 updates          = {:.7f}", orientation.Length());

        const Quaternion fixed = Quaternion::Normalize(orientation);
        ctx.Print("length after Quaternion::Normalize   = {:.7f}", fixed.Length());
        ctx.CheckNear(fixed.Length(), 1.0, 1e-6, "Normalize restores unit length");

        orientation.Normalize();   // the in-place member form
        ctx.Check(orientation == fixed, "member Normalize() matches the static form");
        ctx.CheckNear(Vector3::Transform(Vector3::UnitX, fixed).Length(), 1.0, 1e-5, "rotating no longer scales vectors");
    }
}

CNA_DEMOS_CONSOLE(QuaternionNormalize, {
    .id = "math.quaternion.normalize",
    .title = "Normalize a quaternion",
    .description = "Accumulates many small rotations, measures the drift with Quaternion::Length and restores a "
                   "unit quaternion with Quaternion::Normalize.",
    .apis = {"Quaternion::Normalize", "Quaternion::Length", "Quaternion::op_Multiply"},
    .tags = {"quaternion", "precision", "drift"},
})
