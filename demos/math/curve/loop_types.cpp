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

        const std::pair<CurveLoopType, std::string_view> modes[] = {
            {CurveLoopType::Constant, "Constant"}, {CurveLoopType::Cycle, "Cycle"},
            {CurveLoopType::CycleOffset, "CycleOffset"}, {CurveLoopType::Oscillate, "Oscillate"},
            {CurveLoopType::Linear, "Linear"}};

        ctx.Print("PostLoop       Evaluate(1.25)  Evaluate(2.25)");
        for (const auto& [mode, name] : modes)
        {
            ramp.setPostLoopProperty(mode);
            ctx.Print("{:<12} {:15.3f} {:15.3f}", name, ramp.Evaluate(1.25f), ramp.Evaluate(2.25f));
        }

        ramp.setPostLoopProperty(CurveLoopType::Constant);
        ctx.Check(ramp.Evaluate(5.0f) == 1.0f, "Constant holds the last value");
        ramp.setPostLoopProperty(CurveLoopType::Cycle);
        ctx.CheckNear(ramp.Evaluate(1.25f), 0.25, 1e-5, "Cycle repeats the curve");
        ramp.setPostLoopProperty(CurveLoopType::CycleOffset);
        ctx.CheckNear(ramp.Evaluate(1.25f), 1.25, 1e-5, "CycleOffset repeats it, stacked on the previous end value");
        ramp.setPostLoopProperty(CurveLoopType::Oscillate);
        ctx.CheckNear(ramp.Evaluate(1.25f), 0.75, 1e-5, "Oscillate plays it backwards on alternate cycles");
        ramp.setPostLoopProperty(CurveLoopType::Linear);
        ctx.CheckNear(ramp.Evaluate(2.0f), 2.0, 1e-5, "Linear extends the end tangent");

        // PreLoop does the same before the first key.
        ramp.setPreLoopProperty(CurveLoopType::Cycle);
        ctx.CheckNear(ramp.Evaluate(-0.25f), 0.75, 1e-5, "PreLoop Cycle repeats the curve before the start");
        ctx.Check(ramp.getPreLoopProperty() == CurveLoopType::Cycle, "PreLoop reads back");
    }
}

CNA_DEMOS_CONSOLE(CurveLoopTypes, {
    .id = "math.curve.loop_types",
    .title = "Curve loop types",
    .description = "Evaluates a Curve beyond its last key under every CurveLoopType set through PostLoop, and "
                   "before its first key through PreLoop.",
    .apis = {"Curve::PostLoop", "Curve::PreLoop", "CurveLoopType::Constant", "CurveLoopType::Cycle",
             "CurveLoopType::CycleOffset", "CurveLoopType::Oscillate", "CurveLoopType::Linear", "Curve::Keys",
             "Curve::Evaluate"},
    .tags = {"curve", "loop", "animation"},
})
