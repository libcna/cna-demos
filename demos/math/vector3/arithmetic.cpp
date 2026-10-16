// SPDX-License-Identifier: MS-PL
// Component-wise Vector3 arithmetic: operators and the equivalent static methods.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Arithmetic(CnaDemos::DemoContext& ctx)
    {
        const Vector3 a(1.0f, 2.0f, 3.0f);
        const Vector3 b(4.0f, 5.0f, 6.0f);
        ctx.Check(a.X == 1.0f && a.Y == 2.0f && a.Z == 3.0f, "the constructor preserves X, Y and Z");

        ctx.Print("a + b     = {}", (a + b).ToString());
        ctx.Print("b - a     = {}", (b - a).ToString());
        ctx.Print("a * b     = {}   (component-wise)", (a * b).ToString());
        ctx.Print("a * 2     = {}", (a * 2.0f).ToString());
        ctx.Print("b / 2     = {}", (b / 2.0f).ToString());
        ctx.Print("-a        = {}", (-a).ToString());

        // Every operator has a named static counterpart (the C# API surface).
        ctx.Check(Vector3::Add(a, b) == a + b, "Add equals operator+");
        ctx.Check(Vector3::Subtract(b, a) == Vector3(3.0f, 3.0f, 3.0f), "Subtract gives (3, 3, 3)");
        ctx.Check(Vector3::Multiply(a, b) == Vector3(4.0f, 10.0f, 18.0f), "Multiply(Vector3, Vector3) is component-wise");
        ctx.Check(Vector3::Multiply(a, 2.0f) == Vector3(2.0f, 4.0f, 6.0f), "Multiply(Vector3, float) scales");
        ctx.Check(Vector3::Divide(b, 2.0f) == Vector3(2.0f, 2.5f, 3.0f), "Divide(Vector3, float) divides each component");
        ctx.Check(Vector3::Divide(b, a) == Vector3(4.0f, 2.5f, 2.0f), "Divide(Vector3, Vector3) is component-wise");
        ctx.Check(Vector3::Negate(a) == -a, "Negate equals unary minus");

        const Vector3 numerator(1.0f, -1.0f, 0.0f);
        const Vector3 divisors(+0.0f, -0.0f, +0.0f);
        const Vector3 componentQuotient = Vector3::Divide(numerator, divisors);
        Vector3 componentOutput;
        Vector3::Divide(numerator, divisors, componentOutput);
        ctx.Check(std::isinf(componentQuotient.X) && !std::signbit(componentQuotient.X) &&
                      std::isinf(componentQuotient.Y) && !std::signbit(componentQuotient.Y) &&
                      std::isnan(componentQuotient.Z) && componentOutput.X == componentQuotient.X &&
                      componentOutput.Y == componentQuotient.Y && std::isnan(componentOutput.Z),
                  "component division preserves IEEE infinity signs and zero-over-zero NaN");

        const Vector3 scalarQuotient = Vector3::Divide(numerator, -0.0f);
        Vector3 scalarOutput;
        Vector3::Divide(numerator, -0.0f, scalarOutput);
        ctx.Check(std::isinf(scalarQuotient.X) && std::signbit(scalarQuotient.X) &&
                      std::isinf(scalarQuotient.Y) && !std::signbit(scalarQuotient.Y) &&
                      std::isnan(scalarQuotient.Z) && scalarOutput.X == scalarQuotient.X &&
                      scalarOutput.Y == scalarQuotient.Y && std::isnan(scalarOutput.Z),
                  "negative-zero scalar division gives opposite infinities and a zero-over-zero NaN");
    }
}

CNA_DEMOS_CONSOLE(Vector3Arithmetic, {
    .id = "math.vector3.arithmetic",
    .title = "Vector3 arithmetic",
    .description = "Adds, subtracts, multiplies, divides and negates Vector3 values with operators and with "
                   "the equivalent static methods.",
    .apis = {"Vector3::Vector3", "Vector3::Add", "Vector3::Subtract", "Vector3::Multiply", "Vector3::Divide", "Vector3::Negate",
             "Vector3::op_Addition", "Vector3::op_Subtraction", "Vector3::op_Multiply", "Vector3::op_Division",
             "Vector3::op_UnaryNegation", "Vector3::X", "Vector3::Y", "Vector3::Z"},
    .tags = {"vector", "operators"},
})
