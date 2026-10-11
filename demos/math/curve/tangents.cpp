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

        // Recompute just one key, with different incoming and outgoing modes.
        Curve edited;
        edited.getKeysProperty().Add(CurveKey(0.0f, 0.0f));
        edited.getKeysProperty().Add(CurveKey(1.0f, 10.0f));
        edited.ComputeTangent(0, CurveTangent::Flat, CurveTangent::Linear);
        const auto& first = edited.getKeysProperty().getItemProperty(0);
        const auto& last = edited.getKeysProperty().getItemProperty(1);
        ctx.Check(first.getTangentInProperty() == 0.0f && first.getTangentOutProperty() == 10.0f,
                  "one key can have a flat incoming tangent and a linear outgoing tangent");
        ctx.Check(last.getTangentInProperty() == 0.0f, "ComputeTangent leaves the other key unchanged");
        ctx.CheckNear(edited.Evaluate(0.5f), 6.25, 1e-4, "a faster departure shifts the midpoint above five");
        edited.ComputeTangent(0, CurveTangent::Flat);
        ctx.CheckNear(edited.Evaluate(0.5f), 5.0, 1e-4, "resetting that key restores symmetric easing");
        edited.ComputeTangents(CurveTangent::Linear, CurveTangent::Flat);
        ctx.CheckNear(edited.Evaluate(0.5f), 3.75, 1e-4, "asymmetric whole-curve tangents shift the midpoint below five");
        ctx.Print("Edited midpoint: {} (incoming Linear, outgoing Flat)", edited.Evaluate(0.5f));

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
                   "prints how each shapes the curve, then edits one key and computes asymmetric tangents.",
    .apis = {"Curve::ComputeTangents", "Curve::ComputeTangent", "CurveKey::TangentIn", "CurveKey::TangentOut", "CurveTangent::Flat", "CurveTangent::Linear", "CurveTangent::Smooth",
             "Curve::Keys", "Curve::Evaluate"},
    .tags = {"curve", "tangent", "easing"},
})
