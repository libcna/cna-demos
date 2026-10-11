// SPDX-License-Identifier: MS-PL
// The stock vertex types and IVertexType: every vertex knows its own VertexDeclaration.
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IVertexType.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class StockVertexTypesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // Each stock vertex struct implements IVertexType: code that only has an IVertexType
            // (a generic buffer helper, say) can still ask for the layout.
            const VertexPositionColor pc;
            const VertexPositionColorTexture pct;
            const VertexPositionNormalTexture pnt;
            const VertexPositionTexture pt;
            const std::array<std::pair<const char*, const IVertexType*>, 4> types = {{
                {"VertexPositionColor", &pc},
                {"VertexPositionColorTexture", &pct},
                {"VertexPositionNormalTexture", &pnt},
                {"VertexPositionTexture", &pt},
            }};
            std::array<int, 4> strides{};
            for (std::size_t i = 0; i < types.size(); ++i)
            {
                const VertexDeclaration& declaration = types[i].second->getVertexDeclarationProperty();
                strides[i] = declaration.getVertexStrideProperty();
                ctx.Print("{:<28} stride {:>2} bytes, {} elements", types[i].first, strides[i],
                          declaration.GetVertexElements().size());
            }
            // Position 12 bytes, Color 4, Normal 12, TextureCoordinate 8.
            ctx.Check(strides == std::array<int, 4>{16, 24, 32, 20}, "strides 16, 24, 32 and 20 bytes, as in XNA");

            // VertexPositionColorTexture: a textured quad whose corners also carry a tint.
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setTextureEnabledProperty(true);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            effect_->setTextureProperty(&checker_);
            quad_ = {
                VertexPositionColorTexture(Vector3(200.0f, 80.0f, 0.0f), Color::White, Vector2(0.0f, 0.0f)),
                VertexPositionColorTexture(Vector3(600.0f, 80.0f, 0.0f), Color::Orange, Vector2(4.0f, 0.0f)),
                VertexPositionColorTexture(Vector3(200.0f, 400.0f, 0.0f), Color::DeepSkyBlue, Vector2(0.0f, 3.0f)),
                VertexPositionColorTexture(Vector3(600.0f, 400.0f, 0.0f), Color::LimeGreen, Vector2(4.0f, 3.0f)),
            };
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(26, 28, 38));
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.getSamplerStatesProperty()(0, SamplerState::PointWrap); // UVs beyond 1 repeat the checker
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // The vertex type supplies its own declaration: no VertexDeclaration argument.
                device.DrawUserPrimitives(PrimitiveType::TriangleStrip, quad_.data(), 0, 2);
            }
            if (FramesDrawn() == 0)
            {
                // On white checker squares the vertex colour shows through: white near the top-left
                // corner, orange near the top-right one (UV 3.8 lands on a white square).
                const CnaDemos::BackBufferPixels pixels(device);
                const Color left = pixels.At(205, 85);
                const Color right = pixels.At(580, 85);
                Context().Print("near the white corner {}, near the orange corner {}", left.ToString(), right.ToString());
                Context().Check(left.getBProperty() > 200 && right.getBProperty() < 100 && right.getRProperty() > 200,
                                "texture and vertex colour combine per corner");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        Texture2D checker_;
        std::array<VertexPositionColorTexture, 4> quad_;
    };
}

CNA_DEMOS_GAME(StockVertexTypesDemo, {
    .id = "graphics.vertex_declaration.stock_vertex_types",
    .title = "The stock vertex types",
    .description = "Asks the four stock vertex structs for their VertexDeclaration through IVertexType (strides 16, "
                   "24, 32, 20) and draws a quad of VertexPositionColorTexture -- a repeating checker texture tinted "
                   "by each corner's colour -- with DrawUserPrimitives and no explicit declaration.",
    .apis = {"IVertexType", "IVertexType::VertexDeclaration", "VertexPositionColorTexture",
             "VertexPositionColorTexture::VertexPositionColorTexture", "VertexDeclaration::VertexStride",
             "VertexDeclaration::GetVertexElements", "GraphicsDevice::DrawUserPrimitives", "PrimitiveType::TriangleStrip"},
    .tags = {"3d", "vertex", "declaration", "texture"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
