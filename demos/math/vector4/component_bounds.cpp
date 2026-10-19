// SPDX-License-Identifier: MS-PL
// Vector4 component bounds and arithmetic boundary behavior.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using Microsoft::Xna::Framework::Vector4;

namespace
{
    bool AllSignBits(const Vector4& value, bool sign)
    {
        return std::signbit(value.X) == sign && std::signbit(value.Y) == sign &&
               std::signbit(value.Z) == sign && std::signbit(value.W) == sign;
    }

    void Vector4ComponentBounds(CnaDemos::DemoContext& ctx)
    {
        const Vector4 low(-2.0f, 3.0f, 8.0f, -9.0f);
        const Vector4 high(1.0f, 7.0f, 6.0f, -4.0f);
        const Vector4 value(2.0f, 5.0f, 7.0f, -6.0f);
        Vector4 mixedOutput;
        Vector4::Clamp(value, low, high, mixedOutput);
        ctx.Check(Vector4::Clamp(value, low, high) == Vector4(1.0f, 5.0f, 8.0f, -6.0f) &&
                      mixedOutput == Vector4(1.0f, 5.0f, 8.0f, -6.0f) &&
                      Vector4::Min(low, high) == Vector4(-2.0f, 3.0f, 6.0f, -9.0f) &&
                      Vector4::Max(low, high) == Vector4(1.0f, 7.0f, 8.0f, -4.0f),
                  "Clamp, Min and Max independently handle mixed ordered and reversed component bounds");

        const Vector4 reversedMinimum(10.0f);
        const Vector4 reversedMaximum = Vector4::Zero;
        Vector4 reversedOutput;
        Vector4::Clamp(Vector4(5.0f, -5.0f, 2.0f, 20.0f), reversedMinimum,
                       reversedMaximum, reversedOutput);
        ctx.Check(Vector4::Clamp(Vector4(5.0f, -5.0f, 2.0f, 20.0f), reversedMinimum,
                                 reversedMaximum) == reversedMinimum &&
                      reversedOutput == reversedMinimum,
                  "XNA resolves reversed Clamp bounds to the minimum in value and output forms");

        const Vector4 nanValue =
            Vector4::Clamp(Vector4(NAN, 5.0f, 2.0f, -1.0f), Vector4::Zero, Vector4(4.0f));
        const Vector4 nanMinimum =
            Vector4::Clamp(Vector4(5.0f), Vector4(NAN, 0.0f, 0.0f, 0.0f), Vector4(4.0f));
        const Vector4 nanMaximum =
            Vector4::Clamp(Vector4(5.0f), Vector4::Zero, Vector4(NAN, 4.0f, 4.0f, 4.0f));
        ctx.Check(std::isnan(nanValue.X) && nanValue.Y == 4.0f && nanValue.Z == 2.0f &&
                      nanValue.W == 0.0f && nanMinimum == Vector4(4.0f) &&
                      nanMaximum == Vector4(5.0f, 4.0f, 4.0f, 4.0f),
                  "Clamp propagates a NaN value but ignores NaN bounds in their comparison positions");

        const Vector4 positiveZero(+0.0f);
        const Vector4 negativeZero(-0.0f);
        const Vector4 minPositiveNegative = Vector4::Min(positiveZero, negativeZero);
        const Vector4 minNegativePositive = Vector4::Min(negativeZero, positiveZero);
        const Vector4 maxPositiveNegative = Vector4::Max(positiveZero, negativeZero);
        const Vector4 maxNegativePositive = Vector4::Max(negativeZero, positiveZero);
        Vector4 minOutput;
        Vector4 maxOutput;
        Vector4::Min(positiveZero, negativeZero, minOutput);
        Vector4::Max(positiveZero, negativeZero, maxOutput);
        ctx.Check(AllSignBits(minPositiveNegative, true) && AllSignBits(minNegativePositive, false) &&
                      AllSignBits(maxPositiveNegative, true) && AllSignBits(maxNegativePositive, false) &&
                      AllSignBits(minOutput, true) && AllSignBits(maxOutput, true),
                  "XNA Min and Max choose the second operand when signed zeros compare equal");

        const Vector4 numerator(1.0f, -1.0f, 0.0f, -0.0f);
        const Vector4 divisors(+0.0f, -0.0f, +0.0f, -0.0f);
        const Vector4 componentQuotient = Vector4::Divide(numerator, divisors);
        Vector4 componentOutput;
        Vector4::Divide(numerator, divisors, componentOutput);
        ctx.Check(std::isinf(componentQuotient.X) && !std::signbit(componentQuotient.X) &&
                      std::isinf(componentQuotient.Y) && !std::signbit(componentQuotient.Y) &&
                      std::isnan(componentQuotient.Z) && std::isnan(componentQuotient.W) &&
                      componentOutput.X == componentQuotient.X && componentOutput.Y == componentQuotient.Y &&
                      std::isnan(componentOutput.Z) && std::isnan(componentOutput.W),
                  "component division preserves IEEE infinity signs and zero-over-zero NaNs");

        const Vector4 scalarQuotient = Vector4::Divide(numerator, -0.0f);
        Vector4 scalarOutput;
        Vector4::Divide(numerator, -0.0f, scalarOutput);
        ctx.Check(std::isinf(scalarQuotient.X) && std::signbit(scalarQuotient.X) &&
                      std::isinf(scalarQuotient.Y) && !std::signbit(scalarQuotient.Y) &&
                      std::isnan(scalarQuotient.Z) && std::isnan(scalarQuotient.W) &&
                      scalarOutput.X == scalarQuotient.X && scalarOutput.Y == scalarQuotient.Y &&
                      std::isnan(scalarOutput.Z) && std::isnan(scalarOutput.W),
                  "negative-zero scalar division gives opposite infinities and two NaNs");

        const Vector4 a(1.0f, -2.0f, 3.0f, -4.0f);
        const Vector4 b(5.0f, 6.0f, -7.0f, -8.0f);
        ctx.Check(Vector4::Add(a, b) == a + b && Vector4::Subtract(a, b) == a - b &&
                      Vector4::Multiply(a, b) == a * b && Vector4::Multiply(a, -2.0f) == a * -2.0f &&
                      -2.0f * a == a * -2.0f && Vector4::Negate(a) == -a,
                  "named arithmetic helpers match all corresponding operators");

        Vector4 output;
        Vector4::Add(a, b, output);
        const bool add = output == Vector4(6.0f, 4.0f, -4.0f, -12.0f);
        Vector4::Subtract(a, b, output);
        const bool subtract = output == Vector4(-4.0f, -8.0f, 10.0f, 4.0f);
        Vector4::Multiply(a, b, output);
        const bool multiplyComponents = output == Vector4(5.0f, -12.0f, -21.0f, 32.0f);
        Vector4::Multiply(a, -2.0f, output);
        const bool multiplyScalar = output == Vector4(-2.0f, 4.0f, -6.0f, 8.0f);
        Vector4::Negate(a, output);
        ctx.Check(add && subtract && multiplyComponents && multiplyScalar &&
                      output == Vector4(-1.0f, 2.0f, -3.0f, 4.0f),
                  "arithmetic output forms preserve every signed component");
    }
}

CNA_DEMOS_CONSOLE(Vector4ComponentBounds, {
    .id = "math.vector4.component_bounds",
    .title = "Vector4 component bounds",
    .description = "Exercises ordinary, reversed and NaN Clamp bounds, signed-zero Min/Max ties and component "
                   "arithmetic through named, operator and output forms.",
    .apis = {"Vector4::Clamp", "Vector4::Min", "Vector4::Max", "Vector4::Add", "Vector4::Subtract",
             "Vector4::Multiply", "Vector4::Divide", "Vector4::Negate", "Vector4::op_Addition",
             "Vector4::op_Subtraction", "Vector4::op_Multiply", "Vector4::op_Division",
             "Vector4::op_UnaryNegation"},
    .tags = {"vector", "bounds", "operators", "precision"},
})
