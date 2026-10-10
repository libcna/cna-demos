// SPDX-License-Identifier: MS-PL
// Component-wise Vector3::Clamp, Min and Max.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3ClampMinMax(CnaDemos::DemoContext& ctx)
    {
        // Keep a position inside a room from (-10, 0, -10) to (10, 5, 10).
        const Vector3 roomMin(-10.0f, 0.0f, -10.0f);
        const Vector3 roomMax(10.0f, 5.0f, 10.0f);
        const Vector3 wanted(14.0f, -2.0f, 3.0f);

        const Vector3 clamped = Vector3::Clamp(wanted, roomMin, roomMax);
        ctx.Print("Clamp({}) = {}", wanted.ToString(), clamped.ToString());
        ctx.Check(clamped == Vector3(10.0f, 0.0f, 3.0f), "each component is clamped on its own");

        const Vector3 a(1.0f, 8.0f, -3.0f);
        const Vector3 b(4.0f, 2.0f, -1.0f);
        ctx.Print("Min({}, {}) = {}", a.ToString(), b.ToString(), Vector3::Min(a, b).ToString());
        ctx.Print("Max({}, {}) = {}", a.ToString(), b.ToString(), Vector3::Max(a, b).ToString());
        ctx.Check(Vector3::Min(a, b) == Vector3(1.0f, 2.0f, -3.0f), "Min takes the smaller value per component");
        ctx.Check(Vector3::Max(a, b) == Vector3(4.0f, 8.0f, -1.0f), "Max takes the larger value per component");
    }
}

CNA_DEMOS_CONSOLE(Vector3ClampMinMax, {
    .id = "math.vector3.clamp_min_max",
    .title = "Clamp, Min and Max",
    .description = "Limits a position to a box with Vector3::Clamp and combines vectors per component with "
                   "Vector3::Min and Vector3::Max.",
    .apis = {"Vector3::Clamp", "Vector3::Min", "Vector3::Max"},
    .tags = {"bounds", "component-wise"},
})
