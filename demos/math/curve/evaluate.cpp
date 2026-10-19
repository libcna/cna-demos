// SPDX-License-Identifier: MS-PL
// Curve defaults and evaluation at ordinary, duplicate and nonfinite positions.
#include <cmath>
#include <limits>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveTangent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void CurveEvaluate(CnaDemos::DemoContext& ctx)
    {
        const Curve empty;
        ctx.Check(empty.getIsConstantProperty() && empty.getKeysProperty().getCountProperty() == 0 &&
                      empty.getPreLoopProperty() == CurveLoopType::Constant &&
                      empty.getPostLoopProperty() == CurveLoopType::Constant &&
                      empty.Evaluate(-5.0f) == 0.0f && empty.Evaluate(5.0f) == 0.0f &&
                      empty.Evaluate(std::numeric_limits<float>::quiet_NaN()) == 0.0f,
                  "a new empty Curve has constant loops, is constant and evaluates to zero");

        // Brightness over a 10-second day: dark, bright at noon, dark again.
        Curve brightness;
        brightness.getKeysProperty().Add(CurveKey(0.0f, 0.0f));
        brightness.getKeysProperty().Add(CurveKey(5.0f, 1.0f));
        brightness.getKeysProperty().Add(CurveKey(10.0f, 0.0f));

        // Keys need tangents; ComputeTangents derives them from neighbouring keys.
        brightness.ComputeTangents(CurveTangent::Linear);

        for (const float time : {0.0f, 2.5f, 5.0f, 7.5f, 10.0f})
        {
            ctx.Print("Evaluate({:4.1f}) = {:.3f}", time, brightness.Evaluate(time));
        }
        ctx.Check(brightness.Evaluate(5.0f) == 1.0f, "a curve passes exactly through its keys");
        ctx.CheckNear(brightness.Evaluate(2.5f), 0.5, 1e-5, "with linear tangents, halfway between keys is the average");
        ctx.Check(brightness.getKeysProperty().getCountProperty() == 3, "the curve holds three keys");
        ctx.Check(brightness.Evaluate(-std::numeric_limits<float>::infinity()) == 0.0f &&
                      brightness.Evaluate(std::numeric_limits<float>::infinity()) == 0.0f,
                  "default Constant loops hold the first and last values at infinite positions");
        ctx.Check(std::isnan(brightness.Evaluate(std::numeric_limits<float>::quiet_NaN())),
                  "a NaN position propagates through a nonconstant Curve evaluation");

        Curve duplicatePositions;
        duplicatePositions.getKeysProperty().Add(CurveKey(0.0f, 0.0f));
        duplicatePositions.getKeysProperty().Add(CurveKey(0.0f, 10.0f));
        duplicatePositions.getKeysProperty().Add(CurveKey(1.0f, 20.0f));
        duplicatePositions.ComputeTangents(CurveTangent::Linear);
        ctx.Check(duplicatePositions.Evaluate(0.0f) == 0.0f &&
                      duplicatePositions.Evaluate(std::numeric_limits<float>::denorm_min()) == 10.0f,
                  "an exact duplicate position selects the first key while the next float enters the later segment");
    }
}

CNA_DEMOS_CONSOLE(CurveEvaluate, {
    .id = "math.curve.evaluate",
    .title = "Evaluate a curve",
    .description = "Checks Curve defaults and evaluates ordinary linear keys, constant-loop infinities, NaN and "
                   "an exact duplicate-position boundary.",
    .apis = {"Curve::Curve", "Curve::IsConstant", "Curve::Keys", "Curve::PreLoop", "Curve::PostLoop",
             "CurveKeyCollection::Add", "CurveKey::CurveKey", "Curve::ComputeTangents", "Curve::Evaluate",
             "CurveKeyCollection::Count"},
    .tags = {"curve", "animation", "keyframe", "boundaries", "ieee-754"},
})
