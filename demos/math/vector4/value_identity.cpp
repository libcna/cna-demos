// SPDX-License-Identifier: MS-PL
// Vector4 construction, constants and equality/hash identity boundaries.
#include <any>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector4ValueIdentity(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(Vector4() == Vector4::Zero && Vector4(-2.5f) == Vector4(-2.5f, -2.5f, -2.5f, -2.5f),
                  "default and replicated-scalar constructors preserve their four components");

        const Vector4 components(1.0f, -2.0f, 3.0f, -4.0f);
        ctx.Check(Vector4(Vector2(1.0f, -2.0f), 3.0f, -4.0f) == components &&
                      Vector4(Vector3(1.0f, -2.0f, 3.0f), -4.0f) == components,
                  "Vector2-plus-ZW and Vector3-plus-W constructors match the four-scalar form");

        ctx.Check(Vector4::UnitX + Vector4::UnitY + Vector4::UnitZ + Vector4::UnitW == Vector4::One &&
                      Vector4::Zero == Vector4(0.0f),
                  "Zero, One and all four unit axes expose the canonical basis");

        const Vector4 copy(Vector3(1.0f, -2.0f, 3.0f), -4.0f);
        ctx.Check(components.Equals(copy) && components.Equals(std::any{copy}) &&
                      components == copy && !(components != copy) &&
                      components.GetHashCode() == copy.GetHashCode(),
                  "typed, boxed and operator equality give ordinary equal values equal hashes");
        ctx.Check(!components.Equals(std::any{42}) && !components.Equals(std::any{}),
                  "boxed equality rejects a different type and an empty object");

        const Vector4 signedUnit(1.0f, -0.0f, 0.0f, 0.0f);
        const Vector4 signedZero(0.0f, 0.0f, 0.0f, -0.0f);
        ctx.Check(Vector4::UnitX.Equals(signedUnit) && Vector4::UnitX.Equals(std::any{signedUnit}) &&
                      Vector4::UnitX == signedUnit && !(Vector4::UnitX != signedUnit) &&
                      Vector4::UnitX.GetHashCode() == signedUnit.GetHashCode() &&
                      Vector4::Zero.Equals(signedZero) && Vector4::Zero.Equals(std::any{signedZero}) &&
                      Vector4::Zero == signedZero && !(Vector4::Zero != signedZero) &&
                      Vector4::Zero.GetHashCode() == signedZero.GetHashCode(),
                  "equal Vector4 values keep equal hashes when only a zero sign bit differs");

        const Vector4 nan(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f, 0.0f);
        const Vector4 nanCopy = nan;
        ctx.Check(!nan.Equals(nanCopy) && !(nan == nanCopy) && nan != nanCopy,
                  "a NaN component makes typed and operator equality false, even for a copy");
    }
}

CNA_DEMOS_CONSOLE(Vector4ValueIdentity, {
    .id = "math.vector4.value_identity",
    .title = "Vector4 value identity",
    .description = "Builds Vector4 values through every constructor, relates their constants and checks typed, "
                   "boxed and operator equality with ordinary, signed-zero and NaN components.",
    .apis = {"Vector4::Vector4", "Vector4::Zero", "Vector4::One", "Vector4::UnitX", "Vector4::UnitY",
             "Vector4::UnitZ", "Vector4::UnitW", "Vector4::X", "Vector4::Y", "Vector4::Z", "Vector4::W",
             "Vector4::Equals", "Vector4::GetHashCode", "Vector4::op_Equality", "Vector4::op_Inequality"},
    .tags = {"vector", "constructor", "equality", "hash"},
    .restrictions = "CNA-DEMOS-ISSUE-120 records signed-zero Vector4 hash inconsistency.",
    .knownIssue = "CNA-DEMOS-ISSUE-120",
})
