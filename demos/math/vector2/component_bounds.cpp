// SPDX-License-Identifier: MS-PL
// Vector2 component bounds, IEEE-754 division and reflection boundary behavior.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using Microsoft::Xna::Framework::Vector2;

namespace
{
    void Vector2ComponentBounds(CnaDemos::DemoContext& ctx)
    {
        const Vector2 value(5.0f, -5.0f);
        const Vector2 reversedMinimum(10.0f, 10.0f);
        const Vector2 reversedMaximum(0.0f, 0.0f);
        Vector2 clampOutput;
        Vector2::Clamp(value, reversedMinimum, reversedMaximum, clampOutput);
        ctx.Check(Vector2::Clamp(value, reversedMinimum, reversedMaximum) == reversedMinimum &&
                      clampOutput == reversedMinimum,
                  "XNA resolves reversed Clamp bounds to the minimum in value and output forms");

        const Vector2 nanValue = Vector2::Clamp(Vector2(NAN, 5.0f), Vector2(0.0f), Vector2(4.0f));
        const Vector2 nanMinimum = Vector2::Clamp(Vector2(5.0f), Vector2(NAN, 0.0f), Vector2(4.0f));
        const Vector2 nanMaximum = Vector2::Clamp(Vector2(5.0f), Vector2(0.0f), Vector2(NAN, 4.0f));
        ctx.Check(std::isnan(nanValue.X) && nanValue.Y == 4.0f &&
                      nanMinimum == Vector2(4.0f, 4.0f) && nanMaximum == Vector2(5.0f, 4.0f),
                  "Clamp propagates a NaN value but ignores NaN bounds in their comparison positions");

        const Vector2 positiveZero(+0.0f, +0.0f);
        const Vector2 negativeZero(-0.0f, -0.0f);
        const Vector2 minPositiveNegative = Vector2::Min(positiveZero, negativeZero);
        const Vector2 minNegativePositive = Vector2::Min(negativeZero, positiveZero);
        const Vector2 maxPositiveNegative = Vector2::Max(positiveZero, negativeZero);
        const Vector2 maxNegativePositive = Vector2::Max(negativeZero, positiveZero);
        Vector2 minOutput;
        Vector2 maxOutput;
        Vector2::Min(positiveZero, negativeZero, minOutput);
        Vector2::Max(positiveZero, negativeZero, maxOutput);
        ctx.Check(std::signbit(minPositiveNegative.X) && std::signbit(minPositiveNegative.Y) &&
                      !std::signbit(minNegativePositive.X) && !std::signbit(minNegativePositive.Y) &&
                      std::signbit(maxPositiveNegative.X) && std::signbit(maxPositiveNegative.Y) &&
                      !std::signbit(maxNegativePositive.X) && !std::signbit(maxNegativePositive.Y) &&
                      std::signbit(minOutput.X) && std::signbit(minOutput.Y) &&
                      std::signbit(maxOutput.X) && std::signbit(maxOutput.Y),
                  "XNA Min and Max choose the second operand when signed zeros compare equal");

        const Vector2 numerator(1.0f, -1.0f);
        const Vector2 divisors(+0.0f, -0.0f);
        const Vector2 componentQuotient = Vector2::Divide(numerator, divisors);
        Vector2 componentOutput;
        Vector2::Divide(numerator, divisors, componentOutput);
        ctx.Check(std::isinf(componentQuotient.X) && !std::signbit(componentQuotient.X) &&
                      std::isinf(componentQuotient.Y) && !std::signbit(componentQuotient.Y) &&
                      componentOutput == componentQuotient,
                  "component division preserves IEEE signs and its output form agrees");

        const Vector2 scalarQuotient = Vector2::Divide(numerator, -0.0f);
        Vector2 scalarOutput;
        Vector2::Divide(numerator, -0.0f, scalarOutput);
        ctx.Check(std::isinf(scalarQuotient.X) && std::signbit(scalarQuotient.X) &&
                      std::isinf(scalarQuotient.Y) && !std::signbit(scalarQuotient.Y) &&
                      scalarOutput == scalarQuotient,
                  "negative-zero scalar division gives opposite signed infinities");

        const Vector2 incident(3.0f, 4.0f);
        ctx.Check(Vector2::Reflect(incident, Vector2(0.0f, -1.0f)) == Vector2(3.0f, -4.0f) &&
                      Vector2::Reflect(incident, Vector2::Zero) == incident,
                  "Reflect uses a unit normal directly and a zero normal leaves the vector unchanged");

        const Vector2 scaledNormal(0.0f, -2.0f);
        const Vector2 scaledReflection = Vector2::Reflect(incident, scaledNormal);
        Vector2 reflectionOutput;
        Vector2::Reflect(incident, scaledNormal, reflectionOutput);
        Vector2 reflectionAlias = scaledNormal;
        Vector2::Reflect(incident, reflectionAlias, reflectionAlias);
        ctx.Check(scaledReflection == Vector2(3.0f, -28.0f) &&
                      reflectionOutput == scaledReflection && reflectionAlias == scaledReflection,
                  "Reflect does not normalize its normal and its output may alias that normal");
    }
}

CNA_DEMOS_CONSOLE(Vector2ComponentBounds, {
    .id = "math.vector2.component_bounds",
    .title = "Vector2 component boundaries",
    .description = "Exercises reversed and NaN Clamp bounds, signed-zero Min/Max ties, IEEE division and "
                   "unit, zero and scaled-normal reflection.",
    .apis = {"Vector2::Clamp", "Vector2::Min", "Vector2::Max", "Vector2::Divide",
             "Vector2::Reflect"},
    .tags = {"vector", "bounds", "precision"},
    .restrictions = "CNA-DEMOS-ISSUE-122 records reversed Clamp and signed-zero tie differences.",
    .knownIssue = "CNA-DEMOS-ISSUE-122",
})
