// SPDX-License-Identifier: MS-PL
// CurveKey construction, mutation, cloning, equality and hashing.
#include <any>
#include <bit>
#include <cstdint>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/CurveContinuity.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    float FromBits(std::uint32_t bits)
    {
        return std::bit_cast<float>(bits);
    }

    void CurveKeyValueIdentity(CnaDemos::DemoContext& ctx)
    {
        const CurveKey minimal(1.0f, 2.0f);
        const CurveKey tangents(1.0f, 2.0f, 3.0f, 4.0f);
        const CurveKey stepped(1.0f, 2.0f, 3.0f, 4.0f, CurveContinuity::Step);
        ctx.Check(minimal.getPositionProperty() == 1.0f && minimal.getValueProperty() == 2.0f &&
                      minimal.getTangentInProperty() == 0.0f && minimal.getTangentOutProperty() == 0.0f &&
                      minimal.getContinuityProperty() == CurveContinuity::Smooth &&
                      tangents.getTangentInProperty() == 3.0f && tangents.getTangentOutProperty() == 4.0f &&
                      tangents.getContinuityProperty() == CurveContinuity::Smooth &&
                      stepped.getContinuityProperty() == CurveContinuity::Step,
                  "all three constructors preserve position/value and apply XNA's tangent/continuity defaults");

        CurveKey mutableKey = stepped;
        mutableKey.setValueProperty(-2.0f);
        mutableKey.setTangentInProperty(-3.0f);
        mutableKey.setTangentOutProperty(-4.0f);
        mutableKey.setContinuityProperty(CurveContinuity::Smooth);
        ctx.Check(mutableKey.getPositionProperty() == 1.0f && mutableKey.getValueProperty() == -2.0f &&
                      mutableKey.getTangentInProperty() == -3.0f &&
                      mutableKey.getTangentOutProperty() == -4.0f &&
                      mutableKey.getContinuityProperty() == CurveContinuity::Smooth,
                  "the four mutable properties change independently while Position remains read-only");

        const CurveKey clone = mutableKey.Clone();
        ctx.Check(mutableKey.Equals(clone) && mutableKey.Equals(std::any{clone}) &&
                      !mutableKey.Equals(std::any{42}) && !mutableKey.Equals(std::any{}),
                  "Clone retains typed and boxed identity while wrong and empty boxes are unequal");
        ctx.Check(mutableKey == clone && mutableKey != tangents &&
                      mutableKey.GetHashCode() == clone.GetHashCode(),
                  "operators and ordinary equal hashing include values, tangents and continuity");

        const float negativeZero = FromBits(0x80000000);
        const CurveKey positiveZero(0.0f, 1.0f, 2.0f, 3.0f, CurveContinuity::Smooth);
        const CurveKey negativePositionZero(negativeZero, 1.0f, 2.0f, 3.0f,
                                            CurveContinuity::Smooth);
        ctx.Check(positiveZero == negativePositionZero &&
                      positiveZero.GetHashCode() == negativePositionZero.GetHashCode(),
                  "equal keys differing only by Position's zero sign receive the same hash");

        const CurveKey nanValue(1.0f, std::numeric_limits<float>::quiet_NaN());
        ctx.Check(nanValue != nanValue && !nanValue.Equals(nanValue),
                  "a NaN field follows XNA float equality and makes even self-comparison unequal");
    }
}

CNA_DEMOS_CONSOLE(CurveKeyValueIdentity, {
    .id = "math.curve.key_value_identity",
    .title = "Curve-key value identity",
    .description = "Covers every CurveKey constructor and property plus clone, boxed/operator identity, ordinary and "
                   "signed-zero hashing, and NaN equality.",
    .apis = {"CurveKey::CurveKey", "CurveKey::Position", "CurveKey::Value", "CurveKey::TangentIn",
             "CurveKey::TangentOut", "CurveKey::Continuity", "CurveKey::Clone", "CurveKey::Equals",
             "CurveKey::GetHashCode", "CurveKey::op_Equality", "CurveKey::op_Inequality",
             "CurveContinuity::Smooth", "CurveContinuity::Step"},
    .tags = {"curve", "keyframe", "value-semantics", "ieee-754"},
    .restrictions = "CNA-DEMOS-ISSUE-120 includes CurveKey's unequal hashes for equal signed-zero positions.",
    .knownIssue = "CNA-DEMOS-ISSUE-120",
})
