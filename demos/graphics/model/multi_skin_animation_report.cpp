// SPDX-License-Identifier: MS-PL
// One glTF animation split across two independent skin palettes and its aggregate import report.
#include <cmath>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/GltfImportReportEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    class MultiSkinAnimationReportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            Model model = getContentProperty().Load<Model>("models/loose_multi_skin");
            const auto& skins = model.getSkinsEXTProperty();
            if (!ctx.Check(skins.size() == 2,
                           "the model exposes both independently animated skins")) return;

            const ModelSkinEXT* left = nullptr;
            const ModelSkinEXT* right = nullptr;
            for (const ModelSkinEXT& skin : skins)
            {
                if (skin.Name == "LeftRig") left = &skin;
                if (skin.Name == "RightRig") right = &skin;
            }
            if (!ctx.Check(left && right && left->Data && right->Data,
                           "the named skin mappings retain both SkinningData objects")) return;

            const auto leftClip = left->Data->AnimationClips.find("SplitMotion");
            const auto rightClip = right->Data->AnimationClips.find("SplitMotion");
            if (!ctx.Check(leftClip != left->Data->AnimationClips.end() &&
                               rightClip != right->Data->AnimationClips.end(),
                           "the one source animation is retained in both palette clip collections")) return;
            ctx.Check(left->Data->AnimationClips.size() == 1 &&
                          right->Data->AnimationClips.size() == 1 &&
                          leftClip->second.TargetSpace == ClipTargetSpaceEXT::JointPalette &&
                          rightClip->second.TargetSpace == ClipTargetSpaceEXT::JointPalette &&
                          leftClip->second.Duration == TimeSpan::FromSeconds(1) &&
                          rightClip->second.Duration == TimeSpan::FromSeconds(1) &&
                          leftClip->second.Tracks.size() == 1 &&
                          rightClip->second.Tracks.size() == 1 &&
                          leftClip->second.Tracks[0].BoneIndex == 0 &&
                          rightClip->second.Tracks[0].BoneIndex == 0,
                      "each palette retains exactly its own one-joint one-second track");

            AnimationPlayer leftPlayer(*left->Data);
            AnimationPlayer rightPlayer(*right->Data);
            leftPlayer.StartClip(leftClip->second);
            rightPlayer.StartClip(rightClip->second);
            leftPlayer.Update(TimeSpan::FromSeconds(1), false, false);
            rightPlayer.Update(TimeSpan::FromSeconds(1), false, false);
            ctx.Check(std::abs(leftPlayer.GetBoneTransforms()[0].getTranslationProperty().Y - .2f) < 1e-4f &&
                          std::abs(rightPlayer.GetBoneTransforms()[0].getTranslationProperty().Y + .3f) < 1e-4f,
                      "the two retained clips reproduce both authored final joint translations");

            const GltfImportReportEXT& report = model.getGltfImportReportEXTProperty();
            ctx.Check(report.SkinCount == 2 && report.AnimationCount == 1 && report.ClipCount == 2,
                      "the report distinguishes one source animation from two retained palette clips");

            std::size_t unavailableEntries = 0;
            std::size_t unavailableOccurrences = 0;
            bool rigidDrop = false;
            for (const GltfImportDiagnosticEXT& diagnostic : report.Diagnostics)
            {
                if (diagnostic.Code == "animation-targets-unavailable")
                {
                    ++unavailableEntries;
                    unavailableOccurrences += diagnostic.Count;
                }
                if (diagnostic.Code == "rigid-animation-dropped-on-skinned-model")
                    rigidDrop = true;
            }
            ctx.Check(!rigidDrop,
                      "no scene-node animation is reported lost when every track belongs to a retained palette");
            ctx.Check(unavailableEntries == 0,
                      "channels retained by a different skin palette are not reported as unavailable loss");
            ctx.Check(unavailableEntries == 2 && unavailableOccurrences == 2 &&
                          report.getWarningCountProperty() == 2 &&
                          report.getDroppedFeatureCountProperty() == 2 &&
                          report.getApproximationCountProperty() == 0 && report.AnythingLost(),
                      "the current report exposes one false unavailable-channel warning per palette");
        }
    };
}

CNA_DEMOS_GAME(MultiSkinAnimationReportDemo, {
    .id = "graphics.model.multi_skin_animation_report",
    .title = "Aggregate animation reporting across skins",
    .description = "Imports one glTF animation whose two channels belong to different independent skins, "
                   "plays both retained palette clips, and exposes CNA-DEMOS-ISSUE-095's false per-palette "
                   "unavailable-target loss in the model-level report.",
    .apis = {"Model::SkinsEXT", "ModelSkinEXT", "SkinningData::AnimationClips",
             "AnimationPlayer::StartClip", "AnimationPlayer::Update",
             "AnimationPlayer::GetBoneTransforms", "Model::GltfImportReportEXT",
             "GltfImportReportEXT::Diagnostics", "GltfImportReportEXT::AnythingLost"},
    .tags = {"model", "gltf", "skinning", "animation", "diagnostics", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .assets = {"models/loose_multi_skin.gltf"},
    .knownIssue = "CNA-DEMOS-ISSUE-095",
})
