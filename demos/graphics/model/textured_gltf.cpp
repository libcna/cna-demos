// SPDX-License-Identifier: MS-PL
// A textured glTF model: the image embedded in the file becomes a Texture2D on the part's BasicEffect.
#include <exception>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class TexturedGltfDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            // Through the content pipeline the embedded image becomes a Texture2D asset of its own
            // beside the compiled model, which refers to it (CNA-DEMOS-ISSUE-022).
            bool compiledTextured = false;
            try
            {
                const Model compiled = getContentProperty().Load<Model>("models/textured_crate");
                const ModelMeshPart* part = *compiled.getMeshesProperty()[0]->getMeshPartsProperty().begin();
                const auto* effect = dynamic_cast<const BasicEffect*>(part->getEffectProperty());
                const Texture2D* texture = effect != nullptr ? effect->getTextureProperty() : nullptr;
                compiledTextured = texture != nullptr && texture->getWidthProperty() == 64 &&
                                   texture->getHeightProperty() == 64;
            }
            catch (const std::exception& error)
            {
                ctx.Print("compiled model: {}", error.what());
            }
            ctx.Check(compiledTextured, "the compiled model loads with the 64x64 texture embedded in the file");
            // The rest of the demo draws the same file imported at run time.
            crate_.emplace(getContentProperty().Load<Model>("models/loose_textured_crate"));
            // The importer decoded the PNG embedded in the glTF buffer into a Texture2D the model
            // owns, and gave the (unlit) part a BasicEffect that samples it.
            const ModelMeshPart* part = *crate_->getMeshesProperty()[0]->getMeshPartsProperty().begin();
            auto* effect = dynamic_cast<BasicEffect*>(part->getEffectProperty());
            ctx.Check(effect != nullptr, "the part has a BasicEffect");
            if (effect == nullptr)
            {
                return;
            }
            const Texture2D* texture = effect->getTextureProperty();
            ctx.Check(effect->getTextureEnabledProperty() && texture != nullptr, "texturing is on, with a texture");
            if (texture != nullptr)
            {
                ctx.Print("base colour texture: {}x{}, {} level(s)", texture->getWidthProperty(), texture->getHeightProperty(),
                          texture->getLevelCountProperty());
                ctx.Check(texture->getWidthProperty() == 64 && texture->getHeightProperty() == 64,
                          "it is the 64x64 crate image from the file");
            }
            view_ = Matrix::CreateLookAt(Vector3(0.0f, 1.8f, 5.0f), Vector3::Zero, Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(28, 30, 40));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            crate_->Draw(Matrix::CreateRotationY(0.6f + t * 0.7f), view_, projection_);

            if (FramesDrawn() == 0)
            {
                // The middle of the screen is on the crate: wood colours from the texture.
                const Color middle = CnaDemos::BackBufferPixels(device).At(400, 250);
                Context().Print("crate at the centre: {}", middle.ToString());
                Context().Check(middle.getRProperty() > middle.getGProperty() && middle.getGProperty() > middle.getBProperty() &&
                                    middle.getRProperty() > 80,
                                "the crate shows the wooden texture");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<Model> crate_;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(TexturedGltfDemo, {
    .id = "graphics.model.textured_gltf",
    .title = "A textured glTF model",
    .description = "Loads a glTF cube whose base-colour texture is a PNG embedded in the file, finds the Texture2D "
                   "the importer created on the part's BasicEffect, and draws the turning crate; the compiled build "
                   "carries the same image as a Texture2D asset of its own.",
    .apis = {"Model::Draw", "ModelMeshPart::Effect", "BasicEffect::Texture", "BasicEffect::TextureEnabled",
             "Texture2D::Width", "Texture2D::Height"},
    .tags = {"3d", "model", "gltf", "texture"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 30,
    .assets = {"models/textured_crate.cnb", "models/loose_textured_crate.gltf"},
})
