// SPDX-License-Identifier: MS-PL
// CurveContinuity::Step: a curve that holds each key's value instead of blending.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveContinuity.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveTangent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void CurveStepContinuity(CnaDemos::DemoContext& ctx)
    {
        // A sprite-sheet frame index over time (seconds): frames must not blend.
        Curve frame;
        frame.getKeysProperty().Add(CurveKey(0.0f, 0.0f, 0.0f, 0.0f, CurveContinuity::Step));
        frame.getKeysProperty().Add(CurveKey(1.0f, 1.0f, 0.0f, 0.0f, CurveContinuity::Step));
        frame.getKeysProperty().Add(CurveKey(2.0f, 2.0f, 0.0f, 0.0f, CurveContinuity::Step));
        frame.getKeysProperty().Add(CurveKey(3.0f, 3.0f, 0.0f, 0.0f, CurveContinuity::Step));

        for (const float t : {0.0f, 0.5f, 1.0f, 1.5f, 2.5f, 3.0f})
        {
            ctx.Print("t = {:.1f} s -> frame {}", t, frame.Evaluate(t));
        }

        // XNA documents Step as: "Specifying a position between the two points returns this point."
        ctx.Check(frame.Evaluate(0.5f) == 0.0f, "between keys 0 and 1, the first key's value is held");
        ctx.Check(frame.Evaluate(1.5f) == 1.0f, "between keys 1 and 2, the value of key 1 is held");
        ctx.Check(frame.Evaluate(2.5f) == 2.0f, "between keys 2 and 3, the value of key 2 is held");

        // Continuity belongs to each key and can be changed later.
        frame.getKeysProperty().getItemProperty(0).setContinuityProperty(CurveContinuity::Smooth);
        frame.ComputeTangents(CurveTangent::Linear);
        ctx.CheckNear(frame.Evaluate(0.5f), 0.5, 1e-5, "after switching key 0 to Smooth, the first segment blends");
    }
}

CNA_DEMOS_CONSOLE(CurveStepContinuity, {
    .id = "math.curve.step_continuity",
    .title = "Stepped curve keys",
    .description = "Uses CurveContinuity::Step so a Curve holds each key's value until the next key (a frame index "
                   "over time), then switches a key back to Smooth.",
    .apis = {"CurveKey::CurveKey", "CurveKey::Continuity", "CurveContinuity::Step", "CurveContinuity::Smooth",
             "CurveKeyCollection::Item", "CurveKeyCollection::Add"},
    .tags = {"curve", "step", "keyframe"},
    .knownIssue = "CNA-DEMOS-ISSUE-002",
})
