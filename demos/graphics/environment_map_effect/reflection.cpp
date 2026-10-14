// SPDX-License-Identifier: MS-PL
// EnvironmentMapEffect: a sphere reflecting a TextureCube, with and without the Fresnel effect.
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/EnvironmentMapEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ReflectionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            sphere_ = CnaDemos::MakeSphere(1.1f);

            // An environment of six solid faces: sky above, ground below, a colour per side.
            constexpr int size = 16;
            environment_ = std::make_unique<TextureCube>(device, size, false, SurfaceFormat::Color);
            const std::array<std::pair<CubeMapFace, Color>, 6> faces = {{
                {CubeMapFace::PositiveX, Color::Red},     {CubeMapFace::NegativeX, Color::Cyan},
                {CubeMapFace::PositiveY, Color::White},   {CubeMapFace::NegativeY, Color::SaddleBrown},
                {CubeMapFace::PositiveZ, Color::Lime},    {CubeMapFace::NegativeZ, Color::Blue},
            }};
            for (const auto& [face, color] : faces)
            {
                const std::vector<Color> pixels(static_cast<std::size_t>(size) * size, color);
                environment_->SetData(face, pixels.data(), static_cast<int>(pixels.size()));
            }

            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);

            effects_[0] = std::make_unique<EnvironmentMapEffect>(device);
            effects_[0]->setWorldProperty(Matrix::CreateTranslation(-1.5f, 0.0f, 0.0f));
            effects_[0]->setViewProperty(Matrix::CreateLookAt(
                Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effects_[0]->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effects_[0]->EnableDefaultLighting();
            effects_[0]->setTextureProperty(white_.get());               // the surface's own texture
            const Vector3 emissive(0.25f, 0.0f, 0.0f);
            const Vector3 environmentSpecular(0.0f, 0.0f, 0.2f);
            effects_[0]->setDiffuseColorProperty(Vector3::Zero);
            effects_[0]->setEmissiveColorProperty(emissive);
            effects_[0]->setAlphaProperty(0.75f);
            effects_[0]->setEnvironmentMapProperty(environment_.get());   // what it reflects
            effects_[0]->setEnvironmentMapAmountProperty(1.0f);
            effects_[0]->setEnvironmentMapSpecularProperty(environmentSpecular);

            std::unique_ptr<Effect> cloneOwner(effects_[0]->Clone());
            auto* clone = dynamic_cast<EnvironmentMapEffect*>(cloneOwner.get());
            if (clone == nullptr)
                throw std::runtime_error("EnvironmentMapEffect::Clone returned the wrong runtime type");
            Context().Check(clone->getTextureProperty() == white_.get() &&
                                clone->getEnvironmentMapProperty() == environment_.get() &&
                                clone->getDiffuseColorProperty() == Vector3::Zero &&
                                clone->getEmissiveColorProperty() == emissive &&
                                clone->getAlphaProperty() == 0.75f &&
                                clone->getEnvironmentMapSpecularProperty() == environmentSpecular &&
                                clone->getAmbientLightColorProperty() == effects_[0]->getAmbientLightColorProperty() &&
                                clone->getDirectionalLight0Property().getEnabledProperty() &&
                                clone->getDirectionalLight1Property().getEnabledProperty() &&
                                clone->getDirectionalLight2Property().getEnabledProperty(),
                            "Clone retains texture identities, complete material and all default lights");
            effects_[1].reset(static_cast<EnvironmentMapEffect*>(cloneOwner.release()));

            effects_[0]->setAlphaProperty(0.25f);
            effects_[0]->setEmissiveColorProperty(Vector3::One);
            effects_[0]->setEnvironmentMapSpecularProperty(Vector3::Zero);
            Context().Check(effects_[1]->getAlphaProperty() == 0.75f &&
                                effects_[1]->getEmissiveColorProperty() == emissive &&
                                effects_[1]->getEnvironmentMapSpecularProperty() == environmentSpecular,
                            "clone alpha, emissive and environment specular state are independent");
            effects_[0]->setAlphaProperty(0.75f);
            effects_[0]->setEmissiveColorProperty(emissive);
            effects_[0]->setEnvironmentMapSpecularProperty(environmentSpecular);

            // FresnelFactor 0: the same reflection everywhere. The default (1) reflects little where
            // the surface faces the viewer and more and more towards the silhouette, like real glossy paint.
            effects_[0]->setFresnelFactorProperty(0.0f);
            effects_[1]->setWorldProperty(Matrix::CreateTranslation(1.5f, 0.0f, 0.0f));
            Context().Check(effects_[0]->getWorldProperty() != effects_[1]->getWorldProperty() &&
                                effects_[0]->getFresnelFactorProperty() == 0.0f &&
                                effects_[1]->getFresnelFactorProperty() == 1.0f,
                            "clone transform and Fresnel changes are independent");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (auto& effect : effects_)
            {
                for (EffectPass& pass : effect->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, sphere_.vertices.data(), 0,
                                                     sphere_.VertexCount(), sphere_.indices.data(), 0,
                                                     sphere_.PrimitiveCount());
                }
            }

            if (FramesDrawn() == 0)
            {
                // The centre of each sphere faces the camera, so it reflects the +Z (lime) face.
                const CnaDemos::BackBufferPixels p(device);
                const Color fullCentre = p.At(263, 240);
                const Color fresnelCentre = p.At(537, 240);
                Context().Print("sphere centre: FresnelFactor 0 {}, FresnelFactor 1 {}", fullCentre.ToString(),
                                fresnelCentre.ToString());
                Context().Check(fullCentre.getRProperty() <= 2 &&
                                    fullCentre.getGProperty() >= 188 && fullCentre.getGProperty() <= 194 &&
                                    fullCentre.getBProperty() >= 35 && fullCentre.getBProperty() <= 41 &&
                                    fullCentre.getAProperty() >= 188 && fullCentre.getAProperty() <= 194,
                                "full lime reflection is alpha-scaled and receives the blue environment specular term");
                Context().Check(fresnelCentre.getRProperty() > 40 &&
                                    fresnelCentre.getGProperty() < fullCentre.getGProperty() - 120 &&
                                    fresnelCentre.getBProperty() >= 35 && fresnelCentre.getBProperty() <= 41 &&
                                    fresnelCentre.getAProperty() >= 188 && fresnelCentre.getAProperty() <= 194,
                                "Fresnel reveals the red emissive surface while preserving specular and alpha");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "FresnelFactor 0", Vector2(170.0f, 400.0f), Color::White);
            spriteBatch_->DrawString(*font_, "FresnelFactor 1 (default)", Vector2(410.0f, 400.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        CnaDemos::Mesh sphere_;
        std::unique_ptr<TextureCube> environment_;
        std::unique_ptr<Texture2D> white_;
        std::array<std::unique_ptr<EnvironmentMapEffect>, 2> effects_;
    };
}

CNA_DEMOS_GAME(ReflectionDemo, {
    .id = "graphics.environment_map_effect.reflection",
    .title = "Environment-mapped reflection",
    .description = "Builds a TextureCube from six coloured faces and reflects it on two spheres with "
                   "EnvironmentMapEffect, with FresnelFactor 0 (uniform mirror) and 1 (stronger at grazing angles).",
    .apis = {"EnvironmentMapEffect::EnvironmentMapEffect", "EnvironmentMapEffect::EnvironmentMap",
             "EnvironmentMapEffect::EnvironmentMapAmount", "EnvironmentMapEffect::Clone",
             "EnvironmentMapEffect::FresnelFactor",
             "EnvironmentMapEffect::Texture", "EnvironmentMapEffect::DiffuseColor", "EnvironmentMapEffect::Alpha",
             "EnvironmentMapEffect::EmissiveColor", "EnvironmentMapEffect::EnvironmentMapSpecular",
             "EnvironmentMapEffect::AmbientLightColor", "EnvironmentMapEffect::DirectionalLight0",
             "EnvironmentMapEffect::DirectionalLight1", "EnvironmentMapEffect::DirectionalLight2",
             "EnvironmentMapEffect::EnableDefaultLighting",
             "TextureCube::TextureCube", "TextureCube::SetData", "CubeMapFace::PositiveZ",
             "EnvironmentMapEffect::World", "EnvironmentMapEffect::View", "EnvironmentMapEffect::Projection"},
    .tags = {"3d", "reflection", "cubemap", "effect", "chrome"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
