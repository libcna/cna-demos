// SPDX-License-Identifier: MS-PL
// How CurveTangent (Flat, Linear, Smooth) shapes a curve between its keys.
#include <stdexcept>

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
        ctx.Check(static_cast<int>(CurveTangent::Flat) == 0 &&
                      static_cast<int>(CurveTangent::Linear) == 1 &&
                      static_cast<int>(CurveTangent::Smooth) == 2,
                  "CurveTangent exposes XNA's exact Flat, Linear and Smooth ordinals");

        Curve empty;
        empty.ComputeTangents(CurveTangent::Flat);
        empty.ComputeTangents(CurveTangent::Linear, CurveTangent::Smooth);
        Curve single;
        single.getKeysProperty().Add(CurveKey(2.0f, 7.0f, 4.0f, 5.0f));
        bool singleModesZero = true;
        for (const CurveTangent mode : {CurveTangent::Flat, CurveTangent::Linear, CurveTangent::Smooth})
        {
            single.ComputeTangents(mode);
            const CurveKey& key = single.getKeysProperty().getItemProperty(0);
            singleModesZero = singleModesZero && key.getTangentInProperty() == 0.0f &&
                key.getTangentOutProperty() == 0.0f;
        }
        ctx.Check(empty.getKeysProperty().getCountProperty() == 0 && singleModesZero,
                  "whole-curve tangent computation is a no-op for empty and zeroes every single-key mode");

        const Curve flat = MakeCurve(CurveTangent::Flat);
        const Curve linear = MakeCurve(CurveTangent::Linear);
        const Curve smooth = MakeCurve(CurveTangent::Smooth);

        ctx.Check(flat.getKeysProperty().getItemProperty(0).getTangentInProperty() == 0.0f &&
                      flat.getKeysProperty().getItemProperty(0).getTangentOutProperty() == 0.0f &&
                      flat.getKeysProperty().getItemProperty(1).getTangentInProperty() == 0.0f &&
                      flat.getKeysProperty().getItemProperty(1).getTangentOutProperty() == 0.0f &&
                      flat.getKeysProperty().getItemProperty(2).getTangentInProperty() == 0.0f &&
                      flat.getKeysProperty().getItemProperty(2).getTangentOutProperty() == 0.0f,
                  "Flat assigns zero incoming and outgoing tangents to every key");
        ctx.Check(linear.getKeysProperty().getItemProperty(0).getTangentOutProperty() == 10.0f &&
                      linear.getKeysProperty().getItemProperty(1).getTangentInProperty() == 10.0f &&
                      linear.getKeysProperty().getItemProperty(1).getTangentOutProperty() == 0.0f &&
                      linear.getKeysProperty().getItemProperty(2).getTangentInProperty() == 0.0f,
                  "Linear uses adjacent value differences and zeroes missing endpoint neighbours");
        ctx.CheckNear(smooth.getKeysProperty().getItemProperty(1).getTangentInProperty(),
                      10.0 / 3.0, 1e-6, "Smooth weights the middle incoming tangent by key spacing");
        ctx.CheckNear(smooth.getKeysProperty().getItemProperty(1).getTangentOutProperty(),
                      20.0 / 3.0, 1e-6, "Smooth weights the middle outgoing tangent by key spacing");

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

        const CurveKey firstBeforeInvalid = edited.getKeysProperty().getItemProperty(0);
        bool negativeRejected = false;
        bool pastEndRejected = false;
        try
        {
            edited.ComputeTangent(-1, CurveTangent::Flat);
        }
        catch (const std::out_of_range&)
        {
            negativeRejected = true;
        }
        try
        {
            edited.ComputeTangent(edited.getKeysProperty().getCountProperty(), CurveTangent::Flat);
        }
        catch (const std::out_of_range&)
        {
            pastEndRejected = true;
        }
        ctx.Check(negativeRejected && pastEndRejected &&
                      edited.getKeysProperty().getItemProperty(0) == firstBeforeInvalid,
                  "C++ ComputeTangent rejects both invalid index directions without changing keys");

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
