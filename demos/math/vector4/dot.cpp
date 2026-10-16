// SPDX-License-Identifier: MS-PL
// Vector4 dot products, including wide-product cancellation before the final float result.
#include <cmath>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using Microsoft::Xna::Framework::Vector4;

namespace
{
    void Vector4Dot(CnaDemos::DemoContext& ctx)
    {
        const Vector4 a(1.0f, -2.0f, 3.0f, -4.0f);
        const Vector4 b(5.0f, 6.0f, -7.0f, -8.0f);
        float output = 0.0f;
        Vector4::Dot(a, b, output);
        ctx.Check(Vector4::Dot(a, b) == 4.0f && output == 4.0f,
                  "ordinary value and output Dot forms combine all four signed products");

        Vector4::Dot(Vector4::UnitX, Vector4::UnitW, output);
        ctx.Check(Vector4::Dot(Vector4::UnitX, Vector4::UnitW) == 0.0f && output == 0.0f,
                  "different unit axes are orthogonal in both Dot forms");

        const Vector4 point(2.0f, 3.0f, 4.0f, 1.0f);
        const Vector4 homogeneousWeight(0.0f, 0.0f, 0.0f, 7.0f);
        Vector4::Dot(point, homogeneousWeight, output);
        ctx.Check(Vector4::Dot(point, homogeneousWeight) == 7.0f && output == 7.0f,
                  "the W product participates in a homogeneous Vector4 Dot");

        const Vector4 hugeA(1e20f, 1e20f, 1.0f, 2.0f);
        const Vector4 hugeB(1e20f, -1e20f, 1.0f, 3.0f);
        Vector4::Dot(hugeA, hugeB, output);
        ctx.Check(Vector4::Dot(hugeA, hugeB) == 7.0f && output == 7.0f,
                  "XNA cancels opposite huge products before adding the finite Z and W terms");

        const Vector4 infinity(std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f);
        Vector4::Dot(infinity, Vector4::Zero, output);
        ctx.Check(std::isnan(Vector4::Dot(infinity, Vector4::Zero)) && std::isnan(output),
                  "infinity times zero propagates NaN through both Dot forms");
    }
}

CNA_DEMOS_CONSOLE(Vector4Dot, {
    .id = "math.vector4.dot",
    .title = "Vector4 dot product",
    .description = "Computes ordinary, orthogonal and homogeneous Vector4 dot products and exposes wide-product "
                   "cancellation before the final float result.",
    .apis = {"Vector4::Dot", "Vector4::UnitX", "Vector4::UnitW", "Vector4::Zero"},
    .tags = {"vector", "dot-product", "homogeneous", "precision"},
    .restrictions = "CNA-DEMOS-ISSUE-123 records premature Vector4 product overflow.",
    .knownIssue = "CNA-DEMOS-ISSUE-123",
})
