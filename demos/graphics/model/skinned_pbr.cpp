// SPDX-License-Identifier: MS-PL
// A skinned glTF with a metallic-roughness material: CNA gives it a SkinnedPbrEffect (CNAEXT),
// which takes the AnimationPlayer's skin transforms like XNA's SkinnedEffect.
#include <cmath>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedPbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SkinnedPbrDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            // CNA documents PbrEffect and SkinnedPbrEffect as an unlit fallback on these renderers,
            // with no capability query for it (CNA-DEMOS-ISSUE-006).
            const std::string_view renderer = getGraphicsDeviceProperty().GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                ctx.Skip(std::string(renderer) + " draws SkinnedPbrEffect as an unlit fallback");

            // content/models/painted_column.gltf: the bending strip's rig and clip on a square column
            // with an orange metallic-roughness material (tools/gltf_models.py).
            column_.emplace(getContentProperty().Load<Model>("models/painted_column"));
            int pbr = 0, parts = 0;
            for (ModelMesh* mesh : column_->getMeshesProperty())
                for (Effect* effect : mesh->getEffectsProperty())
                {
                    ++parts;
                    pbr += dynamic_cast<SkinnedPbrEffect*>(effect) != nullptr ? 1 : 0;
                }
            ctx.Check(parts > 0 && pbr == parts, "skinned + metallic-roughness imports as SkinnedPbrEffect");

            const auto* skinning = dynamic_cast<const SkinningData*>(column_->getTagProperty());
            ctx.Check(skinning != nullptr && skinning->BoneCount == 2, "Model.Tag holds SkinningData for two joints");
            if (skinning == nullptr || pbr != parts)
                return;
            player_.emplace(*skinning);
            player_->StartClip(skinning->AnimationClips.at("Bend"));
            player_->Update(System::TimeSpan::FromSeconds(0.5), false);  // the +50 degree key

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 2.2f, 8.0f), Vector3(0.0f, 2.2f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            if (checked_)
                player_->Update(gameTime.getElapsedGameTimeProperty(), true, true);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(24, 26, 32));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            for (ModelMesh* mesh : column_->getMeshesProperty())
            {
                for (Effect* effect : mesh->getEffectsProperty())
                {
                    // The same call as SkinnedEffect's: one skin transform per joint, every frame.
                    auto* skinned = dynamic_cast<SkinnedPbrEffect*>(effect);
                    skinned->SetBoneTransforms(player_->GetSkinTransforms());
                    skinned->setViewProperty(view_);
                    skinned->setProjectionProperty(projection_);
                }
                mesh->Draw();
            }

            if (!checked_)
            {
                // The column's top swings around the joint at (0, 2, 0) by 50 degrees; its front
                // face there is orange paint lit by the default lights: much more red than blue.
                const float a = MathHelper::ToRadians(50.0f);
                const Vector3 top(-1.8f * std::sin(a), 2.0f + 1.8f * std::cos(a), 0.25f);
                const Vector3 onScreen = device.getViewportProperty().Project(top, projection_, view_,
                                                                              Matrix::getIdentityProperty());
                const Color pixel =
                    CnaDemos::BackBufferPixels(device).At(static_cast<int>(onScreen.X), static_cast<int>(onScreen.Y));
                Context().Print("near the bent top: {}", pixel.ToString());
                Context().Check(pixel.getRProperty() > pixel.getBProperty() + 40,
                                "the column is drawn bent, orange where the clip puts its top");
                checked_ = true;
            }
            Game::Draw(gameTime);
        }

    private:
        std::optional<Model> column_;
        std::optional<AnimationPlayer> player_;
        Matrix view_;
        Matrix projection_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(SkinnedPbrDemo, {
    .id = "graphics.model.skinned_pbr",
    .title = "Skinned PBR model",
    .description = "A skinned glTF column with an orange metallic-roughness material imports with CNA's "
                   "SkinnedPbrEffect; AnimationPlayer's skin transforms go to SkinnedPbrEffect::SetBoneTransforms "
                   "as they would to XNA's SkinnedEffect.",
    .apis = {"SkinnedPbrEffect::SetBoneTransforms", "SkinnedPbrEffect", "AnimationPlayer::GetSkinTransforms",
             "AnimationPlayer::StartClip", "SkinningData", "Model::Tag", "ModelMesh::Draw"},
    .tags = {"model", "skinning", "pbr", "gltf", "animation", "cnaext", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .assets = {"models/painted_column.cnb"},
    .restrictions = "Skipped on SOFTWARE and HEADLESS, where CNA draws SkinnedPbrEffect as an unlit "
                    "fallback.",
})
