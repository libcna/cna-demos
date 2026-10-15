// SPDX-License-Identifier: MS-PL
// Sample a hierarchical skin palette without a graphics device or an imported model.
#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    bool Near(const Vector3& actual, const Vector3& expected)
    {
        return Vector3::Distance(actual, expected) < .0001f;
    }

    void PaletteSampling(CnaDemos::DemoContext& ctx)
    {
        SkinnedModelEXT model;
        model.BoneCount = 3;
        model.ParentBoneIndices = {-1, 0, 1}; // root -> untracked child -> animated tip
        model.BindPoseLocal = {Matrix::CreateTranslation(2, 0, 0), Matrix::CreateTranslation(0, 1, 0),
                               Matrix::CreateTranslation(1, 0, 0)};
        model.InverseBindPoseGlobal = {Matrix::CreateTranslation(-2, 0, 0), Matrix::CreateTranslation(-2, -1, 0),
                                       Matrix::CreateTranslation(-3, -1, 0)};
        AnimationClipEXT clip;
        clip.Duration = TimeSpan::FromSeconds(2);
        BoneTrackEXT root;
        root.BoneIndex = 0;
        // Exact positive-Z half turn: zero W avoids roundoff choosing the opposite shortest arc.
        root.Keys = {{TimeSpan::Zero, Vector3(2, 0, 0)},
                     {clip.Duration, Vector3(4, 0, 0), Quaternion(0, 0, 1, 0), Vector3(3, 3, 1)}};
        BoneTrackEXT tip;
        tip.BoneIndex = 2;
        tip.Keys = {{TimeSpan::Zero, Vector3(1, 0, 0)}, {clip.Duration, Vector3(1, 2, 0)}};
        clip.Tracks = {root, tip}; // bone 1 deliberately has no track: its local bind pose survives
        model.Clips.emplace("TurnAndReach", clip);

        std::vector<Matrix> palette;
        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::Zero, false, palette);
        const std::array<Vector3, 3> bindJoints{Vector3(2, 0, 0), Vector3(2, 1, 0), Vector3(3, 1, 0)};
        if (!ctx.Check(palette.size() == 3, "the palette has one matrix per bone")) return;
        bool restIsIdentity = true;
        for (const auto& bone : palette) restIsIdentity = restIsIdentity && bone == Matrix::getIdentityProperty();
        ctx.Check(restIsIdentity, "inverse global bind transforms cancel the complete rest hierarchy");

        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::FromSeconds(1), false, palette);
        const auto middle = palette;
        const auto rootPosition = Vector3::Transform(bindJoints[0], palette[0]);
        const auto childPosition = Vector3::Transform(bindJoints[1], palette[1]);
        const auto tipPosition = Vector3::Transform(bindJoints[2], palette[2]);
        ctx.Print("At 1 s: root ({:.2f}, {:.2f}), child ({:.2f}, {:.2f}), tip ({:.2f}, {:.2f})",
                  rootPosition.X, rootPosition.Y, childPosition.X, childPosition.Y, tipPosition.X, tipPosition.Y);
        ctx.Check(Near(rootPosition, Vector3(3, 0, 0)), "root translation interpolates from 2 to 4");
        ctx.Check(Near(Vector3::Transform(Vector3(3, 0, 0), palette[0]), Vector3(3, 2, 0)),
                  "root scale and quaternion interpolation produce a doubled, quarter-turned offset");
        ctx.Check(Near(childPosition, Vector3(1, 0, 0)),
                  "the untracked child's bind offset inherits the animated parent's scale and rotation");
        ctx.Check(Near(tipPosition, Vector3(-1, 2, 0)),
                  "the tip's local animation composes through both parents before inverse-bind skinning");

        // The Model-facing player uses SkinningData, not SkinnedModelEXT. Reuse the authored
        // skeleton to distinguish its three coordinate spaces without another rendering scene.
        SkinningData skeleton;
        skeleton.BoneCount = model.BoneCount;
        skeleton.SkeletonHierarchy = model.ParentBoneIndices;
        skeleton.BindPose = model.BindPoseLocal;
        skeleton.InverseBindPose = model.InverseBindPoseGlobal;
        AnimationPlayer player(skeleton); // skeleton and clip both outlive this borrowed player
        bool playerRestIsIdentity = player.GetSkinTransforms().size() == 3;
        for (const auto& bone : player.GetSkinTransforms())
            playerRestIsIdentity = playerRestIsIdentity && bone == Matrix::getIdentityProperty();
        if (!ctx.Check(player.GetBoneTransforms() == skeleton.BindPose && playerRestIsIdentity,
                       "before a clip the Model player retains local bind offsets but publishes an identity skin palette")) return;
        player.StartClip(clip);
        player.Update(TimeSpan::FromSeconds(1), false, false);
        const auto& local = player.GetBoneTransforms();
        const auto& world = player.GetWorldTransforms();
        const auto& skin = player.GetSkinTransforms();
        ctx.Check(Near(Vector3::Transform(Vector3::Zero, local[0]), Vector3(3, 0, 0)) &&
                  Near(Vector3::Transform(Vector3::Zero, local[1]), Vector3(0, 1, 0)) &&
                  Near(Vector3::Transform(Vector3::Zero, local[2]), Vector3(1, 1, 0)),
                  "local transforms contain each tracked pose or untracked bind offset without ancestor motion");
        ctx.Check(Near(Vector3::Transform(Vector3::Zero, world[0]), Vector3(3, 0, 0)) &&
                  Near(Vector3::Transform(Vector3::Zero, world[1]), Vector3(1, 0, 0)) &&
                  Near(Vector3::Transform(Vector3::Zero, world[2]), Vector3(-1, 2, 0)),
                  "world transforms locate joint origins after the parent's interpolated scale and quarter turn");
        ctx.Check(Near(Vector3::Transform(Vector3::Zero, skin[0]), Vector3(3, -4, 0)) &&
                  Near(Vector3::Transform(Vector3::Zero, skin[1]), Vector3(3, -4, 0)) &&
                  Near(Vector3::Transform(Vector3::Zero, skin[2]), Vector3(1, -4, 0)) &&
                  Near(Vector3::Transform(bindJoints[2], skin[2]), Vector3(-1, 2, 0)),
                  "skin transforms move bind-model points rather than joint-local origins after inverse-bind correction");
        bool samePlayers = skin.size() == middle.size();
        for (std::size_t i = 0; i < skin.size() && samePlayers; ++i)
            for (const auto& point : {bindJoints[i], bindJoints[i] + Vector3::UnitX})
                samePlayers = samePlayers && Near(Vector3::Transform(point, skin[i]), Vector3::Transform(point, middle[i]));
        ctx.Check(samePlayers, "both public evaluators agree on transformed bind points and offsets for this authored hierarchy");

        // Compare transformed rest-space points, not implementation-generated expected matrices.
        const auto samePose = [&](const std::vector<Matrix>& expected) {
            if (palette.size() != expected.size()) return false;
            for (std::size_t i = 0; i < palette.size(); ++i)
                for (const auto& point : {bindJoints[i], bindJoints[i] + Vector3::UnitX})
                    if (!Near(Vector3::Transform(point, palette[i]), Vector3::Transform(point, expected[i]))) return false;
            return true;
        };

        // A hand-authored clip can contain sparse or irrelevant tracks. Empty tracks and indices
        // outside this palette are omissions, not permission to read outside the bone arrays.
        BoneTrackEXT empty;
        empty.BoneIndex = 1;
        BoneTrackEXT negative;
        negative.BoneIndex = -2;
        negative.Keys = {{TimeSpan::Zero, Vector3(100, 100, 100)}};
        BoneTrackEXT pastEnd;
        pastEnd.BoneIndex = model.BoneCount;
        pastEnd.Keys = {{TimeSpan::Zero, Vector3(200, 200, 200)}};
        auto& tracks = model.Clips.at("TurnAndReach").Tracks;
        const std::size_t authoredTrackCount = tracks.size();
        tracks.insert(tracks.end(), {empty, negative, pastEnd});
        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::FromSeconds(1), false, palette);
        ctx.Check(samePose(middle),
                  "empty, negative-index and past-end tracks are skipped without changing valid motion");
        tracks.resize(authoredTrackCount);

        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::FromSeconds(3), true, palette);
        ctx.Check(samePose(middle), "looping at 3 seconds wraps to the 1-second pose");
        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::FromSeconds(-1), true, palette);
        ctx.Check(samePose(middle), "negative looping time wraps into the same clip interval");
        model.ComputeBoneTransformsEXT("TurnAndReach", clip.Duration, true, palette);
        ctx.Check(Near(Vector3::Transform(bindJoints[2], palette[2]), bindJoints[2]),
                  "looping at the exact duration returns to the rest pose");
        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::FromSeconds(3), false, palette);
        ctx.Check(Near(Vector3::Transform(bindJoints[1], palette[1]), Vector3(4, -3, 0)) &&
                  Near(Vector3::Transform(bindJoints[2], palette[2]), Vector3(1, -9, 0)),
                  "non-looping time past the end holds the final scaled half-turn and tip translation");
        model.ComputeBoneTransformsEXT("TurnAndReach", TimeSpan::FromSeconds(-1), false, palette);
        ctx.Check(Near(Vector3::Transform(bindJoints[2], palette[2]), bindJoints[2]),
                  "negative non-looping time clamps to the start");

        const auto savedPalette = palette;
        const auto refusedWithoutOutputChange = [&](std::string_view name) {
            bool refused = false;
            try { model.ComputeBoneTransformsEXT(std::string(name), TimeSpan::Zero, false, palette); }
            catch (const System::ArgumentException&) { refused = true; }
            return refused && palette == savedPalette;
        };
        ctx.Check(refusedWithoutOutputChange("Missing"), "an unknown clip is refused without overwriting the caller's palette");
        model.ParentBoneIndices[1] = 1;
        ctx.Check(refusedWithoutOutputChange("TurnAndReach"), "a self-parent is refused before publishing a palette");
        model.ParentBoneIndices[1] = 0;
        model.InverseBindPoseGlobal.pop_back();
        ctx.Check(refusedWithoutOutputChange("TurnAndReach"), "a truncated skeleton array is refused before publishing a palette");
    }
}

CNA_DEMOS_CONSOLE(PaletteSampling, {
    .id = "graphics.model.palette_sampling",
    .title = "Sample a hierarchical skin palette",
    .description = "Samples translation, scale and rotation through a three-bone hierarchy, preserves untracked and safely skipped track state, "
                   "distinguishes AnimationPlayer local/world/skin coordinates, compares looped and clamped time, and rejects invalid clip/skeleton requests.",
    .apis = {"Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::ComputeBoneTransformsEXT",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::ParentBoneIndices",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::BindPoseLocal",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::InverseBindPoseGlobal",
             "Microsoft::Xna::Framework::Graphics::AnimationClipEXT",
             "Microsoft::Xna::Framework::Graphics::BoneTrackEXT",
             "Microsoft::Xna::Framework::Graphics::KeyframeEXT",
             "Microsoft::Xna::Framework::Graphics::SkinningData",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::StartClip",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::Update",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::GetBoneTransforms",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::GetWorldTransforms",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::GetSkinTransforms"},
    .tags = {"model", "skinning", "animation", "hierarchy", "cnaext"},
})
