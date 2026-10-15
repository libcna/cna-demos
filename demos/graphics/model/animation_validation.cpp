// SPDX-License-Identifier: MS-PL
// Loose glTF animation validation at the public ContentManager boundary.
#include <cmath>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    class AnimationValidationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            const auto refusal = [&](std::string_view asset) {
                try
                {
                    (void)getContentProperty().Load<Model>(std::string(asset));
                    return std::string{};
                }
                catch (const Content::ContentLoadException& error)
                {
                    return std::string(error.what());
                }
            };

            const std::string badOrder =
                refusal("models/loose_animation_bad_input_order");
            ctx.Check(badOrder.find("not ascending") != std::string::npos &&
                          badOrder.find("strictly increasing") != std::string::npos &&
                          badOrder.find("Backwards") != std::string::npos,
                      "a decreasing sampler input is refused with the clip, channel and ordering rule");

            const std::string shortOutput =
                refusal("models/loose_animation_bad_output_count");
            ctx.Check(!shortOutput.empty(),
                      "structural glTF validation refuses an animation output shorter than its input");

            const Model valid = getContentProperty().Load<Model>("models/turntable");
            ctx.Check(valid.getBonesProperty()["Platform"] != nullptr,
                      "a valid model still loads through the same manager after both refusals");

            std::optional<Model> invalidInterpolation;
            try
            {
                invalidInterpolation.emplace(
                    getContentProperty().Load<Model>("models/loose_animation_bad_interpolation"));
            }
            catch (const Content::ContentLoadException&)
            {
                // The required behavior: glTF permits exactly LINEAR, STEP and CUBICSPLINE.
            }
            ctx.Check(!invalidInterpolation.has_value(),
                      "an interpolation mode outside LINEAR, STEP and CUBICSPLINE is refused");

            bool fallbackShape = true;
            if (invalidInterpolation)
            {
                const auto* animations = dynamic_cast<const ModelAnimationsEXT*>(
                    invalidInterpolation->getTagProperty());
                fallbackShape = animations != nullptr &&
                    animations->Clips.contains("UnknownInterpolation");
                if (fallbackShape)
                {
                    const AnimationClip& clip =
                        animations->Clips.at("UnknownInterpolation");
                    ApplyClipToBonesEXT(*invalidInterpolation, clip,
                                        TimeSpan::FromSeconds(.5));
                    const float x = invalidInterpolation->getBonesProperty()["Animated"]
                                        ->getTransformProperty().getTranslationProperty().X;
                    fallbackShape = clip.Duration == TimeSpan::FromSeconds(1) &&
                        clip.Tracks.size() == 1 && clip.Tracks[0].Keys.size() == 2 &&
                        std::abs(x - .2f) < 1e-4f &&
                        invalidInterpolation->getGltfImportReportEXTProperty()
                                .getWarningCountProperty() == 0;
                }
            }
            ctx.Check(fallbackShape,
                      "if accepted, the invalid mode is silently represented as an ordinary LINEAR clip");
        }
    };
}

CNA_DEMOS_GAME(AnimationValidationDemo, {
    .id = "graphics.model.animation_validation",
    .title = "glTF animation validation",
    .description = "Loads original malformed loose glTF animation samplers through ContentManager: "
                   "decreasing input and a short output are refused without poisoning later loads, while an "
                   "unknown interpolation mode exposes CNA-DEMOS-ISSUE-093's silent LINEAR fallback.",
    .apis = {"ContentManager::Load", "ContentLoadException", "ModelAnimationsEXT",
             "ApplyClipToBonesEXT", "Model::GltfImportReportEXT", "Model::Bones"},
    .tags = {"model", "gltf", "animation", "validation", "errors", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .assets = {"models/turntable.cnb", "models/loose_animation_bad_input_order.gltf",
               "models/loose_animation_bad_output_count.gltf",
               "models/loose_animation_bad_interpolation.gltf"},
    .knownIssue = "CNA-DEMOS-ISSUE-093",
})
