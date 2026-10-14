// SPDX-License-Identifier: MS-PL
// A skinned glTF with a metallic-roughness material: CNA gives it a SkinnedPbrEffect (CNAEXT),
// which takes the AnimationPlayer's skin transforms like XNA's SkinnedEffect.
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/ImageBasedLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedPbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
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

            irradiance_ = std::make_unique<TextureCube>(getGraphicsDeviceProperty(), 1, false, SurfaceFormat::Color);
            blackSpecular_ =
                std::make_unique<TextureCube>(getGraphicsDeviceProperty(), 1, false, SurfaceFormat::Color);
            const Color green = Color::Lime;
            const Color black = Color::Black;
            for (int face = 0; face < 6; ++face)
            {
                irradiance_->SetData(static_cast<CubeMapFace>(face), &green, 1);
                blackSpecular_->SetData(static_cast<CubeMapFace>(face), &black, 1);
            }
            brdfLut_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1, false, SurfaceFormat::Color);
            const Color brdfScaleBias(255, 0, 0, 255);
            brdfLut_->SetData(&brdfScaleBias, 1);
        }

        void Update(GameTime& gameTime) override
        {
            if (iblChecked_)
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

                ImageBasedLightEXT light;
                light.Irradiance = irradiance_.get();
                light.PrefilteredSpecular = blackSpecular_.get();
                light.BrdfLut = brdfLut_.get();
                light.PrefilteredMipCount = 1;
                light.Intensity = 2.0f;
                for (ModelMesh* mesh : column_->getMeshesProperty())
                    for (Effect* effect : mesh->getEffectsProperty())
                    {
                        auto* skinned = dynamic_cast<SkinnedPbrEffect*>(effect);
                        skinned->getDirectionalLight0Property().setEnabledProperty(false);
                        skinned->getDirectionalLight1Property().setEnabledProperty(false);
                        skinned->getDirectionalLight2Property().setEnabledProperty(false);
                        skinned->setAmbientLightColorProperty(Vector3(1.0f, 0.0f, 0.0f));
                        skinned->setEncodeOutputToSrgbEXTProperty(false);
                        skinned->setImageBasedLightEXT(light);
                    }
                auto* first = dynamic_cast<SkinnedPbrEffect*>(
                    column_->getMeshesProperty()[0]->getEffectsProperty()[0]);
                const ImageBasedLightEXT& stored = first->getImageBasedLightEXT();
                Context().Check(stored.IsValidEXT() && stored.Irradiance == irradiance_.get() &&
                                    stored.PrefilteredSpecular == blackSpecular_.get() &&
                                    stored.BrdfLut == brdfLut_.get() && stored.Intensity == 2.0f,
                                "SkinnedPbrEffect retains the complete environment bundle");
                checked_ = true;
            }
            else if (!iblChecked_)
            {
                const float a = MathHelper::ToRadians(50.0f);
                const Vector3 top(-1.8f * std::sin(a), 2.0f + 1.8f * std::cos(a), 0.25f);
                const Vector3 onScreen = device.getViewportProperty().Project(top, projection_, view_,
                                                                              Matrix::getIdentityProperty());
                const Color pixel =
                    CnaDemos::BackBufferPixels(device).At(static_cast<int>(onScreen.X), static_cast<int>(onScreen.Y));
                Context().Print("near the IBL-lit bent top: {}", pixel.ToString());
                Context().Check(pixel.getGProperty() > pixel.getRProperty() + 80 &&
                                    pixel.getGProperty() > pixel.getBProperty() + 80,
                                "the unchanged skinned pose samples green environment irradiance");
                iblChecked_ = true;
            }
            Game::Draw(gameTime);
        }

    private:
        std::optional<Model> column_;
        std::optional<AnimationPlayer> player_;
        Matrix view_;
        Matrix projection_;
        std::unique_ptr<TextureCube> irradiance_;
        std::unique_ptr<TextureCube> blackSpecular_;
        std::unique_ptr<Texture2D> brdfLut_;
        bool checked_ = false;
        bool iblChecked_ = false;
    };
}

CNA_DEMOS_GAME(SkinnedPbrDemo, {
    .id = "graphics.model.skinned_pbr",
    .title = "Skinned PBR model",
    .description = "A skinned glTF column with an orange metallic-roughness material imports with CNA's "
                   "SkinnedPbrEffect; AnimationPlayer supplies its skin transforms, then a complete image-based "
                   "light replaces the default directional lighting without changing the bent pose.",
    .apis = {"SkinnedPbrEffect::SetBoneTransforms", "SkinnedPbrEffect", "AnimationPlayer::GetSkinTransforms",
             "SkinnedPbrEffect::ImageBasedLightEXT", "ImageBasedLightEXT::IsValidEXT",
             "AnimationPlayer::StartClip", "SkinningData", "Model::Tag", "ModelMesh::Draw"},
    .tags = {"model", "skinning", "pbr", "ibl", "gltf", "animation", "cnaext", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .assets = {"models/painted_column.cnb"},
    .restrictions = "Skipped on SOFTWARE and HEADLESS, where CNA draws SkinnedPbrEffect as an unlit "
                    "fallback.",
})
