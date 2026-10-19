// SPDX-License-Identifier: MS-PL
// Component-wise Vector3::Clamp, Min and Max.
#include <cmath>

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

        const Vector3 reversedMinimum(10.0f);
        const Vector3 reversedMaximum = Vector3::Zero;
        Vector3 reversedOutput;
        Vector3::Clamp(Vector3(5.0f, -5.0f, 2.0f), reversedMinimum, reversedMaximum,
                       reversedOutput);
        ctx.Check(Vector3::Clamp(Vector3(5.0f, -5.0f, 2.0f), reversedMinimum, reversedMaximum) ==
                      reversedMinimum && reversedOutput == reversedMinimum,
                  "XNA resolves reversed Clamp bounds to the minimum in value and output forms");

        const Vector3 nanValue =
            Vector3::Clamp(Vector3(NAN, 5.0f, 2.0f), Vector3::Zero, Vector3(4.0f));
        const Vector3 nanMinimum =
            Vector3::Clamp(Vector3(5.0f), Vector3(NAN, 0.0f, 0.0f), Vector3(4.0f));
        const Vector3 nanMaximum =
            Vector3::Clamp(Vector3(5.0f), Vector3::Zero, Vector3(NAN, 4.0f, 4.0f));
        ctx.Check(std::isnan(nanValue.X) && nanValue.Y == 4.0f && nanValue.Z == 2.0f &&
                      nanMinimum == Vector3(4.0f) && nanMaximum == Vector3(5.0f, 4.0f, 4.0f),
                  "Clamp propagates a NaN value but ignores NaN bounds in their comparison positions");

        const Vector3 positiveZero(+0.0f);
        const Vector3 negativeZero(-0.0f);
        const Vector3 minPositiveNegative = Vector3::Min(positiveZero, negativeZero);
        const Vector3 minNegativePositive = Vector3::Min(negativeZero, positiveZero);
        const Vector3 maxPositiveNegative = Vector3::Max(positiveZero, negativeZero);
        const Vector3 maxNegativePositive = Vector3::Max(negativeZero, positiveZero);
        Vector3 minOutput;
        Vector3 maxOutput;
        Vector3::Min(positiveZero, negativeZero, minOutput);
        Vector3::Max(positiveZero, negativeZero, maxOutput);
        ctx.Check(std::signbit(minPositiveNegative.X) && std::signbit(minPositiveNegative.Y) &&
                      std::signbit(minPositiveNegative.Z) && !std::signbit(minNegativePositive.X) &&
                      !std::signbit(minNegativePositive.Y) && !std::signbit(minNegativePositive.Z) &&
                      std::signbit(maxPositiveNegative.X) && std::signbit(maxPositiveNegative.Y) &&
                      std::signbit(maxPositiveNegative.Z) && !std::signbit(maxNegativePositive.X) &&
                      !std::signbit(maxNegativePositive.Y) && !std::signbit(maxNegativePositive.Z) &&
                      std::signbit(minOutput.X) && std::signbit(minOutput.Y) && std::signbit(minOutput.Z) &&
                      std::signbit(maxOutput.X) && std::signbit(maxOutput.Y) && std::signbit(maxOutput.Z),
                  "XNA Min and Max choose the second operand when signed zeros compare equal");
    }
}

CNA_DEMOS_CONSOLE(Vector3ClampMinMax, {
    .id = "math.vector3.clamp_min_max",
    .title = "Clamp, Min and Max",
    .description = "Exercises ordinary, reversed and NaN Vector3 Clamp bounds plus ordinary and signed-zero "
                   "Min/Max tie behavior.",
    .apis = {"Vector3::Clamp", "Vector3::Min", "Vector3::Max"},
    .tags = {"bounds", "component-wise", "precision"},
})
