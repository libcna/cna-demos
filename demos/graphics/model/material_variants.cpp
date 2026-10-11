// SPDX-License-Identifier: MS-PL
// glTF KHR_materials_variants: one model, several paint jobs, switched at run time (CNAEXT).
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
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
            // A red body with the variants "Ocean" (blue) and "Gold", and black wheels with no
            // variant mapping (tools/gltf_models.py). Imported at run time: CNA's content pipeline
            // refuses material variants, which its compiled model format cannot carry.
            toy_.emplace(getContentProperty().Load<Model>("models/loose_paint_toy"));
            const auto& names = toy_->getMaterialVariantNamesEXTProperty();
            std::string list;
            for (const std::string& name : names)
                list += " " + name;
            ctx.Print("variants:{}", list);
            ctx.Check(names.size() == 2 && names[0] == "Ocean" && names[1] == "Gold",
                      "the model lists the file's variants in order");
            ctx.Check(toy_->getMaterialVariantEXTProperty() == -1, "a loaded model shows its default materials (-1)");

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
            if (FramesDrawn() == 0)
            {
                // The body's top in the middle of the screen, a wheel near the bottom left.
                const Color red = Paint(-1);
                const Color blue = Paint(0);
                const Color gold = Paint(1);
                Context().Print("body: default {}, Ocean {}, Gold {}", red.ToString(), blue.ToString(), gold.ToString());
                Context().Check(red.getRProperty() > 180 && red.getBProperty() < 60, "the default body is red");
                Context().Check(blue.getBProperty() > 180 && blue.getRProperty() < 60, "Ocean paints it blue");
                Context().Check(gold.getRProperty() > 200 && gold.getGProperty() > 150 && gold.getBProperty() < 80,
                                "Gold paints it gold");
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

        // Selects a variant, draws, and reads the body's colour.
        Color Paint(int variant)
        {
            toy_->setMaterialVariantEXTProperty(variant);
            Render();
            return CnaDemos::BackBufferPixels(getGraphicsDeviceProperty()).At(BodyX, BodyY);
        }

        static constexpr int BodyX = 400;
        static constexpr int BodyY = 200;
        static constexpr int WheelX = 295;
        static constexpr int WheelY = 300;

        std::optional<Model> toy_;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(MaterialVariantsDemo, {
    .id = "graphics.model.material_variants",
    .title = "Material variants (glTF)",
    .description = "A toy car whose glTF declares KHR_materials_variants: Model::MaterialVariantNamesEXT lists "
                   "\"Ocean\" and \"Gold\", MaterialVariantEXT repaints the body (-1 restores the default red), and "
                   "the wheels without a mapping keep their material.",
    .apis = {"Model::MaterialVariantNamesEXT", "Model::MaterialVariantEXT", "Model::Draw", "ContentManager::Load"},
    .tags = {"model", "gltf", "material", "variant", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_paint_toy.gltf"},
})
