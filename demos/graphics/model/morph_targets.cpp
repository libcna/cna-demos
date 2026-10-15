// SPDX-License-Identifier: MS-PL
// CNAEXT glTF morph targets: blend shapes on a model part, weighted by hand or by the file's animation.
#include <algorithm>
#include <cmath>
#include <cstring>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/MorphTargetEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class MorphTargetsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            flag_.emplace(getContentProperty().Load<Model>("models/morph_flag"));
            // The part's Tag carries its morph targets: the base vertices, a set of per-vertex
            // deltas per target, the current weights and the file's "weights" animation track.
            ModelMesh* mesh = flag_->getMeshesProperty()[0];
            part_ = (*mesh->getMeshPartsProperty().begin());
            morph_ = dynamic_cast<MorphTargetDataEXT*>(part_->getTagProperty());
            ctx.Check(morph_ != nullptr, "ModelMeshPart.Tag holds MorphTargetDataEXT");
            if (morph_ == nullptr)
            {
                return;
            }
            ctx.Print("{} morph targets over {} vertices; weight track: {} keys", morph_->PositionDeltas.size(),
                      morph_->PositionDeltas.empty() ? 0 : morph_->PositionDeltas[0].size(),
                      morph_->WeightTrack.Keys.size());
            ctx.Check(morph_->PositionDeltas.size() == 2 && morph_->Weights.size() == 2,
                      "the flag has its two targets, Stretch and Shear");
            ctx.Check(morph_->WeightTrack.Keys.size() == 4, "the file's Morph animation became a weight track");

            // Blending is a pure function of the base pose, the deltas and the weights.
            const std::vector<std::uint8_t> rest = BlendMorphTargetsEXT(*morph_, {0.0f, 0.0f});
            const std::vector<std::uint8_t> stretched = BlendMorphTargetsEXT(*morph_, {1.0f, 0.0f});
            ctx.Check(rest == morph_->BaseVertexBytes, "all weights zero gives the base pose");
            // The last vertex is the top-right corner: Stretch lifts it by 0.8.
            const std::size_t last = morph_->BaseVertexBytes.size() - static_cast<std::size_t>(morph_->Stride);
            float baseY = 0.0f;
            float stretchedY = 0.0f;
            std::memcpy(&baseY, rest.data() + last + 4, sizeof(float));
            std::memcpy(&stretchedY, stretched.data() + last + 4, sizeof(float));
            ctx.Print("top-right corner: y {:.2f} at rest, {:.2f} stretched", baseY, stretchedY);
            ctx.Check(std::abs(stretchedY - baseY - 0.8f) < 1e-4f, "weight 1 on Stretch adds its full delta");

            std::vector<Vector3> stretchedPositions;
            for (std::size_t offset = 0; offset < stretched.size(); offset += static_cast<std::size_t>(morph_->Stride))
            {
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                std::memcpy(&x, stretched.data() + offset, sizeof(float));
                std::memcpy(&y, stretched.data() + offset + 4, sizeof(float));
                std::memcpy(&z, stretched.data() + offset + 8, sizeof(float));
                stretchedPositions.emplace_back(x, y, z);
            }
            const Vector3 stretchedCorner = stretchedPositions.back();
            const BoundingSphere importedMeshBounds = mesh->getBoundingSphereProperty();
            const auto importedModelBounds = flag_->getBoundingSphereEXTProperty();
            ctx.Check(importedModelBounds &&
                          importedModelBounds->Contains(stretchedCorner) == ContainmentType::Disjoint,
                      "the imported base-pose bound excludes the deliberately taller morph");

            SetMorphWeightsEXT(*part_, {1.0f, 0.0f});
            const auto unchangedModelBounds = flag_->getBoundingSphereEXTProperty();
            ctx.Check(morph_->Weights == std::vector<float>({1.0f, 0.0f}) && unchangedModelBounds &&
                          unchangedModelBounds->Contains(stretchedCorner) == ContainmentType::Disjoint,
                      "uploading morph weights leaves the documented caller-managed mesh bound unchanged");

            mesh->setBoundingSphereProperty(BoundingSphere::CreateFromPoints(stretchedPositions));
            const auto updatedModelBounds = flag_->getBoundingSphereEXTProperty();
            float farthestDistance = 0.0f;
            if (updatedModelBounds)
            {
                for (const Vector3& point : stretchedPositions)
                {
                    farthestDistance = std::max(farthestDistance,
                                                Vector3::Distance(updatedModelBounds->Center, point));
                }
                ctx.Print("morph bounds: imported radius {:.6f}; updated radius {:.6f}, farthest vertex {:.6f}",
                          importedModelBounds->Radius, updatedModelBounds->Radius, farthestDistance);
            }
            ctx.Check(updatedModelBounds && farthestDistance <= updatedModelBounds->Radius + 1e-5f &&
                          updatedModelBounds->Radius > importedModelBounds->Radius,
                      "updating ModelMesh.BoundingSphere immediately expands the aggregate model bound");
            mesh->setBoundingSphereProperty(importedMeshBounds);
            SetMorphWeightsEXT(*part_, {0.0f, 0.0f});

            // The track is the animation: (0,0) -> (1,0) -> (1,1) -> (0,0) over three seconds.
            const std::vector<float> at1_5 = EvaluateMorphWeightsEXT(morph_->WeightTrack, 1.5);
            ctx.Check(at1_5.size() == 2 && std::abs(at1_5[0] - 1.0f) < 1e-4f && std::abs(at1_5[1] - 0.5f) < 1e-4f,
                      "half way between the keys at 1 s and 2 s the weights are (1, 0.5)");

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 1.3f, 5.5f), Vector3(0.3f, 1.2f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (morph_ == nullptr)
            {
                return;
            }
            // Each frame: weights from the track at the looping time, then re-blend the part's vertices.
            const double t = std::fmod(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() * 1.5, 3.0);
            SetMorphWeightsEXT(*part_, EvaluateMorphWeightsEXT(morph_->WeightTrack, t));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(30, 32, 44));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            flag_->Draw(Matrix::getIdentityProperty(), view_, projection_);
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<Model> flag_;
        ModelMeshPart* part_ = nullptr;
        MorphTargetDataEXT* morph_ = nullptr;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(MorphTargetsDemo, {
    .id = "graphics.model.morph_targets",
    .title = "Morph targets (CNAEXT)",
    .description = "Loads a glTF flag with two morph targets and a weights animation, reads MorphTargetDataEXT from "
                   "the mesh part's Tag, checks BlendMorphTargetsEXT and EvaluateMorphWeightsEXT, and animates the "
                   "shape by re-blending the part with SetMorphWeightsEXT every frame, including caller-managed "
                   "mesh-bound updates for morphed vertices.",
    .apis = {"MorphTargetDataEXT", "MorphWeightTrackEXT", "BlendMorphTargetsEXT", "EvaluateMorphWeightsEXT",
             "SetMorphWeightsEXT", "ModelMeshPart::Tag", "ModelMesh::BoundingSphere", "Model::BoundingSphereEXT", "Model::Draw",
             "ModelMeshPartCollection"},
    .tags = {"3d", "model", "gltf", "morph", "blend-shapes", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 40,
    .assets = {"models/morph_flag.cnb"},
})
