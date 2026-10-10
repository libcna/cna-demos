// SPDX-License-Identifier: MS-PL
// A Curve authored as CNJ (CNA's JSON content format), compiled to CNB and loaded as a Curve.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveLoopType.hpp"

using namespace Microsoft::Xna::Framework;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void CurveAsset(CnaDemos::DemoContext& ctx)
    {
        // content/curves/bounce.cnj describes one bounce: keys at 0, 0.5 and 1 with tangents,
        // repeated by postLoop Cycle. XNA authored curves like this as XML in a content project;
        // CNA's pipeline compiles the JSON into curves/bounce.cnb. A Curve needs no graphics
        // device, so a ContentManager without services is enough.
        ContentManager content(nullptr, "assets");
        const Curve bounce = content.Load<Curve>("curves/bounce");

        ctx.Print("{} keys, PreLoop {}, PostLoop {}", bounce.getKeysProperty().getCountProperty(),
                  static_cast<int>(bounce.getPreLoopProperty()), static_cast<int>(bounce.getPostLoopProperty()));
        ctx.Check(bounce.getKeysProperty().getCountProperty() == 3, "the curve has the three authored keys");
        ctx.Check(bounce.getPostLoopProperty() == CurveLoopType::Cycle, "PostLoop is Cycle, as authored");

        for (const float t : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f})
            ctx.Print("Evaluate({:.2f}) = {:.3f}", t, bounce.Evaluate(t));
        ctx.CheckNear(bounce.Evaluate(0.5f), 1.0, 1e-5, "the top of the bounce is at 0.5");
        ctx.CheckNear(bounce.Evaluate(1.25f), bounce.Evaluate(0.25f), 1e-5, "after the last key the bounce cycles");
        ctx.Check(bounce.Evaluate(-1.0f) == 0.0f, "before the first key PreLoop Constant holds the first value");
    }
}

CNA_DEMOS_CONSOLE(CurveAsset, {
    .id = "content.cnj.curve",
    .title = "Curve from CNJ content",
    .description = "Loads a Curve authored in CNA's JSON content format (.cnj) and compiled by the content pipeline, "
                   "then evaluates it inside, after (PostLoop Cycle) and before (PreLoop Constant) its keys.",
    .apis = {"ContentManager::Load", "Curve::Evaluate", "Curve::Keys", "Curve::PreLoop", "Curve::PostLoop"},
    .tags = {"content", "cnj", "json", "curve", "animation"},
    .assets = {"curves/bounce.cnb"},
})
