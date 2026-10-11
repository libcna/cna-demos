// SPDX-License-Identifier: MS-PL
// How CurveTangent (Flat, Linear, Smooth) shapes a curve between its keys.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveTangent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    Curve MakeCurve(CurveTangent tangents)
    {
        Curve curve;
        curve.getKeysProperty().Add(CurveKey(0.0f, 0.0f));
        curve.getKeysProperty().Add(CurveKey(1.0f, 10.0f));
        curve.getKeysProperty().Add(CurveKey(3.0f, 10.0f));
        curve.ComputeTangents(tangents);
        return curve;
    }

    void CurveTangents(CnaDemos::DemoContext& ctx)
    {
        const Curve flat = MakeCurve(CurveTangent::Flat);
        const Curve linear = MakeCurve(CurveTangent::Linear);
        const Curve smooth = MakeCurve(CurveTangent::Smooth);

        ctx.Print("position  Flat     Linear   Smooth");
        for (const float p : {0.25f, 0.5f, 0.75f, 1.5f, 2.0f})
        {
            ctx.Print("{:8.2f} {:7.3f} {:8.3f} {:8.3f}", p, flat.Evaluate(p), linear.Evaluate(p), smooth.Evaluate(p));
        }

        // Linear: straight segments. Flat: zero slope at every key (ease in and out).
        ctx.CheckNear(linear.Evaluate(0.25f), 2.5, 1e-4, "Linear moves at constant speed");
        ctx.Check(flat.Evaluate(0.25f) < linear.Evaluate(0.25f), "Flat starts slower than Linear");
        ctx.CheckNear(flat.Evaluate(0.5f), 5.0, 1e-4, "Flat and Linear meet halfway");
        // Smooth uses the neighbours' values: on the plateau after a rise it overshoots slightly.
        ctx.Check(smooth.Evaluate(1.5f) != linear.Evaluate(1.5f), "Smooth bends the segment after the rise");
    }
}

CNA_DEMOS_CONSOLE(CurveTangents, {
    .id = "math.curve.tangents",
    .title = "Curve tangent modes",
    .description = "Builds the same keys with Flat, Linear and Smooth tangents via Curve::ComputeTangents and "
                   "prints how each shapes the curve.",
    .apis = {"Curve::ComputeTangents", "CurveTangent::Flat", "CurveTangent::Linear", "CurveTangent::Smooth",
             "Curve::Keys", "Curve::Evaluate"},
    .tags = {"curve", "tangent", "easing"},
})
