// SPDX-License-Identifier: MS-PL
// Independent specular and diffuse light terms, their sums and material shininess on a skinned mesh.
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
namespace
{
    class SpecularLightsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (const Vector2& position : {Vector2(-.35f, -.36f), Vector2(.35f, -.36f),
                                           Vector2(.35f, .36f), Vector2(-.35f, .36f)})
                vertices.emplace_back(Vector3(position.X, position.Y, 0),
                    Vector3(0, 0, 1),
                    Vector2::Zero, Vector4(1, 0, 0, 0), std::array<std::uint8_t, 4>{0, 0, 0, 0});
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 4, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 4);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);
            texture_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color texel = Color::Black; // Opaque: specular adds independently of texture RGB.
            texture_->SetData(&texel, 1);
            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color whiteTexel = Color::White;
            white_->SetData(&whiteTexel, 1);
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(texture_.get());
            effect_->setWeightsPerVertexProperty(1);
            effect_->setWorldProperty(Matrix::getIdentityProperty());
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::CreateOrthographic(1, 1.2f, .1f, 10));
            effect_->setAmbientLightColorProperty(Vector3::Zero);
            effect_->setEmissiveColorProperty(Vector3::Zero);
            effect_->setDiffuseColorProperty(Vector3::Zero);
            effect_->setSpecularColorProperty(Vector3(.5f, 1, .25f));
            effect_->setPreferPerPixelLightingProperty(true);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            auto& red = effect_->getDirectionalLight1Property();
            red.setDirectionProperty(Vector3(-.8f, 0, -.6f));
            red.setDiffuseColorProperty(Vector3::Zero);
            red.setSpecularColorProperty(Vector3(1, 0, 0));
            auto& blue = effect_->getDirectionalLight2Property();
            blue.setDirectionProperty(Vector3(.8f, 0, -.6f));
            blue.setDiffuseColorProperty(Vector3::Zero);
            blue.setSpecularColorProperty(Vector3(0, 0, 1));
            effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -3)});
            Context().Print("Two rows of equal-camera viewports. Top retains red/blue specular contributions, their sum and power 8. Bottom isolates two diffuse lights, sums them, then adds specular. Diffuse material is (.5,1,.25); light colors (1,.5,0) and (0,.5,1) share N dot L=.6, so their green terms sum in float before byte rounding. One bone moves the bind mesh to Z=-3.");
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            device.Clear(Color::MidnightBlue); // different from every expected lit sample
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            const int rowHeight = full.getHeightProperty() / 2;
            for (int row = 0; row < 2; ++row)
            {
                effect_->setTextureProperty(row == 0 ? texture_.get() : white_.get());
                effect_->setDiffuseColorProperty(row == 0 ? Vector3::Zero : Vector3(.5f, 1, .25f));
                effect_->getDirectionalLight1Property().setDiffuseColorProperty(
                    row == 0 ? Vector3::Zero : Vector3(1, .5f, 0));
                effect_->getDirectionalLight2Property().setDiffuseColorProperty(
                    row == 0 ? Vector3::Zero : Vector3(0, .5f, 1));
                for (int panel = 0; panel < 4; ++panel)
                {
                    const int left = full.getWidthProperty() * panel / 4;
                    const int right = full.getWidthProperty() * (panel + 1) / 4;
                    device.setViewportProperty(Viewport(left, row * rowHeight, right - left, rowHeight));
                    effect_->getDirectionalLight1Property().setEnabledProperty(panel != 1);
                    effect_->getDirectionalLight2Property().setEnabledProperty(panel != 0);
                    effect_->setSpecularColorProperty(row == 0 || panel == 3 ? Vector3(.5f, 1, .25f) : Vector3::Zero);
                    effect_->setSpecularPowerProperty(row == 0 && panel == 3 ? 8.0f : 2.0f);
                    for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                    }
                }
            }
            device.setViewportProperty(full);
            constexpr std::array<const char*, 4> topLabels{"Specular 1", "Specular 2", "Sum", "Power 8"};
            constexpr std::array<const char*, 4> bottomLabels{"Diffuse 1", "Diffuse 2", "Sum", "+ specular"};
            batch_->Begin();
            for (int panel = 0; panel < 4; ++panel)
            {
                const float left = static_cast<float>(full.getWidthProperty() * panel / 4 + 10);
                batch_->DrawString(*font_, topLabels[panel], Vector2(left, 12), Color::White);
                batch_->DrawString(*font_, bottomLabels[panel], Vector2(left, static_cast<float>(rowHeight + 12)), Color::White);
            }
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // Eye direction is +Z at each panel center. The symmetric light directions give
                // dot(N,H)^2=.8; power 8 gives .8^4=.4096. Pixel-center offsets fit tolerance 1.
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 8, pixels.Height() / 4), Color(102, 0, 0), 1),
                                "light 1 supplies its independent red specular contribution scaled by material color");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 3 / 8, pixels.Height() / 4), Color(0, 0, 51), 1),
                                "light 2 supplies its independent blue specular contribution scaled by material color");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 5 / 8, pixels.Height() / 4), Color(102, 0, 51), 1),
                                "both enabled skinned specular lights add without diffuse or texture-RGB modulation");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 7 / 8, pixels.Height() / 4), Color(52, 0, 26), 1),
                                "higher material SpecularPower reduces this off-axis highlight by the expected exponent");
                // N dot L=.6 for both authored directions. Sum the float light terms before
                // quantization: two .3 green contributions give153, not rounded77+77.
                const int diffuseY = pixels.Height() * 3 / 4;
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 8, diffuseY), Color(77, 77, 0), 1),
                                "light 1 supplies its independent diffuse color multiplied by material and N dot L");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 3 / 8, diffuseY), Color(0, 77, 38), 1),
                                "light 2 supplies its independent diffuse color multiplied by material and N dot L");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 5 / 8, diffuseY), Color(77, 153, 38), 1),
                                "both skinned diffuse lights add before the final eight-bit channel conversion");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 7 / 8, diffuseY), Color(179, 153, 89), 1),
                                "the combined skinned result adds both independently verified diffuse and specular terms");
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> texture_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}
CNA_DEMOS_GAME(SpecularLightsDemo, {
    .id = "graphics.skinned_effect.specular_lights",
    .title = "Colored light contributions on a skinned mesh",
    .description = "Isolates directional lights 1 and 2, their colored specular sum and the material shininess "
                   "exponent, then checks their independent diffuse terms and combined result with equal-camera viewports "
                   "and a bone-transformed mesh.",
    .apis = {"SkinnedEffect::SpecularColor", "SkinnedEffect::SpecularPower", "SkinnedEffect::DirectionalLight1",
             "SkinnedEffect::DirectionalLight2", "SkinnedEffect::DiffuseColor", "DirectionalLight::DiffuseColor",
             "SkinnedEffect::PreferPerPixelLighting", "SkinnedEffect::SetBoneTransforms"},
    .tags = {"skinning", "bones", "effect", "lighting", "specular", "diffuse", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Per-pixel lighting, opaque textures and light 0 disabled. Vertex-lighting variants, "
                    "nonuniform joint scaling, arbitrary weights and alpha interactions remain separate.",
})
