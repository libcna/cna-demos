// SPDX-License-Identifier: MS-PL
// Authored cascade slices distinguish split selection, blending and the fitting camera.
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
#include "Microsoft/Xna/Framework/Graphics/ShadowCascadeStateEXT.hpp"
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
    class ShadowCascadesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer != "OPENGLES3" && renderer != "OPENGL33")
                Context().Skip("this shadow-cascade example requires the EasyGL stock shadow-sampling path");
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
            // The single-map fallback selects u=.25,v=.5,depth=.5 independently of receiver position.
            effect_->setLightViewProjectionEXT(Matrix::CreateScale(0, 0, 0) * Matrix::CreateTranslation(-.5f, 0, 0));
            effect_->setShadowMapEXT(shadow_.get());
            effect_->setShadowsEnabledEXT(true);
            effect_->setShadowDepthBiasEXT(0);
            effect_->setShadowFilterRadiusEXT(0); // each panel below selects its comparison kernel
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            Context().Print("Two authored atlas slices store depths64/255 and192/255. Split depth3 selects the first slice inclusively; depth4 selects the second. A one-unit band blends at depth2.5. A separate fitting camera changes cascade selection without moving the rendering camera. Count0 restores the low-depth single-map lookup. Two panels in the last row send both cascades to the exact same atlas UV(.5,.5); each selected slice must keep radius2 taps in its own region. The final cell uses four horizontal strips: fitting depth7 selects slot3; its comparison depth changes from .5 to .9 and enables the yellow diagnostic tint.");
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
            const std::array<float, 8> depths{2, 3, 4, 2.5f, 4, 4, 2, 4};
            for (int panel = 0; panel < 8; ++panel)
            {
                const int column = panel % 3, row = panel / 3;
                const int left = full.getWidthProperty() * column / 3;
                const int right = full.getWidthProperty() * (column + 1) / 3;
                device.setViewportProperty(Viewport(left, row * full.getHeightProperty() / 3,
                                                   right - left, full.getHeightProperty() / 3));
                effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -depths[panel])});
                ShadowCascadeStateEXT cascades;
                cascades.Count = panel == 5 ? 0 : 2;
                // Cascade matrices already map world positions into normalized atlas space.
                cascades.WorldToAtlas[0] = Matrix::CreateScale(.5f, .5f, 0) * Matrix::CreateTranslation(.25f, .5f, .5f);
                cascades.WorldToAtlas[1] = Matrix::CreateScale(.5f, .5f, 0) * Matrix::CreateTranslation(.75f, .5f, .5f);
                if (panel >= 6)
                {
                    // The exact shared atlas edge belongs to either selected slice. Each slice
                    // must constrain its own taps even though both matrices produce the same UV.
                    cascades.WorldToAtlas[0] = Matrix::CreateScale(0, 0, 0) * Matrix::CreateTranslation(.5f, .5f, .5f);
                    cascades.WorldToAtlas[1] = cascades.WorldToAtlas[0];
                }
                effect_->setShadowFilterRadiusEXT(panel >= 6 ? 2 : 0);
                cascades.SplitDistance[0] = 3;
                cascades.SplitDistance[1] = 10;
                cascades.CameraView = panel == 4 ? Matrix::CreateTranslation(0, 0, 2) : Matrix::getIdentityProperty();
                cascades.BlendBand = panel == 3 ? 1.0f : 0.0f;
                effect_->setShadowCascadesEXT(cascades);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            // Use the remaining cell for the last slot at the four-cascade limit. CNA
            // packs cascades as horizontal strips, not a 2x2 atlas. The first three slots
            // compare at depth .9; slot3 changes from .5 (lit) to .9 (shadowed and tinted).
            const int fourthLeft = full.getWidthProperty() * 2 / 3;
            const int fourthWidth = full.getWidthProperty() - fourthLeft;
            for (int variant = 0; variant < 2; ++variant)
            {
                const int left = fourthLeft + fourthWidth * variant / 2;
                const int right = fourthLeft + fourthWidth * (variant + 1) / 2;
                device.setViewportProperty(Viewport(left, full.getHeightProperty() * 2 / 3,
                                                   right - left, full.getHeightProperty() / 3));
                effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -7)});
                ShadowCascadeStateEXT cascades;
                cascades.Count = ShadowCascadeStateEXT::kMaxCascades;
                for (int slot = 0; slot < cascades.Count; ++slot)
                {
                    const float center = (static_cast<float>(slot) + .5f) / static_cast<float>(cascades.Count);
                    const float depth = slot == cascades.Count - 1 && variant == 0 ? .5f : .9f;
                    cascades.WorldToAtlas[slot] = Matrix::CreateScale(0, 0, 0) *
                        Matrix::CreateTranslation(center, .5f, depth);
                    cascades.SplitDistance[slot] = 2.0f * static_cast<float>(slot + 1);
                }
                cascades.CameraView = Matrix::getIdentityProperty();
                cascades.DebugTint = variant == 1;
                effect_->setShadowFilterRadiusEXT(0);
                effect_->setShadowCascadesEXT(cascades);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            device.setViewportProperty(full);
            const std::array<const char*, 8> labels{
                "Depth 2: near", "Depth 3: split", "Depth 4: far",
                "Depth 2.5: blend", "Fitting camera +2", "Count 0: single", "Near atlas edge", "Far atlas edge"};
            batch_->Begin();
            for (int panel = 0; panel < 8; ++panel)
                batch_->DrawString(*font_, labels[panel], Vector2(
                    static_cast<float>((panel % 3) * full.getWidthProperty() / 3 + 8),
                    static_cast<float>((panel / 3) * full.getHeightProperty() / 3 + 8)), Color::White);
            batch_->DrawString(*font_, "4th: lit/tinted", Vector2(
                static_cast<float>(full.getWidthProperty() * 2 / 3 + 8),
                static_cast<float>(full.getHeightProperty() * 2 / 3 + 8)), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Color lit(173, 130, 77), occluded(20, 28, 26), halfVisible(97, 79, 51);
                const std::array<Color, 8> expected{occluded, occluded, lit, halfVisible, occluded, occluded, occluded, lit};
                const std::array<const char*, 8> checks{
                    "view depth2 selects the occluded near atlas slice",
                    "the exact depth3 split remains in the near cascade",
                    "view depth4 selects the directly lit far atlas slice",
                    "a one-unit blend band gives half visibility at depth2.5",
                    "cascade selection follows the fitting camera independently of the rendering View",
                    "Count0 restores the separately supplied single-map transform",
                    "radius2 sampling at the near slice edge excludes the lit neighboring slice",
                    "radius2 sampling at the far slice edge excludes the occluded neighboring slice"};
                for (int panel = 0; panel < 8; ++panel)
                {
                    const int x = pixels.Width() * (2 * (panel % 3) + 1) / 6;
                    const int y = pixels.Height() * (2 * (panel / 3) + 1) / 6;
                    Context().Check(CnaDemos::Near(pixels.At(x, y), expected[panel], 1), checks[panel]);
                }
                const int lastLeft = pixels.Width() * 2 / 3;
                const int lastWidth = pixels.Width() - lastLeft;
                const int lastY = pixels.Height() * 5 / 6;
                Context().Check(CnaDemos::Near(pixels.At(lastLeft + lastWidth / 4, lastY), lit, 1),
                                "four cascades select the last slot and its lit depth at fitting depth7");
                // DebugTint for slot3 is (1,1,.6), applied to the unquantized ambient/emissive floor.
                Context().Check(CnaDemos::Near(pixels.At(lastLeft + lastWidth * 3 / 4, lastY), Color(20, 28, 15), 1),
                                "the last slot's changed depth becomes shadowed with its yellow debug tint");
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
CNA_DEMOS_GAME(ShadowCascadesDemo, {
    .id = "graphics.skinned_effect.shadow_cascades",
    .title = "Authored cascade atlas selection and sampling",
    .description = "Selects two authored shadow-atlas slices by fitting-camera depth, checks the inclusive split, "
                   "blends across its near side, restores the single-map path with Count0, isolates the shared atlas edge, and checks last-slot depth and debug tint at the four-cascade limit.",
    .apis = {"SkinnedEffect::SetBoneTransforms",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowMapEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowCascadesEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowFilterRadiusEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setLightViewProjectionEXT",
             "Microsoft::Xna::Framework::Graphics::ShadowCascadeStateEXT",
             "Microsoft::Xna::Framework::Graphics::ShadowCascadeStateEXT::kMaxCascades",
             "Microsoft::Xna::Framework::Graphics::ShadowCascadeStateEXT::DebugTint"},
    .tags = {"skinning", "shadows", "cascades", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "EasyGL OPENGLES3/OPENGL33 stock receiver path only; other renderers explicitly skipped. "
                    "Authored Color atlas, two-slice controls and the fourth slot at maximum count; no shadow-map generation, "
                    "arbitrary atlas layouts, all kernel weights, every four-slice selection/debug color or other-renderer proof.",
})
