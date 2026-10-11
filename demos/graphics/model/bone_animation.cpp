// SPDX-License-Identifier: MS-PL
// Animating a model by its bones: change a bone's Transform, then draw each mesh with the
// absolute transform of its parent bone -- XNA's classic per-mesh drawing loop.
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class BoneAnimationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            arm_.emplace(getContentProperty().Load<Model>("models/robot_arm"));
            upperArm_ = arm_->getBonesProperty()["UpperArm"];
            forearm_ = arm_->getBonesProperty()["Forearm"];
            // The authored (rest) transforms; animation rotates on top of them.
            upperRest_ = upperArm_->getTransformProperty();
            forearmRest_ = forearm_->getTransformProperty();

            // Rotating UpperArm a quarter turn about Z swings everything below it: the forearm's
            // base, 2 up the upper arm, ends up 2 to the left.
            Pose(MathHelper::PiOver2, 0.0f);
            // As in XNA, the destination has room for every bone (CopyAbsoluteBoneTransformsTo
            // refuses a smaller one).
            absolute_.resize(static_cast<std::size_t>(arm_->getBonesProperty().getCountProperty()));
            std::vector<Matrix>& absolute = absolute_;
            arm_->CopyAbsoluteBoneTransformsTo(absolute);
            const Vector3 elbow = absolute[static_cast<std::size_t>(forearm_->getIndexProperty())].getTranslationProperty();
            Context().Print("UpperArm turned 90 degrees: elbow at ({:.2f}, {:.2f}, {:.2f})", elbow.X, elbow.Y, elbow.Z);
            Context().Check(Vector3::Distance(elbow, Vector3(-2.0f, 0.5f, 0.0f)) < 1e-4f,
                            "a parent bone's rotation moves its children");

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 3.0f, 9.0f), Vector3(0.0f, 1.8f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            Pose(0.6f * std::sin(t), 1.2f * std::sin(t * 1.7f));
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::CornflowerBlue);
            device.setDepthStencilStateProperty(DepthStencilState::Default);

            // The loop Model::Draw runs internally, written out: each mesh's effects get the
            // absolute transform of the mesh's parent bone, then the mesh draws.
            arm_->CopyAbsoluteBoneTransformsTo(absolute_);
            for (ModelMesh* mesh : arm_->getMeshesProperty())
            {
                for (Effect* effect : mesh->getEffectsProperty())
                {
                    auto* basic = dynamic_cast<BasicEffect*>(effect);
                    basic->setWorldProperty(absolute_[static_cast<std::size_t>(mesh->getParentBoneProperty()->getIndexProperty())]);
                    basic->setViewProperty(view_);
                    basic->setProjectionProperty(projection_);
                }
                mesh->Draw();
            }
            Game::Draw(gameTime);
        }

    private:
        void Pose(float shoulder, float elbow)
        {
            upperArm_->setTransformProperty(Matrix::CreateRotationZ(shoulder) * upperRest_);
            forearm_->setTransformProperty(Matrix::CreateRotationZ(elbow) * forearmRest_);
        }

        std::optional<Model> arm_;
        ModelBone* upperArm_ = nullptr;
        ModelBone* forearm_ = nullptr;
        Matrix upperRest_;
        Matrix forearmRest_;
        std::vector<Matrix> absolute_;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(BoneAnimationDemo, {
    .id = "graphics.model.bone_animation",
    .title = "Animate model bones",
    .description = "Swings the robot arm by setting ModelBone::Transform on its shoulder and elbow each frame, then "
                   "draws every mesh with its parent bone's absolute transform from CopyAbsoluteBoneTransformsTo.",
    .apis = {"ModelBone::Transform", "Model::CopyAbsoluteBoneTransformsTo", "ModelMesh::Effects", "ModelMesh::Draw",
             "ModelMesh::ParentBone", "BasicEffect::World", "BasicEffect::View", "BasicEffect::Projection",
             "Matrix::CreateRotationZ", "Model::Bones", "Model::Meshes", "ModelBone::Index"},
    .tags = {"model", "bones", "animation", "hierarchy", "robot", "3d"},
    .needs = CnaDemos::Need::ThreeD,
})
