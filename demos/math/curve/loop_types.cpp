// SPDX-License-Identifier: MS-PL
// What a Curve returns outside its keys: CurveLoopType for PreLoop and PostLoop.
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveLoopType.hpp"
#include "Microsoft/Xna/Framework/CurveTangent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void CurveLoopTypes(CnaDemos::DemoContext& ctx)
    {
        // A ramp from 0 to 1 over positions 0..1.
        Curve ramp;
        ramp.getKeysProperty().Add(CurveKey(0.0f, 0.0f));
        ramp.getKeysProperty().Add(CurveKey(1.0f, 1.0f));
        ramp.ComputeTangents(CurveTangent::Linear);

        // ComputeTangents leaves the first incoming and last outgoing tangent at zero.
        // Set explicit endpoint slopes for extrapolation; distinct first/last outgoing
        // tangents expose CNA-DEMOS-ISSUE-036 instead of accidentally concealing it.
        ramp.getKeysProperty().getItemProperty(0).setTangentInProperty(2.0f);
        ramp.getKeysProperty().getItemProperty(1).setTangentOutProperty(3.0f);

        const std::pair<CurveLoopType, std::string_view> modes[] = {
            {CurveLoopType::Constant, "Constant"}, {CurveLoopType::Cycle, "Cycle"},
            {CurveLoopType::CycleOffset, "CycleOffset"}, {CurveLoopType::Oscillate, "Oscillate"},
            {CurveLoopType::Linear, "Linear"}};

        ctx.Print("PostLoop       Evaluate(1.25)  Evaluate(2.25)");
        bool postLoopReadback = true;
        for (const auto& [mode, name] : modes)
        {
            ramp.setPostLoopProperty(mode);
            postLoopReadback = postLoopReadback && ramp.getPostLoopProperty() == mode;
            ctx.Print("{:<12} {:15.3f} {:15.3f}", name, ramp.Evaluate(1.25f), ramp.Evaluate(2.25f));
        }
        ctx.Check(static_cast<int>(CurveLoopType::Constant) == 0 &&
                      static_cast<int>(CurveLoopType::Cycle) == 1 &&
                      static_cast<int>(CurveLoopType::CycleOffset) == 2 &&
                      static_cast<int>(CurveLoopType::Oscillate) == 3 &&
                      static_cast<int>(CurveLoopType::Linear) == 4 && postLoopReadback,
                  "all five CurveLoopType ordinals round-trip through PostLoop");

        ramp.setPostLoopProperty(CurveLoopType::Constant);
        ctx.Check(ramp.Evaluate(5.0f) == 1.0f, "Constant holds the last value");
        ramp.setPostLoopProperty(CurveLoopType::Cycle);
        ctx.CheckNear(ramp.Evaluate(1.25f), 0.25, 1e-5, "Cycle repeats the curve");
        ramp.setPostLoopProperty(CurveLoopType::CycleOffset);
        ctx.CheckNear(ramp.Evaluate(1.25f), 1.25, 1e-5, "CycleOffset repeats it, stacked on the previous end value");
        ramp.setPostLoopProperty(CurveLoopType::Oscillate);
        ctx.CheckNear(ramp.Evaluate(1.25f), 0.75, 1e-5, "Oscillate plays it backwards on alternate cycles");
        ramp.setPostLoopProperty(CurveLoopType::Linear);
        ctx.Print("Linear after the end: actual {}, expected 4 (last value 1 + last tangent 3)", ramp.Evaluate(2.0f));
        ctx.CheckNear(ramp.Evaluate(2.0f), 4.0, 1e-5, "Linear extends the last key's outgoing tangent");

        // Negative times also exercise alternating cycles and exact wrap boundaries.
        ramp.setPreLoopProperty(CurveLoopType::Constant);
        ctx.CheckNear(ramp.Evaluate(-2.0f), 0.0, 1e-5, "PreLoop Constant holds the first value");
        ramp.setPreLoopProperty(CurveLoopType::Linear);
        ctx.CheckNear(ramp.Evaluate(-0.25f), -0.5, 1e-5, "PreLoop Linear uses the first incoming tangent");
        ramp.setPreLoopProperty(CurveLoopType::CycleOffset);
        ctx.CheckNear(ramp.Evaluate(-0.25f), -0.25, 1e-5, "negative CycleOffset subtracts one value span");
        ctx.CheckNear(ramp.Evaluate(-1.0f), -1.0, 1e-5, "CycleOffset is continuous at a negative wrap boundary");
        ctx.CheckNear(ramp.Evaluate(-1.25f), -1.25, 1e-5, "CycleOffset accounts for several negative cycles");
        ramp.setPreLoopProperty(CurveLoopType::Oscillate);
        ctx.CheckNear(ramp.Evaluate(-0.25f), 0.25, 1e-5, "negative Oscillate mirrors the first cycle");
        ctx.CheckNear(ramp.Evaluate(-1.0f), 1.0, 1e-5, "Oscillate reaches the far endpoint at minus one");
        ctx.CheckNear(ramp.Evaluate(-1.25f), 0.75, 1e-5, "Oscillate reverses again in the next negative cycle");
        ctx.CheckNear(ramp.Evaluate(-2.0f), 0.0, 1e-5, "Oscillate returns to the start at minus two");
        ramp.setPreLoopProperty(CurveLoopType::Cycle);
        ctx.CheckNear(ramp.Evaluate(-0.25f), 0.75, 1e-5, "PreLoop Cycle repeats the curve before the start");
        ctx.Check(ramp.getPreLoopProperty() == CurveLoopType::Cycle, "PreLoop reads back");
    }
}

CNA_DEMOS_CONSOLE(CurveLoopTypes, {
    .id = "math.curve.loop_types",
    .title = "Curve loop types",
    .description = "Evaluates a Curve beyond its last key under every CurveLoopType set through PostLoop, and "
                   "before its first key, including negative wrap boundaries and explicit endpoint tangents.",
    .apis = {"Curve::PostLoop", "Curve::PreLoop", "CurveLoopType::Constant", "CurveLoopType::Cycle",
             "CurveLoopType::CycleOffset", "CurveLoopType::Oscillate", "CurveLoopType::Linear", "Curve::Keys",
             "Curve::Evaluate", "CurveKey::TangentIn", "CurveKey::TangentOut"},
    .tags = {"curve", "loop", "animation"},
})
