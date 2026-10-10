// SPDX-License-Identifier: MS-PL
// A Curve: keys at positions, evaluated anywhere in between.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveTangent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void CurveEvaluate(CnaDemos::DemoContext& ctx)
    {
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
    }
}

CNA_DEMOS_CONSOLE(CurveEvaluate, {
    .id = "math.curve.evaluate",
    .title = "Evaluate a curve",
    .description = "Adds CurveKeys to a Curve, computes their tangents and evaluates the curve between the keys "
                   "with Curve::Evaluate.",
    .apis = {"Curve::Curve", "Curve::Keys", "CurveKeyCollection::Add", "CurveKey::CurveKey", "Curve::ComputeTangents",
             "Curve::Evaluate", "CurveKeyCollection::Count"},
    .tags = {"curve", "animation", "keyframe"},
})
