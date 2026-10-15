// SPDX-License-Identifier: MS-PL
// AnimationPlayer validation and its deliberate borrowed SkinningData/AnimationClip lifetimes.
#include <cmath>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    SkinningData MakeSkinningData()
    {
        SkinningData data;
        data.BoneCount = 2;
        data.SkeletonHierarchy = {-1, 0};
        data.BindPose = {Matrix::CreateTranslation(1, 0, 0),
                         Matrix::CreateTranslation(0, 1, 0)};
        data.InverseBindPose = {Matrix::getIdentityProperty(), Matrix::getIdentityProperty()};
        data.SkeletonRootPrefix = {Matrix::CreateTranslation(3, 0, 0),
                                   Matrix::getIdentityProperty()};
        return data;
    }

    AnimationClip MakeClip()
    {
        AnimationClip clip;
        clip.Duration = TimeSpan::FromSeconds(1);
        BoneTrackEXT root;
        root.BoneIndex = 0;
        root.Keys = {{TimeSpan::Zero, Vector3(1, 0, 0)},
                     {clip.Duration, Vector3(3, 0, 0)}};
        clip.Tracks = {root};
        return clip;
    }

    template<class Mutate>
    bool ConstructorRefuses(Mutate&& mutate)
    {
        SkinningData data = MakeSkinningData();
        std::forward<Mutate>(mutate)(data);
        try
        {
            AnimationPlayer player(data);
            (void)player;
        }
        catch (const System::ArgumentException&)
        {
            return true;
        }
        return false;
    }

    bool Near(float actual, float expected)
    {
        return std::abs(actual - expected) < 1e-4f;
    }

    void AnimationPlayerValidation(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(ConstructorRefuses([](SkinningData& data) {
                      data.InverseBindPose.pop_back();
                  }),
                  "construction refuses required skeleton arrays whose sizes do not match BoneCount");
        ctx.Check(ConstructorRefuses([](SkinningData& data) {
                      data.SkeletonHierarchy[0] = 1;
                  }),
                  "construction refuses a parent that is not earlier than its child");

        SkinningData noPrefix = MakeSkinningData();
        noPrefix.SkeletonRootPrefix.clear();
        AnimationPlayer identityPrefix(noPrefix);
        ctx.Check(Near(identityPrefix.GetWorldTransforms()[0].getTranslationProperty().X, 1) &&
                      Near(identityPrefix.GetWorldTransforms()[1].getTranslationProperty().X, 1) &&
                      Near(identityPrefix.GetWorldTransforms()[1].getTranslationProperty().Y, 1),
                  "an empty optional root-prefix array means identity for the whole skeleton");

        SkinningData data = MakeSkinningData();
        AnimationPlayer player(data);
        ctx.Check(Near(player.GetWorldTransforms()[0].getTranslationProperty().X, 4) &&
                      Near(player.GetWorldTransforms()[1].getTranslationProperty().X, 4) &&
                      Near(player.GetWorldTransforms()[1].getTranslationProperty().Y, 1),
                  "a complete root-prefix array places the root and its child in scene context");

        const bool constructorRejectsPartial = ConstructorRefuses([](SkinningData& malformed) {
            malformed.SkeletonRootPrefix.pop_back();
        });
        data.SkeletonRootPrefix.pop_back();
        bool updateRejectsPartial = false;
        try { player.Update(TimeSpan::Zero, false, false); }
        catch (const System::ArgumentException&) { updateRejectsPartial = true; }
        ctx.Check(constructorRejectsPartial && updateRejectsPartial,
                  "a nonempty root-prefix array must match BoneCount at construction and recomputation");
        data.SkeletonRootPrefix.push_back(Matrix::getIdentityProperty());

        AnimationClip clip = MakeClip();
        player.StartClip(clip);
        player.Update(TimeSpan::FromSeconds(.5), false, false);
        ctx.Check(player.getCurrentClipProperty() == &clip &&
                      Near(player.GetBoneTransforms()[0].getTranslationProperty().X, 2) &&
                      Near(player.GetWorldTransforms()[0].getTranslationProperty().X, 5),
                  "the player borrows the selected clip while applying the retained root prefix");

        data.SkeletonHierarchy[1] = 1;
        bool rejectedBorrowedHierarchy = false;
        try { player.Update(TimeSpan::FromSeconds(.75), false, false); }
        catch (const System::ArgumentException&) { rejectedBorrowedHierarchy = true; }
        ctx.Check(rejectedBorrowedHierarchy,
                  "each update revalidates a borrowed hierarchy changed after construction");
        data.SkeletonHierarchy[1] = 0;
        player.Update(TimeSpan::FromSeconds(.75), false, false);
        ctx.Check(Near(player.GetWorldTransforms()[0].getTranslationProperty().X, 5.5f) &&
                      Near(player.GetWorldTransforms()[1].getTranslationProperty().X, 5.5f) &&
                      Near(player.GetWorldTransforms()[1].getTranslationProperty().Y, 1),
                  "restoring borrowed skeleton data lets the same player recompute a complete pose");

        data.InverseBindPose.pop_back();
        bool rejectedBorrowedSize = false;
        try { player.Update(TimeSpan::FromSeconds(.5), false, false); }
        catch (const System::ArgumentException&) { rejectedBorrowedSize = true; }
        data.InverseBindPose.push_back(Matrix::getIdentityProperty());
        ctx.Check(rejectedBorrowedSize,
                  "each update also revalidates borrowed required-array sizes");

        clip.Tracks[0].Keys.back().Translation.X = 5;
        player.Update(TimeSpan::FromSeconds(.5), false, false);
        const float changed = player.GetWorldTransforms()[0].getTranslationProperty().X;
        clip.Tracks[0].Keys.back().Translation.X = 3;
        player.Update(TimeSpan::FromSeconds(.5), false, false);
        ctx.Check(Near(changed, 6) &&
                      Near(player.GetWorldTransforms()[0].getTranslationProperty().X, 5),
                  "later clip edits are observed through the documented borrowed clip reference");
    }
}

CNA_DEMOS_CONSOLE(AnimationPlayerValidation, {
    .id = "graphics.model.animation_player_validation",
    .title = "Validate borrowed animation data",
    .description = "Exercises AnimationPlayer construction and per-update validation, identity and "
                   "complete root prefixes, recovery after borrowed skeleton repair, live borrowed "
                   "clip edits, and CNA-DEMOS-ISSUE-096's silent partial-prefix fallback.",
    .apis = {"Microsoft::Xna::Framework::Graphics::AnimationPlayer",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::StartClip",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::Update",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::GetBoneTransforms",
             "Microsoft::Xna::Framework::Graphics::AnimationPlayer::GetWorldTransforms",
             "Microsoft::Xna::Framework::Graphics::SkinningData::SkeletonRootPrefix",
             "Microsoft::Xna::Framework::Graphics::SkinningData::SkeletonHierarchy",
             "Microsoft::Xna::Framework::Graphics::SkinningData::InverseBindPose"},
    .tags = {"model", "skinning", "animation", "validation", "hierarchy", "cnaext"},
    .knownIssue = "CNA-DEMOS-ISSUE-096",
})
