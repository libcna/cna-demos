// SPDX-License-Identifier: MS-PL
// Model bones: the node hierarchy of a model, with relative and absolute transforms.
#include <algorithm>
#include <cstddef>
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
#include "System/ArgumentOutOfRangeException.hpp"

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
            Model arm = getContentProperty().Load<Model>("models/robot_arm");

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
            ModelBone* forearm = arm.getBonesProperty()["Forearm"];
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

            ModelBone* foundBone = nullptr;
            ModelBone detachedBone(-1, "Detached");
            Context().Check(arm.getBonesProperty().TryGetValue("Forearm", foundBone) && foundBone == forearm &&
                                arm.getBonesProperty().Contains(foundBone) &&
                                !arm.getBonesProperty().Contains(&detachedBone) &&
                                !arm.getBonesProperty().TryGetValue("Missing", foundBone) && foundBone == nullptr,
                            "bone lookup and membership distinguish the named member from absent and detached bones");
            Context().Check(forearm->getParentProperty()->getChildrenProperty().getCountProperty() > 0 &&
                                forearm->getParentProperty()->getChildrenProperty().Contains(forearm) &&
                                forearm->getParentProperty()->getChildrenProperty().TryGetValue("Forearm", foundBone) &&
                                foundBone == forearm,
                            "a parent's Children collection contains and finds the same Forearm object");

            ModelMesh* foundMesh = nullptr;
            ModelMesh* forearmMesh = arm.getMeshesProperty()[2];
            Context().Check(arm.getMeshesProperty().TryGetValue(forearmMesh->getNameProperty(), foundMesh) &&
                                foundMesh == forearmMesh && arm.getMeshesProperty().Contains(foundMesh) &&
                                !arm.getMeshesProperty().Contains(nullptr) &&
                                !arm.getMeshesProperty().TryGetValue("Missing", foundMesh) && foundMesh == nullptr,
                            "mesh lookup and membership distinguish the named member from an absent mesh");

            bool shortSourceRefused = false;
            try
            {
                arm.CopyBoneTransformsFrom(std::vector<Matrix>(count - 1));
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                shortSourceRefused = true;
            }
            std::vector<Matrix> afterRefusal(count);
            arm.CopyBoneTransformsTo(afterRefusal);
            Context().Check(shortSourceRefused && afterRefusal == relative,
                            "a short source is refused before changing any local bone transform");

            const int rootIndex = arm.getRootProperty()->getIndexProperty();
            std::vector<Matrix> posed = relative;
            posed[static_cast<std::size_t>(rootIndex)].M41 += 1.0f;
            posed.push_back(Matrix::CreateTranslation(99.0f, 99.0f, 99.0f));
            arm.CopyBoneTransformsFrom(posed);
            std::vector<Matrix> posedLocal(count + 1, Matrix::CreateTranslation(42.0f, 42.0f, 42.0f));
            std::vector<Matrix> posedAbsolute(count);
            arm.CopyBoneTransformsTo(posedLocal);
            arm.CopyAbsoluteBoneTransformsTo(posedAbsolute);
            Context().Check(std::equal(posedLocal.begin(), posedLocal.begin() + static_cast<std::ptrdiff_t>(count), posed.begin()) &&
                                posedLocal.back() == Matrix::CreateTranslation(42.0f, 42.0f, 42.0f),
                            "CopyBoneTransformsFrom updates every local transform and ignores an extra source value");
            const Vector3 originalForearm = absolute[static_cast<std::size_t>(forearm->getIndexProperty())].getTranslationProperty();
            const Vector3 posedForearm = posedAbsolute[static_cast<std::size_t>(forearm->getIndexProperty())].getTranslationProperty();
            Context().Check(Vector3::Distance(posedForearm, originalForearm + Vector3::UnitX) < 1e-5f,
                            "the changed root translation composes through the Forearm's absolute transform");

            bool shortLocalRefused = false;
            bool shortAbsoluteRefused = false;
            std::vector<Matrix> shortDestination(count - 1, Matrix::CreateScale(17.0f));
            const std::vector<Matrix> shortSentinel = shortDestination;
            try
            {
                arm.CopyBoneTransformsTo(shortDestination);
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                shortLocalRefused = true;
            }
            try
            {
                arm.CopyAbsoluteBoneTransformsTo(shortDestination);
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                shortAbsoluteRefused = true;
            }
            Context().Check(shortLocalRefused && shortAbsoluteRefused && shortDestination == shortSentinel,
                            "short local and absolute destinations are refused without partial output");
            arm.CopyBoneTransformsFrom(relative);
            arm.CopyBoneTransformsTo(afterRefusal);
            Context().Check(afterRefusal == relative,
                            "restoring the original local array restores every authored bone transform");
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
    .description = "Navigates the robot arm's bone and mesh collections, copies local and absolute transforms, "
                   "applies a complete pose and verifies its hierarchy composition and array boundaries.",
    .apis = {"Model::Bones", "Model::Root", "Model::CopyBoneTransformsFrom", "Model::CopyBoneTransformsTo",
             "Model::CopyAbsoluteBoneTransformsTo", "ModelBone::Name", "ModelBone::Index", "ModelBone::Parent",
             "ModelBone::Children", "ModelBone::Transform", "ModelBoneCollection::Count",
             "ModelBoneCollection::Item", "ModelBoneCollection::TryGetValue", "ModelBoneCollection::Contains",
             "ModelMeshCollection::Count", "ModelMeshCollection::Item", "ModelMeshCollection::TryGetValue",
             "ModelMeshCollection::Contains", "ModelMesh::ParentBone", "ModelMesh::Name", "Model::Meshes"},
    .tags = {"model", "bones", "hierarchy", "transforms", "skeleton"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "models/robot_arm.cnb"},
})
