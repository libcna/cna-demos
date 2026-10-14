// SPDX-License-Identifier: MS-PL
// glTF KHR_materials_variants: one model, several paint jobs, switched at run time (CNAEXT).
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/MorphTargetEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class MaterialVariantsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            // The variants deliberately need different complete part states: Ocean is PBR (a
            // 60-byte vertex layout), while default red and textured Gold are unlit (24 bytes).
            // Every layout owns a morph Tag, and Gold owns point-clamp texture sampler metadata.
            // Imported at run time: CNA's compiled model format cannot carry material variants.
            toy_.emplace(getContentProperty().Load<Model>("models/loose_paint_toy"));
            const auto& names = toy_->getMaterialVariantNamesEXTProperty();
            std::string list;
            for (const std::string& name : names)
                list += " " + name;
            ctx.Print("variants:{}", list);
            ctx.Check(names.size() == 2 && names[0] == "Ocean" && names[1] == "Gold",
                      "the model lists the file's variants in order");
            ctx.Check(toy_->getMaterialVariantEXTProperty() == -1, "a loaded model shows its default materials (-1)");

            if (!ctx.Check(toy_->getMeshesProperty().getCountProperty() == 2 &&
                           toy_->getMeshesProperty()[0]->getMeshPartsProperty().getCountProperty() == 1,
                           "the imported car keeps one body part and one wheel mesh"))
                return;
            bodyMesh_ = toy_->getMeshesProperty()[0];
            bodyPart_ = bodyMesh_->getMeshPartsProperty()[0];
            defaultEffect_ = bodyPart_->getEffectProperty();
            defaultVertices_ = bodyPart_->getVertexBufferProperty();
            defaultMorph_ = dynamic_cast<MorphTargetDataEXT*>(bodyPart_->getTagProperty());
            auto* defaultBasic = dynamic_cast<BasicEffect*>(defaultEffect_);
            ctx.Check(defaultBasic && !defaultBasic->getLightingEnabledProperty() &&
                          !defaultBasic->getTextureEnabledProperty(),
                      "the default red paint is an untextured unlit BasicEffect");
            if (!ctx.Check(defaultVertices_ &&
                               defaultVertices_->getVertexDeclarationProperty().getVertexStrideProperty() == 24 &&
                               defaultMorph_ && defaultMorph_->Stride == 24 &&
                               defaultMorph_->PositionDeltas.size() == 1,
                           "the default part and its morph Tag use the 24-byte colour layout"))
                return;
            ctx.Check(IsOnlyEffect(defaultEffect_),
                      "the body Effects collection initially contains only the default effect");

            toy_->setMaterialVariantEXTProperty(0);
            auto* oceanEffect = dynamic_cast<PbrEffect*>(bodyPart_->getEffectProperty());
            auto* oceanVertices = bodyPart_->getVertexBufferProperty();
            auto* oceanMorph = dynamic_cast<MorphTargetDataEXT*>(bodyPart_->getTagProperty());
            if (!ctx.Check(oceanEffect && oceanEffect != defaultEffect_ && oceanVertices != defaultVertices_ &&
                               oceanVertices->getVertexDeclarationProperty().getVertexStrideProperty() == 60,
                           "Ocean swaps the effect and vertex buffer to the 60-byte coloured PBR layout"))
                return;
            if (!ctx.Check(oceanMorph && oceanMorph != defaultMorph_ && oceanMorph->Stride == 60,
                           "Ocean swaps Tag to the morph carrier matching its PBR layout"))
                return;
            ctx.Check(IsOnlyEffect(oceanEffect),
                      "the body Effects collection replaces the default effect with Ocean");

            toy_->setMaterialVariantEXTProperty(1);
            goldEffect_ = dynamic_cast<BasicEffect*>(bodyPart_->getEffectProperty());
            auto* goldVertices = bodyPart_->getVertexBufferProperty();
            goldMorph_ = dynamic_cast<MorphTargetDataEXT*>(bodyPart_->getTagProperty());
            const auto& goldSampler = bodyPart_->getSamplerStatesEXTProperty()[0];
            if (!ctx.Check(goldEffect_ && goldEffect_ != defaultEffect_ && goldEffect_->getTextureEnabledProperty() &&
                               goldEffect_->getTextureProperty() &&
                               goldEffect_->getTextureProperty()->getWidthProperty() == 2 &&
                               goldEffect_->getTextureProperty()->getHeightProperty() == 1,
                           "Gold swaps to an owned two-texel base-colour texture"))
                return;
            if (!ctx.Check(goldVertices && goldVertices != defaultVertices_ &&
                               goldVertices->getVertexDeclarationProperty().getVertexStrideProperty() == 24 &&
                               goldMorph_ && goldMorph_ != defaultMorph_ && goldMorph_ != oceanMorph &&
                               goldMorph_->Stride == 24,
                           "Gold swaps the vertex buffer and Tag back to its own 24-byte layout state"))
                return;
            ctx.Check(goldSampler.getFilterProperty() == TextureFilter::Point &&
                          goldSampler.getAddressUProperty() == TextureAddressMode::Clamp &&
                          goldSampler.getAddressVProperty() == TextureAddressMode::Clamp,
                      "Gold swaps slot zero to the imported nearest-clamp sampler");
            ctx.Check(IsOnlyEffect(goldEffect_),
                      "the body Effects collection replaces Ocean with Gold");

            SetMorphWeightsEXT(*bodyPart_, {0.5f});
            ctx.Check(goldMorph_->Weights.size() == 1 && goldMorph_->Weights[0] == 0.5f,
                      "morphing through the selected part updates Gold's layout-specific Tag");
            toy_->setMaterialVariantEXTProperty(-1);
            ctx.Check(bodyPart_->getEffectProperty() == defaultEffect_ &&
                          bodyPart_->getVertexBufferProperty() == defaultVertices_ &&
                          bodyPart_->getTagProperty() == defaultMorph_ &&
                          defaultMorph_->Weights.size() == 1 && defaultMorph_->Weights[0] == 0.0f,
                      "restoring -1 atomically restores the exact default effect, buffer, and Tag");
            const auto& restoredSampler = bodyPart_->getSamplerStatesEXTProperty()[0];
            ctx.Check(restoredSampler.getFilterProperty() == TextureFilter::Linear &&
                          restoredSampler.getAddressUProperty() == TextureAddressMode::Wrap &&
                          restoredSampler.getAddressVProperty() == TextureAddressMode::Wrap &&
                          IsOnlyEffect(defaultEffect_),
                      "restoring -1 also restores the default sampler and Effects collection");
            toy_->setMaterialVariantEXTProperty(1);
            ctx.Check(bodyPart_->getTagProperty() == goldMorph_ && goldMorph_->Weights[0] == 0.5f,
                      "reselecting Gold restores its independently retained morph state");
            SetMorphWeightsEXT(*bodyPart_, {0.0f});
            toy_->setMaterialVariantEXTProperty(-1);
            ready_ = true;

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 2.0f, 4.5f), Vector3(0.0f, 0.4f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f,
                100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            // A showroom: default, Ocean, Gold, one second each.
            const int second = static_cast<int>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            if (!Context().Automated())
                toy_->setMaterialVariantEXTProperty(second % 3 - 1);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            if (!ready_)
            {
                device.Clear(Color(30, 32, 40));
                DemoGame::Draw(gameTime);
                return;
            }
            if (FramesDrawn() == 0)
            {
                // The body's top in the middle of the screen, a wheel near the bottom left.
                const Color red = Paint(-1);
                const Color blue = Paint(0);
                const Color wrappedGold = Paint(1, &SamplerState::PointWrap);
                const Color gold = Paint(1);
                Context().Print("body: default {}, Ocean {}, Gold wrap {}, Gold clamp {}",
                                red.ToString(), blue.ToString(), wrappedGold.ToString(), gold.ToString());
                Context().Check(red.getRProperty() > 180 && red.getBProperty() < 60, "the default body is red");
                Context().Check(blue.getBProperty() > blue.getGProperty() + 40 &&
                                    blue.getBProperty() > blue.getRProperty() + 80,
                                "Ocean's lit PBR result remains distinctly blue");
                Context().Check(wrappedGold.getRProperty() < 60 && wrappedGold.getGProperty() > 180 &&
                                    wrappedGold.getBProperty() > 180,
                                "forcing wrap on Gold samples its first cyan texel");
                Context().Check(gold.getRProperty() > 200 && gold.getGProperty() > 150 && gold.getBProperty() < 80,
                                "Gold's selected clamp sampler reaches its second gold texel");
                toy_->setMaterialVariantEXTProperty(1);
                Render();
                const Color wheel = CnaDemos::BackBufferPixels(device).At(WheelX, WheelY);
                Context().Check(wheel.getRProperty() < 40 && wheel.getGProperty() < 40 && wheel.getBProperty() < 40,
                                "the wheels, which have no mapping, keep their own black material");
                toy_->setMaterialVariantEXTProperty(-1);
            }
            Render();
            DemoGame::Draw(gameTime);
        }

    private:
        void Render()
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(30, 32, 40));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            toy_->Draw(Matrix::getIdentityProperty(), view_, projection_);
        }

        bool IsOnlyEffect(const Effect* effect) const
        {
            return bodyMesh_->getEffectsProperty().getCountProperty() == 1 &&
                   bodyMesh_->getEffectsProperty()[0] == effect;
        }

        // Selects a variant, applies its imported base-colour sampler unless overridden, draws,
        // and reads the body's colour.
        Color Paint(int variant, const SamplerState* samplerOverride = nullptr)
        {
            toy_->setMaterialVariantEXTProperty(variant);
            getGraphicsDeviceProperty().getSamplerStatesProperty()(
                0, samplerOverride ? *samplerOverride : bodyPart_->getSamplerStatesEXTProperty()[0]);
            Render();
            return CnaDemos::BackBufferPixels(getGraphicsDeviceProperty()).At(BodyX, BodyY);
        }

        static constexpr int BodyX = 400;
        static constexpr int BodyY = 200;
        static constexpr int WheelX = 295;
        static constexpr int WheelY = 300;

        std::optional<Model> toy_;
        ModelMesh* bodyMesh_ = nullptr;
        ModelMeshPart* bodyPart_ = nullptr;
        Effect* defaultEffect_ = nullptr;
        VertexBuffer* defaultVertices_ = nullptr;
        MorphTargetDataEXT* defaultMorph_ = nullptr;
        BasicEffect* goldEffect_ = nullptr;
        MorphTargetDataEXT* goldMorph_ = nullptr;
        bool ready_ = false;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(MaterialVariantsDemo, {
    .id = "graphics.model.material_variants",
    .title = "Material variants (glTF)",
    .description = "A toy car whose glTF declares KHR_materials_variants: selection swaps the body's complete "
                   "effect, vertex-layout, texture, sampler and morph-Tag state; -1 restores the exact default, "
                   "while the wheels without a mapping keep their material.",
    .apis = {"Model::MaterialVariantNamesEXT", "Model::MaterialVariantEXT", "ModelMesh::Effects",
             "ModelMeshPart::VertexBuffer", "ModelMeshPart::SamplerStatesEXT", "ModelMeshPart::Tag",
             "MorphTargetDataEXT", "SetMorphWeightsEXT", "Model::Draw", "ContentManager::Load"},
    .tags = {"model", "gltf", "material", "variant", "texture", "sampler", "morph", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_paint_toy.gltf"},
})
