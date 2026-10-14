// SPDX-License-Identifier: MS-PL
// Imported KHR_materials_specular maps retain independent strength/colour sampler metadata.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ImageBasedLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SpecularSamplerStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " does not sample PbrEffect specular maps");

            model_.emplace(getContentProperty().Load<Model>("models/loose_specular_samplers.gltf"));
            if (!Context().Check(model_->getMeshesProperty().getCountProperty() == 1 &&
                                 model_->getMeshesProperty()[0]->getMeshPartsProperty().getCountProperty() == 1,
                                 "the authored glTF imports one PBR mesh part")) return;
            part_ = model_->getMeshesProperty()[0]->getMeshPartsProperty()[0];
            effect_ = dynamic_cast<PbrEffect*>(part_->getEffectProperty());
            if (!Context().Check(effect_ && effect_->getSpecularMapEXTProperty() &&
                                 effect_->getSpecularColorMapEXTProperty(),
                                 "KHR_materials_specular imports both owned texture maps")) return;

            const auto& samplers = part_->getSpecularSamplerStatesEXTProperty();
            Context().Check(samplers[0].getFilterProperty() == TextureFilter::Point &&
                                samplers[0].getAddressUProperty() == TextureAddressMode::Clamp &&
                                samplers[0].getAddressVProperty() == TextureAddressMode::Clamp,
                            "the strength map retains its authored nearest-clamp sampler");
            Context().Check(samplers[1].getFilterProperty() == TextureFilter::Point &&
                                samplers[1].getAddressUProperty() == TextureAddressMode::Wrap &&
                                samplers[1].getAddressVProperty() == TextureAddressMode::Wrap,
                            "the colour map independently retains its nearest-repeat sampler");

            irradiance_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            prefiltered_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            for (int face = 0; face < 6; ++face)
            {
                const Color black = Color::Black;
                const Color white = Color::White;
                irradiance_->SetData(static_cast<CubeMapFace>(face), &black, 1);
                prefiltered_->SetData(static_cast<CubeMapFace>(face), &white, 1);
            }
            brdf_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color scaleOneBiasZero(255, 0, 0, 255);
            brdf_->SetData(&scaleOneBiasZero, 1);

            ImageBasedLightEXT light;
            light.Irradiance = irradiance_.get();
            light.PrefilteredSpecular = prefiltered_.get();
            light.BrdfLut = brdf_.get();
            light.Intensity = 4.0f;
            effect_->setImageBasedLightEXT(light);
            effect_->setDiffuseColorProperty(Vector3::Zero);
            effect_->setMetallicFactorProperty(0.0f);
            effect_->setRoughnessFactorProperty(0.5f);
            effect_->setEncodeOutputToSrgbEXTProperty(false);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);

            view_ = Matrix::CreateLookAt(Vector3(0, 0, 6), Vector3::Zero, Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), .1f, 100.0f);
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
                const auto& imported = part_->getSpecularSamplerStatesEXTProperty();

                // Wrap reaches alpha zero in the strength map, so no reflection survives.
                device.getSamplerStatesProperty()(5, SamplerState::PointWrap);
                device.getSamplerStatesProperty()(6, SamplerState::PointClamp);
                model_->Draw(Matrix::CreateTranslation(-1.8f, 0, 0), view_, projection_);

                // Selecting imported strength Clamp reaches alpha one; forced colour Clamp is green.
                device.getSamplerStatesProperty()(5, imported[0]);
                device.getSamplerStatesProperty()(6, SamplerState::PointClamp);
                model_->Draw(Matrix::getIdentityProperty(), view_, projection_);

                // Selecting imported colour Repeat independently returns to the first, red texel.
                device.getSamplerStatesProperty()(5, imported[0]);
                device.getSamplerStatesProperty()(6, imported[1]);
                model_->Draw(Matrix::CreateTranslation(1.8f, 0, 0), view_, projection_);

                if (FramesDrawn() == 0)
                {
                    const CnaDemos::BackBufferPixels pixels(device);
                    const std::array<Color, 3> colors = {
                        pixels.At(Center(-1.8f).X, Center(-1.8f).Y),
                        pixels.At(Center(0).X, Center(0).Y),
                        pixels.At(Center(1.8f).X, Center(1.8f).Y),
                    };
                    Context().Print("specular sampler centres: wrapped strength {}, clamped colour {}, imported colour {}",
                                    colors[0].ToString(), colors[1].ToString(), colors[2].ToString());
                    Context().Check(CnaDemos::Near(colors[0], Color::Black, 2),
                                    "application Wrap reaches zero strength and removes the reflection");
                    Context().Check(colors[1].getGProperty() >= 38 && colors[1].getGProperty() <= 43 &&
                                        colors[1].getRProperty() <= 2 && colors[1].getBProperty() <= 2,
                                    "imported strength Clamp enables the green forced-colour sample");
                    Context().Check(colors[2].getRProperty() >= 38 && colors[2].getRProperty() <= 43 &&
                                        colors[2].getGProperty() <= 2 && colors[2].getBProperty() <= 2,
                                    "imported colour Repeat independently selects the red texel");
                }
            }
            DemoGame::Draw(time);
        }

    private:
        Point Center(float x)
        {
            const Vector3 projected = getGraphicsDeviceProperty().getViewportProperty().Project(
                Vector3(x, 0, 0), projection_, view_, Matrix::getIdentityProperty());
            return Point(static_cast<int>(projected.X), static_cast<int>(projected.Y));
        }

        std::optional<Model> model_;
        ModelMeshPart* part_ = nullptr;
        PbrEffect* effect_ = nullptr;
        std::unique_ptr<TextureCube> irradiance_;
        std::unique_ptr<TextureCube> prefiltered_;
        std::unique_ptr<Texture2D> brdf_;
        Matrix view_ = Matrix::getIdentityProperty();
        Matrix projection_ = Matrix::getIdentityProperty();
        bool ready_ = false;
    };
}

CNA_DEMOS_GAME(SpecularSamplerStateDemo, {
    .id = "graphics.model.specular_sampler_state",
    .title = "Select imported specular-map samplers",
    .description = "Loads independent KHR_materials_specular strength/colour sampler metadata, then explicitly "
                   "selects each part slot and verifies the resulting reflection with exact channel pixels.",
    .apis = {"Microsoft::Xna::Framework::Graphics::ModelMeshPart::SpecularSamplerStatesEXT",
             "Model::Draw", "PbrEffect::SpecularMapEXT", "PbrEffect::SpecularColorMapEXT",
             "GraphicsDevice::SamplerStates"},
    .tags = {"model", "gltf", "pbr", "specular", "sampler", "texture", "cnaext", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_specular_samplers.gltf"},
    .restrictions = "PbrEffect specular-map sampling requires a renderer with the EasyGL PBR path.",
})
