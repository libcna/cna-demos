// SPDX-License-Identifier: MS-PL
// Authored shadow depths distinguish direct light from the ambient/emissive floor after skinning.
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
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
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
namespace
{
    class ShadowReceiverDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer != "OPENGLES3" && renderer != "OPENGL33")
                Context().Skip("this shadow-receiver example requires the EasyGL stock shadow-sampling path");
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (const Vector2& position : {Vector2(-.35f, -.36f), Vector2(.35f, -.36f),
                                           Vector2(.35f, .36f), Vector2(-.35f, .36f)})
                vertices.emplace_back(Vector3(position.X, position.Y, 0), Vector3(0, 0, 1),
                    Vector2::Zero, Vector4(1, 0, 0, 0), std::array<std::uint8_t, 4>{0, 0, 0, 0});
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 4, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 4);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);
            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
            shadow_ = std::make_unique<Texture2D>(device, 8, 4);
            std::array<Color, 32> depths;
            for (int i = 0; i < 32; ++i)
                depths[i] = Color(i % 8 < 4 ? 64 : 192, 0, 0); // red stores normalized light depth
            shadow_->SetData(depths.data(), static_cast<int>(depths.size()));
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(white_.get());
            effect_->setWeightsPerVertexProperty(1);
            effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -3)});
            effect_->setWorldProperty(Matrix::getIdentityProperty());
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::CreateOrthographic(1, 1.2f, .1f, 10));
            effect_->setPreferPerPixelLightingProperty(true);
            effect_->setDiffuseColorProperty(Vector3(.6f, .4f, .2f));
            effect_->setAmbientLightColorProperty(Vector3(.1f, .2f, .3f));
            effect_->setEmissiveColorProperty(Vector3(.02f, .03f, .04f));
            effect_->setSpecularColorProperty(Vector3::Zero);
            auto& light = effect_->getDirectionalLight0Property();
            light.setEnabledProperty(true);
            light.setDirectionProperty(Vector3(0, 0, -1));
            light.setDiffuseColorProperty(Vector3::One);
            light.setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            // Post-skin Z=-3 becomes clip Z=0, hence comparison depth=.5 after clip-to-map conversion.
            effect_->setLightViewProjectionEXT(Matrix::CreateTranslation(0, 0, 3));
            effect_->setShadowFilterRadiusEXT(0); // isolate a single comparison, not PCF averaging
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            Context().Print("The left half of the authored map stores 64/255, the right 192/255. Receiver depth=.5; only the left half is occluded. Ambient and emission remain. Bias=.3 deliberately removes that shadow, illustrating why excessive bias loses contact. This is receiver sampling, not shadow-map generation.");
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            device.Clear(Color::MidnightBlue);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (int panel = 0; panel < 4; ++panel)
            {
                const int left = full.getWidthProperty() * panel / 4;
                const int right = full.getWidthProperty() * (panel + 1) / 4;
                device.setViewportProperty(Viewport(left, 0, right - left, full.getHeightProperty()));
                effect_->setShadowMapEXT(panel == 3 ? nullptr : shadow_.get());
                effect_->setShadowsEnabledEXT(panel != 0);
                effect_->setShadowDepthBiasEXT(panel == 2 ? .3f : 0.0f);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            device.setViewportProperty(full);
            const std::array<const char*, 4> labels{"Disabled", "Enabled", "Bias .3", "Detached"};
            batch_->Begin();
            for (int panel = 0; panel < 4; ++panel)
                batch_->DrawString(*font_, labels[panel], Vector2(static_cast<float>(panel * full.getWidthProperty() / 4 + 8), 20), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // lit = diffuse + ambient*diffuse + emissive = (.68,.51,.30).
                // occluded = ambient*diffuse + emissive = (.08,.11,.10).
                const Color lit(173, 130, 77), occluded(20, 28, 26);
                const std::array<const char*, 4> checks{
                    "disabling reception leaves both halves directly lit while the map stays attached",
                    "the post-skin shadow lookup removes only direct light on the occluded half",
                    "excessive positive depth bias removes the shadow while the same map remains attached",
                    "detaching the map restores direct light even with reception enabled"};
                for (int panel = 0; panel < 4; ++panel)
                {
                    const int left = pixels.Width() * panel / 4;
                    const int width = pixels.Width() / 4;
                    Context().Check(CnaDemos::Near(pixels.At(left + width / 4, pixels.Height() / 2), panel == 1 ? occluded : lit, 1) &&
                                    CnaDemos::Near(pixels.At(left + width * 3 / 4, pixels.Height() / 2), lit, 1), checks[panel]);
                }
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> white_, shadow_;
        std::unique_ptr<SkinnedEffect> effect_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
    };
}
CNA_DEMOS_GAME(ShadowReceiverDemo, {
    .id = "graphics.skinned_effect.shadow_receiver",
    .title = "Authored-map shadows on a skinned receiver",
    .description = "Samples an authored depth map after a bone transform, contrasts direct-light shadowing with the "
                   "retained ambient/emissive floor, and compares disabled reception, depth bias and map detachment.",
    .apis = {"SkinnedEffect::SetBoneTransforms", "SkinnedEffect::DiffuseColor", "SkinnedEffect::AmbientLightColor",
             "SkinnedEffect::EmissiveColor", "SkinnedEffect::PreferPerPixelLighting",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowMapEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setLightViewProjectionEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowsEnabledEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowDepthBiasEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowFilterRadiusEXT"},
    .tags = {"skinning", "shadows", "lighting", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "EasyGL OPENGLES3/OPENGL33 stock receiver path only; other renderers explicitly skipped. "
                    "An authored Color map and single-tap comparisons do not establish map generation, PCF kernels, "
                    "cascades, punctual-light shadows, specular shadowing or other renderers.",
})
