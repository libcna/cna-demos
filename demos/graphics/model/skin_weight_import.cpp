// SPDX-License-Identifier: MS-PL
// glTF skin weights: quantized decoding, renormalization, extra-set truncation and zero sums.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GltfImportReportEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    bool Near(float actual, float expected)
    {
        return std::fabs(actual - expected) <= 1e-5f;
    }

    class SkinWeightImportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            model_.emplace(getContentProperty().Load<Model>("models/loose_skin_weight_forms.gltf"));
            auto& ctx = Context();
            if (!ctx.Check(model_->getMeshesProperty().getCountProperty() == 4,
                           "four authored weight forms import as four model meshes")) return;
            const auto* skinning = dynamic_cast<const SkinningData*>(model_->getTagProperty());
            if (!ctx.Check(skinning && skinning->BoneCount == 5,
                           "the shared imported skin retains all five joints")) return;

            for (int i = 0; i < 4; ++i)
            {
                auto* mesh = model_->getMeshesProperty()[i];
                if (!ctx.Check(mesh && mesh->getMeshPartsProperty().getCountProperty() == 1,
                               "each weight form keeps one mesh part")) return;
                auto* part = mesh->getMeshPartsProperty()[0];
                auto* effect = dynamic_cast<SkinnedEffect*>(part->getEffectProperty());
                if (!ctx.Check(effect && part->getVertexBufferProperty(),
                               "each skinned unlit material imports through SkinnedEffect")) return;
                effects_[static_cast<std::size_t>(i)] = effect;
                part->getVertexBufferProperty()->GetData(
                    &vertices_[static_cast<std::size_t>(i)], 0, 1);
            }

            const auto& quantized = vertices_[0];
            ctx.Check(quantized.BlendIndices == std::array<std::uint8_t, 4>{3, 1, 2, 0} &&
                          Near(quantized.BlendWeight.X, 128.0f / 255.0f) &&
                          Near(quantized.BlendWeight.Y, 64.0f / 255.0f) &&
                          Near(quantized.BlendWeight.Z, 42.0f / 255.0f) &&
                          Near(quantized.BlendWeight.W, 21.0f / 255.0f),
                      "normalized UBYTE weights and USHORT joint indices retain all four channels");
            ctx.Check(vertices_[1].BlendIndices == std::array<std::uint8_t, 4>{0, 1, 0, 0} &&
                          vertices_[1].BlendWeight == Vector4(.5f, .5f, 0, 0),
                      "a malformed 0.6 weight sum is renormalized to an equal two-joint blend");
            ctx.Check(vertices_[2].BlendIndices == std::array<std::uint8_t, 4>{0, 1, 0, 0} &&
                          vertices_[2].BlendWeight == Vector4(.5f, .5f, 0, 0),
                      "dropping JOINTS_1/WEIGHTS_1 renormalizes the retained first influence set");
            ctx.Check(vertices_[3].BlendWeight == Vector4::Zero,
                      "a zero-sum weight set remains unweighted instead of selecting an arbitrary joint");

            std::size_t renormalizedEntries = 0;
            std::size_t renormalizedVertices = 0;
            std::size_t droppedSets = 0;
            std::size_t zeroVertices = 0;
            double renormalizedWorst = 0;
            double droppedWorst = 0;
            bool renormalizedNamed = false;
            bool droppedNamed = false;
            bool zeroNamed = false;
            for (const auto& diagnostic : model_->getGltfImportReportEXTProperty().Diagnostics)
            {
                if (diagnostic.Code == "weights-renormalised")
                {
                    ++renormalizedEntries;
                    renormalizedVertices += diagnostic.Count;
                    renormalizedWorst = std::max(renormalizedWorst, diagnostic.WorstMagnitude);
                    renormalizedNamed |= diagnostic.Subject.find("Renormalized") != std::string::npos;
                }
                else if (diagnostic.Code == "influence-sets-dropped")
                {
                    droppedSets += diagnostic.Count;
                    droppedWorst = std::max(droppedWorst, diagnostic.WorstMagnitude);
                    droppedNamed |= diagnostic.Subject.find("DroppedExtra") != std::string::npos;
                }
                else if (diagnostic.Code == "zero-weight-vertices")
                {
                    zeroVertices += diagnostic.Count;
                    zeroNamed |= diagnostic.Subject.find("ZeroWeights") != std::string::npos;
                }
            }
            ctx.Check(renormalizedEntries == 2 && renormalizedVertices == 8 &&
                          std::fabs(renormalizedWorst - .4) < 1e-5 && renormalizedNamed,
                      "the report counts both four-vertex renormalizations and their worst deviation");
            ctx.Check(droppedSets == 1 && std::fabs(droppedWorst - .4) < 1e-5 && droppedNamed,
                      "the report measures the discarded second influence set's forty-percent share");
            ctx.Check(zeroVertices == 4 && zeroNamed,
                      "the report names all four zero-weight vertices as dropped data");

            palette_ = {Matrix::getIdentityProperty(), Matrix::CreateTranslation(.12f, 0, 0),
                        Matrix::CreateTranslation(.24f, 0, 0), Matrix::CreateTranslation(.36f, 0, 0),
                        Matrix::CreateTranslation(.72f, 0, 0)};
            for (SkinnedEffect* effect : effects_)
            {
                effect->SetBoneTransforms(palette_);
                effect->setWorldProperty(Matrix::getIdentityProperty());
                effect->setViewProperty(Matrix::getIdentityProperty());
                effect->setProjectionProperty(Matrix::getIdentityProperty());
                effect->setAmbientLightColorProperty(Vector3::One);
                effect->getDirectionalLight0Property().setEnabledProperty(false);
                effect->getDirectionalLight1Property().setEnabledProperty(false);
                effect->getDirectionalLight2Property().setEnabledProperty(false);
                effect->setFogEnabledProperty(false);
            }
            ready_ = true;
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            if (ready_)
            {
                device.setBlendStateProperty(BlendState::Opaque);
                device.setDepthStencilStateProperty(DepthStencilState::None);
                device.setRasterizerStateProperty(RasterizerState::CullNone);
                for (int i = 0; i < 3; ++i)
                    model_->getMeshesProperty()[i]->Draw();

                if (FramesDrawn() == 0)
                {
                    const CnaDemos::BackBufferPixels pixels(device);
                    const auto at = [&](float x) {
                        return pixels.At(static_cast<int>((x + 1) * .5f * pixels.Width()),
                                         pixels.Height() / 2);
                    };
                    const float quantizedShift =
                        (128.0f * .36f + 64.0f * .12f + 42.0f * .24f) / 255.0f;
                    ctxPixel(at(-.75f + quantizedShift), Color::Red,
                             "the renderer consumes all four quantized imported influences");
                    ctxPixel(at(-.10f + .06f), Color::Lime,
                             "the renderer consumes the renormalized two-joint weights");
                    ctxPixel(at(.55f + .06f), Color::Blue,
                             "the truncated influence set draws from its renormalized retained weights");
                    Context().Check(CnaDemos::Near(at(-.75f), Color::DarkSlateGray, 2) &&
                                        CnaDemos::Near(at(-.10f), Color::DarkSlateGray, 2) &&
                                        CnaDemos::Near(at(.55f), Color::DarkSlateGray, 2),
                                    "all three original bind-pose centres are vacated by the imported weights");
                }
            }
            DemoGame::Draw(time);
        }

    private:
        void ctxPixel(const Color& actual, const Color& expected, const char* message)
        {
            Context().Check(CnaDemos::Near(actual, expected, 2), message);
        }

        std::optional<Model> model_;
        std::array<SkinnedEffect*, 4> effects_{};
        std::array<VertexPositionNormalTextureSkinned, 4> vertices_{};
        std::vector<Matrix> palette_;
        bool ready_ = false;
    };
}

CNA_DEMOS_GAME(SkinWeightImportDemo, {
    .id = "graphics.model.skin_weight_import",
    .title = "Import glTF skin weight forms",
    .description = "Reads back normalized integer and repaired/truncated glTF influences, verifies structured "
                   "diagnostics, then draws the imported weights through a controlled five-joint palette.",
    .apis = {"ContentManager::Load", "Model::GltfImportReportEXT", "GltfImportDiagnosticEXT",
             "VertexBuffer::GetData", "SkinnedEffect::SetBoneTransforms", "ModelMesh::Draw"},
    .tags = {"model", "gltf", "skinning", "weights", "diagnostics", "animation", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_skin_weight_forms.gltf"},
})
