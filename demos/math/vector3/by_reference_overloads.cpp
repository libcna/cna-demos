// SPDX-License-Identifier: MS-PL
// The by-reference ("out parameter") overloads that XNA offers next to every value-returning method.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3ByReference(CnaDemos::DemoContext& ctx)
    {
        const Vector3 a(1.0f, 2.0f, 3.0f);
        const Vector3 b(4.0f, 5.0f, 6.0f);

        // In C#, "Vector3.Add(ref a, ref b, out result)" avoided copying 12-byte structs. CNA keeps
        // the overloads with const references in and a reference out.
        Vector3 sum;
        Vector3::Add(a, b, sum);
        float dot = 0.0f;
        Vector3::Dot(a, b, dot);
        Vector3 cross;
        Vector3::Cross(a, b, cross);
        Vector3 lerped;
        Vector3::Lerp(a, b, 0.5f, lerped);

        ctx.Print("Add   -> {}", sum.ToString());
        ctx.Print("Dot   -> {}", dot);
        ctx.Print("Cross -> {}", cross.ToString());
        ctx.Print("Lerp  -> {}", lerped.ToString());

        // They compute exactly what the value-returning forms compute.
        ctx.Check(sum == Vector3::Add(a, b), "Add(a, b, result) matches Add(a, b)");
        ctx.Check(dot == Vector3::Dot(a, b), "Dot(a, b, result) matches Dot(a, b)");
        ctx.Check(cross == Vector3::Cross(a, b), "Cross(a, b, result) matches Cross(a, b)");
        ctx.Check(lerped == Vector3::Lerp(a, b, 0.5f), "Lerp(a, b, t, result) matches Lerp(a, b, t)");
    }
}

CNA_DEMOS_CONSOLE(Vector3ByReference, {
    .id = "math.vector3.by_reference_overloads",
    .title = "By-reference math overloads",
    .description = "Uses the overloads that write their result into a reference parameter (XNA's ref/out "
                   "pattern) and shows they match the value-returning forms.",
    .apis = {"Vector3::Add", "Vector3::Dot", "Vector3::Cross", "Vector3::Lerp"},
    .tags = {"performance", "out-parameter"},
})
