// SPDX-License-Identifier: MS-PL
// ContentManager loads a relocatable loose bundle into the distinct SkinnedModelEXT carrier.
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SkinnedManifestDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            // Sidecars name siblings of the manifest, NOT paths relative to the content root.
            model_ = getContentProperty().Load<std::shared_ptr<SkinnedModelEXT>>("models/skinned_bundle/patches");
            if (!Context().Check(model_ && model_->BoneCount == 2 && model_->Parts.size() == 1 &&
                                  model_->Clips.contains("Slide"), "the nested bundle loads its skeleton, part and named clip")) return;
            Context().Check(model_->ParentBoneIndices == std::vector<int>{-1, 0} &&
                            model_->BindPoseLocal.size() == 2 && model_->InverseBindPoseGlobal.size() == 2,
                            "the skeleton sidecar preserves the authored parent order and both pose arrays");
            const auto& entry = model_->Parts.front();
            if (!Context().Check(entry.Name == "Patches" && entry.Part && entry.Texture &&
                                  entry.Part->getNumVerticesProperty() == 8 && entry.Part->getPrimitiveCountProperty() == 4 &&
                                  entry.Texture->getWidthProperty() == 2 && entry.Texture->getHeightProperty() == 1,
                                  "relative mesh and texture sidecars produce the authored eight-vertex textured part")) return;
            const auto& clip = model_->Clips.at("Slide");
            Context().Check(clip.Duration == System::TimeSpan::FromSeconds(1) && clip.Tracks.size() == 1 &&
                            clip.Tracks[0].BoneIndex == 1 && clip.Tracks[0].Keys.size() == 2,
                            "the binary clip retains its duration, joint target and two keys");
            std::vector<Matrix> palette;
            model_->ComputeBoneTransformsEXT("Slide", System::TimeSpan::Zero, false, palette);
            Context().Check(palette == std::vector<Matrix>(2, Matrix::getIdentityProperty()),
                            "the loaded bind and inverse-bind arrays cancel at the rest key");
            model_->ComputeBoneTransformsEXT("Slide", System::TimeSpan::FromSeconds(1), false, palette);
            if (!Context().Check(palette.size() == 2, "the loaded clip produces both skin matrices")) return;
            Context().Check(palette[0] == Matrix::getIdentityProperty(), "the untracked root remains in its bind pose");
            Context().CheckNear(palette[1].M41, -.4, .0001, "the child skin matrix moves left by the authored 0.4 units");
            bool refused = false;
            try { (void)getContentProperty().Load<std::shared_ptr<SkinnedModelEXT>>("models/skinned_bundle/truncated"); }
            catch (const Content::ContentLoadException& error)
            {
                refused = true;
                Context().Print("Truncated bundle: {}", error.what());
            }
            Context().Check(refused, "a skeleton header without its declared arrays throws ContentLoadException");
            effect_ = std::make_unique<SkinnedEffect>(getGraphicsDeviceProperty());
            effect_->setWorldProperty(Matrix::getIdentityProperty());
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::getIdentityProperty());
            effect_->setWeightsPerVertexProperty(1);
            effect_->SetBoneTransforms(palette);
            effect_->setAmbientLightColorProperty(Vector3::One);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            if (!effect_) { Game::Draw(time); return; }
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.getSamplerStatesProperty()(0, SamplerState::PointClamp);
            for (const auto& entry : model_->Parts)
            {
                effect_->setTextureProperty(entry.Texture);
                effect_->Apply();
                const auto* part = entry.Part;
                device.SetVertexBuffer(part->getVertexBufferProperty());
                device.SetIndexBuffer(part->getIndexBufferProperty());
                device.DrawIndexedPrimitives(PrimitiveType::TriangleList, part->getVertexOffsetProperty(), 0,
                                             part->getNumVerticesProperty(), part->getStartIndexProperty(),
                                             part->getPrimitiveCountProperty());
            }
            if (!checked_)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const auto at = [&](float x, float y) {
                    return pixels.At(static_cast<int>((x + 1) * .5f * pixels.Width()),
                                     static_cast<int>((1 - y) * .5f * pixels.Height()));
                };
                Context().Check(CnaDemos::Near(at(-.4f, -.4f), Color::Red, 2), "loaded joint-zero vertices stay red in the lower left");
                Context().Check(CnaDemos::Near(at(0, .4f), Color::Lime, 2), "loaded joint-one vertices move the green patch left");
                Context().Check(CnaDemos::Near(at(.4f, .4f), Color::DarkSlateGray, 2), "the green patch's original position is vacated");
                checked_ = true;
            }
            Game::Draw(time);
        }

    private:
        std::shared_ptr<SkinnedModelEXT> model_;
        std::unique_ptr<SkinnedEffect> effect_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(SkinnedManifestDemo, {
    .id = "graphics.model.skinned_manifest",
    .title = "Load a skinned manifest bundle",
    .description = "Loads a loose SkinnedModelEXT bundle with relative skeleton, geometry, texture and clip sidecars, "
                   "rejects truncated skeleton data, then verifies the loaded animation with rendered pixels.",
    .apis = {"ContentManager::Load", "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::ComputeBoneTransformsEXT", "SkinnedEffect::SetBoneTransforms"},
    .tags = {"model", "skinning", "content", "manifest", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/skinned_bundle/patches.skinnedmodel.json", "models/skinned_bundle/rig.skeleton.bin",
               "models/skinned_bundle/patches.vertices.bin", "models/skinned_bundle/patches.indices.bin",
               "models/skinned_bundle/patches.png", "models/skinned_bundle/slide.clip.bin",
               "models/skinned_bundle/truncated.skinnedmodel.json", "models/skinned_bundle/truncated.skeleton.bin"},
})
