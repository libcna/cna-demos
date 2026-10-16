// SPDX-License-Identifier: MS-PL
// CurveKey CompareTo ordering for ordinary, infinite, signed-zero and NaN positions.
#include <bit>
#include <cstdint>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    float FromBits(std::uint32_t bits)
    {
        return std::bit_cast<float>(bits);
    }

    void CurveKeyOrdering(CnaDemos::DemoContext& ctx)
    {
        const CurveKey low(-1.0f, 10.0f);
        const CurveKey middle(1.0f, 20.0f);
        const CurveKey high(3.0f, 30.0f);
        ctx.Check(middle.CompareTo(low) > 0 && middle.CompareTo(high) < 0 &&
                      middle.CompareTo(CurveKey(1.0f, 99.0f)) == 0,
                  "CompareTo orders only by Position and ignores the remaining key fields");

        const CurveKey negativeInfinity(-std::numeric_limits<float>::infinity(), 0.0f);
        const CurveKey positiveInfinity(std::numeric_limits<float>::infinity(), 0.0f);
        ctx.Check(negativeInfinity.CompareTo(low) < 0 && positiveInfinity.CompareTo(high) > 0 &&
                      low.CompareTo(negativeInfinity) > 0 && high.CompareTo(positiveInfinity) < 0,
                  "infinite positions remain the two ordered extremes");

        const CurveKey positiveZero(0.0f, 1.0f);
        const CurveKey negativeZero(FromBits(0x80000000), 2.0f);
        ctx.Check(positiveZero.CompareTo(negativeZero) == 0 &&
                      negativeZero.CompareTo(positiveZero) == 0,
                  "positive and negative zero positions compare equal in either direction");

        const CurveKey nan(std::numeric_limits<float>::quiet_NaN(), 1.0f);
        ctx.Check(middle.CompareTo(nan) == 1 && nan.CompareTo(middle) == 1 &&
                      nan.CompareTo(CurveKey(std::numeric_limits<float>::quiet_NaN(), 2.0f)) == 1,
                  "XNA CompareTo returns positive when either compared Position is NaN");
    }
}

CNA_DEMOS_CONSOLE(CurveKeyOrdering, {
    .id = "math.curve.key_ordering",
    .title = "Curve-key ordering",
    .description = "Exercises CurveKey::CompareTo across ordinary positions, equal positions with different values, "
                   "infinities, signed zero and NaN.",
    .apis = {"CurveKey::CompareTo", "CurveKey::Position"},
    .tags = {"curve", "keyframe", "ordering", "ieee-754"},
    .restrictions = "CNA-DEMOS-ISSUE-137 records CompareTo returning zero instead of XNA's positive result for NaN.",
    .knownIssue = "CNA-DEMOS-ISSUE-137",
})
