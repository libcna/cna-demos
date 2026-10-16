// SPDX-License-Identifier: MS-PL
// Comparing, hashing and printing Vector3 values.
#include <any>
#include <unordered_set>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3EqualityAndString(CnaDemos::DemoContext& ctx)
    {
        const Vector3 a(1.5f, -2.0f, 0.25f);
        const Vector3 b(1.5f, -2.0f, 0.25f);
        const Vector3 c = a + Vector3(0.0f, 0.0f, 1e-3f);

        // ToString uses XNA's "{X:.. Y:.. Z:..}" format.
        ctx.Print("a.ToString() = {}", a.ToString());
        ctx.Check(a.ToString() == "{X:1.5 Y:-2 Z:0.25}", "ToString uses the {X:.. Y:.. Z:..} format");

        // Equality is exact, component by component.
        ctx.Check(a == b && a.Equals(b) && a.Equals(std::any{b}) &&
                      !a.Equals(std::any{42}) && !a.Equals(std::any{}),
                  "typed, boxed and operator equality accept only identical Vector3 values");
        ctx.Check(a != c, "a difference of 0.001 makes the vectors unequal");

        // Equal vectors have equal hash codes.
        ctx.Check(a.GetHashCode() == b.GetHashCode(), "equal vectors hash equally");

        // For "close enough", compare a distance against a tolerance instead.
        ctx.Check(Vector3::Distance(a, c) < 0.01f, "a tolerance comparison treats them as the same point");

        const Vector3 signedZeroUnitX(1.0f, -0.0f, 0.0f);
        const Vector3 signedZeroOrigin(-0.0f, 0.0f, 0.0f);
        ctx.Check(Vector3::UnitX.Equals(signedZeroUnitX) &&
                      Vector3::UnitX.Equals(std::any{signedZeroUnitX}) &&
                      Vector3::UnitX == signedZeroUnitX && !(Vector3::UnitX != signedZeroUnitX) &&
                      Vector3::Zero.Equals(signedZeroOrigin) && Vector3::Zero == signedZeroOrigin,
                  "positive and negative zero compare equal through every Vector3 equality form");
        ctx.Check(Vector3::UnitX.GetHashCode() == signedZeroUnitX.GetHashCode() &&
                      Vector3::Zero.GetHashCode() == signedZeroOrigin.GetHashCode(),
                  "XNA gives equal signed-zero Vector3 values equal hash codes");
    }
}

CNA_DEMOS_CONSOLE(Vector3EqualityAndString, {
    .id = "math.vector3.equality_and_string",
    .title = "Compare and print vectors",
    .description = "Shows typed, boxed and operator equality, ordinary and signed-zero hashing, XNA's ToString "
                   "format and a tolerance comparison.",
    .apis = {"Vector3::Equals", "Vector3::op_Equality", "Vector3::op_Inequality", "Vector3::GetHashCode",
             "Vector3::ToString"},
    .tags = {"equality", "hash", "format"},
    .restrictions = "CNA-DEMOS-ISSUE-120 records unequal hashes for equal signed-zero Vector3 values.",
    .knownIssue = "CNA-DEMOS-ISSUE-120",
})
