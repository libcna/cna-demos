// SPDX-License-Identifier: MS-PL
// Quaternion value identity, constructor forms and the equality/hash contract at signed zero.
#include <any>
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionValueIdentity(CnaDemos::DemoContext& ctx)
    {
        const Quaternion zero;
        ctx.Check(zero.X == 0.0f && zero.Y == 0.0f && zero.Z == 0.0f && zero.W == 0.0f &&
                      zero != Quaternion::Identity,
                  "the default value has four zero components and differs from rotation Identity");

        const Quaternion components(1.0f, -2.0f, 3.0f, -4.0f);
        const Quaternion vectorScalar(Vector3(1.0f, -2.0f, 3.0f), -4.0f);
        ctx.Check(vectorScalar == components,
                  "the vector-plus-scalar constructor preserves the same XYZW component order");
        ctx.Check(components.Equals(vectorScalar) && components.Equals(std::any{vectorScalar}) &&
                      components == vectorScalar && !(components != vectorScalar) &&
                      components.GetHashCode() == vectorScalar.GetHashCode(),
                  "typed, boxed and operator equality give ordinary equal values the same hash");

        const float negativeZero = std::copysign(0.0f, -1.0f);
        const Quaternion signedX(negativeZero, 0.0f, 0.0f, 1.0f);
        const Quaternion signedW(0.0f, 0.0f, 0.0f, negativeZero);
        ctx.Check(Quaternion::Identity.Equals(signedX) && Quaternion::Identity.Equals(std::any{signedX}) &&
                      Quaternion::Identity == signedX && !(Quaternion::Identity != signedX) &&
                      zero.Equals(signedW) && zero == signedW,
                  "positive and negative zero components compare equal through every equality form");
        ctx.Check(Quaternion::Identity.GetHashCode() == signedX.GetHashCode() &&
                      zero.GetHashCode() == signedW.GetHashCode(),
                  "XNA gives equal signed-zero Quaternion values equal hash codes");
    }
}

CNA_DEMOS_CONSOLE(QuaternionValueIdentity, {
    .id = "math.quaternion.value_identity",
    .title = "Quaternion value identity",
    .description = "Compares constructor forms and proves that typed, boxed and operator equality remain "
                   "hash-consistent when a component uses negative zero.",
    .apis = {"Quaternion::Quaternion", "Quaternion::Identity", "Quaternion::Equals",
             "Quaternion::GetHashCode", "Quaternion::op_Equality", "Quaternion::op_Inequality",
             "Quaternion::X", "Quaternion::Y", "Quaternion::Z", "Quaternion::W"},
    .tags = {"quaternion", "equality", "hash", "precision"},
    .restrictions = "CNA-DEMOS-ISSUE-120 records unequal hashes for equal signed-zero values.",
    .knownIssue = "CNA-DEMOS-ISSUE-120",
})
