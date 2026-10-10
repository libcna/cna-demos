// SPDX-License-Identifier: MS-PL
// Comparing, hashing and printing Vector3 values.
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
        ctx.Check(a == b && a.Equals(b), "identical components compare equal (== and Equals)");
        ctx.Check(a != c, "a difference of 0.001 makes the vectors unequal");

        // Equal vectors have equal hash codes.
        ctx.Check(a.GetHashCode() == b.GetHashCode(), "equal vectors hash equally");

        // For "close enough", compare a distance against a tolerance instead.
        ctx.Check(Vector3::Distance(a, c) < 0.01f, "a tolerance comparison treats them as the same point");
    }
}

CNA_DEMOS_CONSOLE(Vector3EqualityAndString, {
    .id = "math.vector3.equality_and_string",
    .title = "Compare and print vectors",
    .description = "Shows exact equality with == and Equals, hashing with GetHashCode, XNA's ToString format, "
                   "and a tolerance comparison for nearly equal vectors.",
    .apis = {"Vector3::Equals", "Vector3::op_Equality", "Vector3::op_Inequality", "Vector3::GetHashCode",
             "Vector3::ToString"},
    .tags = {"equality", "hash", "format"},
})
