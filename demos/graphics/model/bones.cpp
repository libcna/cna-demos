// SPDX-License-Identifier: MS-PL
// Model bones: the node hierarchy of a model, with relative and absolute transforms.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::string Format(const Vector3& v)
    {
        const auto f = [](float value) {
            const std::string text = std::to_string(value);
            return text.substr(0, text.find('.') + 2);
        };
        return "(" + f(v.X) + ", " + f(v.Y) + ", " + f(v.Z) + ")";
    }

    class BonesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            const Model arm = getContentProperty().Load<Model>("models/robot_arm");

            // Every glTF node is a bone. Transform is relative to the parent bone; the absolute
            // (model-space) transforms are the product down the hierarchy.
            // As in XNA, the caller provides arrays with room for every bone.
            const auto count = static_cast<std::size_t>(arm.getBonesProperty().getCountProperty());
            std::vector<Matrix> relative(count);
            std::vector<Matrix> absolute(count);
            arm.CopyBoneTransformsTo(relative);
            arm.CopyAbsoluteBoneTransformsTo(absolute);
            for (int i = 0; i < arm.getBonesProperty().getCountProperty(); ++i)
            {
                const ModelBone* bone = arm.getBonesProperty()[i];
                std::string indent;
                for (const ModelBone* up = bone->getParentProperty(); up != nullptr; up = up->getParentProperty())
                    indent += "  ";
                lines_.push_back(indent + bone->getNameProperty() + "  relative " + Format(relative[static_cast<std::size_t>(i)].getTranslationProperty()) +
                                 "  absolute " + Format(absolute[static_cast<std::size_t>(i)].getTranslationProperty()));
                Context().Print("{}", lines_.back());
            }

            // Bones are found by name; meshes know the bone that places them.
            const ModelBone* forearm = arm.getBonesProperty()["Forearm"];
            Context().Check(forearm != nullptr && forearm->getParentProperty()->getNameProperty() == "UpperArm",
                            "Bones[\"Forearm\"] is a child of UpperArm");
            Context().Check(arm.getRootProperty()->getParentProperty() == nullptr, "Root has no parent");
            Context().Check(Vector3::Distance(absolute[static_cast<std::size_t>(forearm->getIndexProperty())].getTranslationProperty(),
                                              Vector3(0.0f, 2.5f, 0.0f)) < 1e-5f,
                            "the forearm sits at 0.5 (UpperArm) + 2.0 (Forearm) = 2.5 above the origin");
            for (const ModelMesh* mesh : arm.getMeshesProperty())
                lines_.push_back("mesh " + mesh->getNameProperty() + " -> bone " + mesh->getParentBoneProperty()->getNameProperty());
            Context().Check(arm.getMeshesProperty()[2]->getParentBoneProperty() == forearm,
                            "ForearmMesh is placed by the Forearm bone");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            float y = 30.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::White);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(BonesDemo, {
    .id = "graphics.model.bones",
    .title = "Model bones",
    .description = "Lists the robot arm's bone hierarchy with relative (CopyBoneTransformsTo) and absolute "
                   "(CopyAbsoluteBoneTransformsTo) translations, finds a bone by name and checks each mesh's "
                   "ParentBone.",
    .apis = {"Model::Bones", "Model::Root", "Model::CopyBoneTransformsTo", "Model::CopyAbsoluteBoneTransformsTo",
             "ModelBone::Name", "ModelBone::Index", "ModelBone::Parent", "ModelBone::Transform",
             "ModelBoneCollection::Item", "ModelMeshCollection::Item", "ModelMesh::ParentBone", "ModelMesh::Name",
             "Model::Meshes"},
    .tags = {"model", "bones", "hierarchy", "transforms", "skeleton"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "models/robot_arm.cnb"},
})
