// SPDX-License-Identifier: MS-PL
// Authored distance maps distinguish point and spot shadow reception after skinning.
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
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/PunctualLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
namespace
{
    class PunctualShadowsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer != "OPENGLES3" && renderer != "OPENGL33")
                Context().Skip("this punctual-light example requires the EasyGL stock light-sampling path");
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
            shadow_ = std::make_unique<Texture2D>(device, 4, 4);
            std::array<Color, 16> distances;
            distances.fill(Color(64, 0, 0)); // normalized distance/range, not projected depth
            shadow_->SetData(distances.data(), static_cast<int>(distances.size()));
            cube_ = std::make_unique<TextureCube>(device, 4, false, SurfaceFormat::Color);
            for (int face = 0; face < 6; ++face)
                cube_->SetData(static_cast<CubeMapFace>(face), distances.data(), static_cast<int>(distances.size()));
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
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            Context().Print("The bone puts the receiver three units from a point/spot light. Linear light intensity4 makes the comparison visible. Range6 gives normalized distance.5; both authored maps store64/255. No map leaves direct diffuse light, the map removes it, and bias.3 restores it. Separate controls exclude the receiver by range or cone. Ambient and emission remain. These are authored receiver inputs, not generated shadows.");
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
            for (int panel = 0; panel < 8; ++panel)
            {
                const int column = panel % 4, row = panel / 4;
                const int left = full.getWidthProperty() * column / 4;
                const int right = full.getWidthProperty() * (column + 1) / 4;
                device.setViewportProperty(Viewport(left, row * full.getHeightProperty() / 2,
                                                   right - left, full.getHeightProperty() / 2));
                PunctualLightEXT light;
                light.Kind = row == 0 ? PunctualLightKindEXT::Point : PunctualLightKindEXT::Spot;
                light.Position = Vector3::Zero;
                light.Direction = panel == 7 ? Vector3(1, 0, 0) : Vector3(0, 0, -1);
                light.DiffuseColor = Vector3(4, 4, 4); // linear intensity, for a clearly visible comparison
                light.Range = panel == 3 ? 2.0f : 6.0f;
                light.InnerAngle = .3f;
                light.OuterAngle = .5f;
                light.ShadowDepthBias = column == 2 ? .3f : 0.0f;
                if (column == 1 || column == 2)
                {
                    if (row == 0) light.ShadowCube = cube_.get();
                    else light.ShadowMap = shadow_.get();
                }
                // Spot XY projects into the center region. Comparison depth still comes
                // from distance/Range=.5; projected depth=.1 would wrongly leave this pixel lit.
                light.ShadowViewProjection = Matrix::CreateTranslation(0, 0, 2.2f);
                effect_->setPunctualLightEXT(light);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            device.setViewportProperty(full);
            const std::array<const char*, 8> labels{
                "Point: no map", "Point: shadow", "Point: bias .3", "Outside range",
                "Spot: no map", "Spot: shadow", "Spot: bias .3", "Outside cone"};
            batch_->Begin();
            for (int panel = 0; panel < 8; ++panel)
                batch_->DrawString(*font_, labels[panel], Vector2(
                    static_cast<float>((panel % 4) * full.getWidthProperty() / 4 + 8),
                    static_cast<float>((panel / 4) * full.getHeightProperty() / 2 + 8)), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // At the panel center, distance=3 and Range=6: the windowed inverse-square
                // attenuation is (1-(3/6)^4)^2/(1+3^2)=.087890625, with N dot L=1.
                // Multiply by the light intensity4 and material diffuse, then add the floor (.08,.11,.10).
                const Color lit(74, 64, 43), floor(20, 28, 26);
                const std::array<Color, 8> expected{lit, floor, lit, floor, lit, floor, lit, floor};
                const std::array<const char*, 8> checks{
                    "a point light without a cube contributes distance-attenuated diffuse light",
                    "the point distance cube removes direct light after the bone translation",
                    "point bias in normalized range units removes the authored shadow",
                    "a point light contributes nothing beyond its range",
                    "an aligned spot without a map contributes its central diffuse light",
                    "the spot distance map removes direct light after the bone translation",
                    "spot bias in normalized range units removes the authored shadow",
                    "a spot pointing away contributes nothing outside its cone"};
                for (int panel = 0; panel < 8; ++panel)
                    Context().Check(CnaDemos::Near(pixels.At(
                        pixels.Width() * (2 * (panel % 4) + 1) / 8,
                        pixels.Height() * (2 * (panel / 4) + 1) / 4), expected[panel], 1), checks[panel]);
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> white_, shadow_;
        std::unique_ptr<TextureCube> cube_;
        std::unique_ptr<SkinnedEffect> effect_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
    };
}
CNA_DEMOS_GAME(PunctualShadowsDemo, {
    .id = "graphics.skinned_effect.punctual_shadows",
    .title = "Point and spot distance-map reception",
    .description = "Compares unshadowed point and spot lights with authored distance-map shadows, normalized bias, "
                   "range exclusion and cone exclusion after a bone translation.",
    .apis = {"SkinnedEffect::SetBoneTransforms", "TextureCube::SetData", "Texture2D::SetData",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setPunctualLightEXT",
             "Microsoft::Xna::Framework::Graphics::PunctualLightEXT"},
    .tags = {"skinning", "shadows", "point-light", "spot-light", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "EasyGL OPENGLES3/OPENGL33 stock light path only; other renderers explicitly skipped. "
                    "Authored constant-distance maps; no shadow generation, all-face direction or seam proof, "
                    "partial spot-kernel weights, cone-edge interpolation, punctual specular or other-renderer evidence.",
})
