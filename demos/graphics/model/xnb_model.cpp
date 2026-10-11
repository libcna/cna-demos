// SPDX-License-Identifier: MS-PL
// A real XNB Model asset: named bones, shared buffer fixups and separate material effects.
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class XnbModelDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            // cna_add_content(FORMAT xnb) compiles our original .x through XImporter/ModelProcessor.
            const auto input = TitleContainer::OpenStream("assets/xnb/models/twin_panels.xnb");
            Context().Check(input->ReadByte() == 'X' && input->ReadByte() == 'N' && input->ReadByte() == 'B',
                            "the built model file carries the XNB container signature");
            model_.emplace(getContentProperty().Load<Model>("xnb/models/twin_panels"));
            auto& model = *model_;
            if (!Context().Check(model.getBonesProperty().getCountProperty() == 5 &&
                                  model.getMeshesProperty().getCountProperty() == 2,
                                  "the XNB reader restores the five-node hierarchy and two meshes")) return;
            ModelBone* leftMount = nullptr;
            ModelBone* rightMount = nullptr;
            auto* root = model.getRootProperty();
            if (!Context().Check(root && root->getNameProperty() == "PanelRig" &&
                                  model.getBonesProperty().TryGetValue("LeftMount", leftMount) &&
                                  model.getBonesProperty().TryGetValue("RightMount", rightMount) &&
                                  leftMount->getParentProperty() == root && rightMount->getParentProperty() == root,
                                  "named child mounts retain their shared root relationship")) return;
            std::vector<Matrix> absolute(5);
            model.CopyAbsoluteBoneTransformsTo(absolute);
            Context().Check(Vector3::Distance(absolute[leftMount->getIndexProperty()].getTranslationProperty(), Vector3(-.5f, .1f, 0)) < .0001f &&
                            Vector3::Distance(absolute[rightMount->getIndexProperty()].getTranslationProperty(), Vector3(.5f, .1f, 0)) < .0001f,
                            "absolute mount transforms compose the root's vertical offset with each side's translation");
            const auto* leftMesh = model.getMeshesProperty()[0];
            const auto* rightMesh = model.getMeshesProperty()[1];
            if (!Context().Check(leftMesh->getMeshPartsProperty().getCountProperty() == 1 &&
                                  rightMesh->getMeshPartsProperty().getCountProperty() == 1,
                                  "each imported mesh retains its one material part")) return;
            const auto* left = leftMesh->getMeshPartsProperty()[0];
            const auto* right = rightMesh->getMeshPartsProperty()[0];
            Context().Check(left->getVertexBufferProperty() && left->getIndexBufferProperty() &&
                            left->getVertexBufferProperty() == right->getVertexBufferProperty() &&
                            left->getIndexBufferProperty() == right->getIndexBufferProperty(),
                            "XNB shared-resource fixups give both parts the same vertex and index buffers");
            Context().Check(left->getNumVerticesProperty() == 4 && right->getNumVerticesProperty() == 4 &&
                            left->getPrimitiveCountProperty() == 2 && right->getPrimitiveCountProperty() == 2 &&
                            left->getStartIndexProperty() != right->getStartIndexProperty() &&
                            left->getVertexOffsetProperty() != right->getVertexOffsetProperty(),
                            "parts address independent quad ranges inside their shared buffers");
            auto* red = dynamic_cast<BasicEffect*>(left->getEffectProperty());
            auto* green = dynamic_cast<BasicEffect*>(right->getEffectProperty());
            if (!Context().Check(red && green && red != green &&
                                  red->getDiffuseColorProperty() == Vector3(1, 0, 0) &&
                                  green->getDiffuseColorProperty() == Vector3(0, 1, 0),
                                  "distinct authored materials become separate red and green BasicEffects")) return;
            red->setLightingEnabledProperty(false);
            green->setLightingEnabledProperty(false); // expose the imported material colors directly
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
                const Matrix identity = Matrix::getIdentityProperty();
                model_->Draw(identity, identity, identity); // includes the restored mesh-parent transforms
                if (!checked_)
                {
                    const CnaDemos::BackBufferPixels pixels(device);
                    const int row = static_cast<int>(.45f * pixels.Height());
                    Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 4, row), Color::Red, 2),
                                    "the XNB model draws its red panel at the left mount");
                    Context().Check(CnaDemos::Near(pixels.At(3 * pixels.Width() / 4, row), Color::Lime, 2),
                                    "the XNB model draws its green panel at the right mount");
                    Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 2, row), Color::DarkSlateGray, 2),
                                    "the gap between independently positioned panels stays clear");
                    checked_ = true;
                }
            }
            Game::Draw(time);
        }

    private:
        std::optional<Model> model_;
        bool ready_ = false, checked_ = false;
    };
}

CNA_DEMOS_GAME(XnbModelDemo, {
    .id = "graphics.model.xnb_model",
    .title = "Load an XNB model with shared resources",
    .description = "Builds an original .x hierarchy into XNB, checks restored named bones, shared buffers and material effects, "
                   "then verifies both mesh placements with Model::Draw and actual pixels.",
    .apis = {"ContentManager::Load", "Model::Root", "Model::Bones", "Model::Meshes", "Model::CopyAbsoluteBoneTransformsTo",
             "Model::Draw", "ModelBoneCollection::TryGetValue", "ModelBone::Parent", "ModelMesh::MeshParts",
             "ModelMeshPart::VertexBuffer", "ModelMeshPart::IndexBuffer", "ModelMeshPart::Effect"},
    .tags = {"model", "xnb", "content", "hierarchy", "shared-resources", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"xnb/models/twin_panels.xnb"},
})
