// SPDX-License-Identifier: MS-PL
// The stock vertex types and IVertexType: every vertex knows its own VertexDeclaration.
#include <array>
#include <any>
#include <initializer_list>
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
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
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
    bool HasElements(const VertexDeclaration& declaration, int stride,
                     std::initializer_list<VertexElement> expected)
    {
        return declaration.getVertexStrideProperty() == stride &&
               declaration.GetVertexElements() == std::vector<VertexElement>(expected);
    }

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
            ctx.Check(
                &pc.getVertexDeclarationProperty() == &VertexPositionColor::getVertexDeclarationStatic() &&
                    &pct.getVertexDeclarationProperty() ==
                        &VertexPositionColorTexture::getVertexDeclarationStatic() &&
                    &pnt.getVertexDeclarationProperty() ==
                        &VertexPositionNormalTexture::getVertexDeclarationStatic() &&
                    &pt.getVertexDeclarationProperty() == &VertexPositionTexture::getVertexDeclarationStatic(),
                "each IVertexType property returns its type's stable static declaration");
            ctx.Check(
                HasElements(VertexPositionColor::getVertexDeclarationStatic(), 16,
                            {VertexElement(0, VertexElementFormat::Vector3,
                                           VertexElementUsage::Position, 0),
                             VertexElement(12, VertexElementFormat::Color,
                                           VertexElementUsage::Color, 0)}) &&
                    HasElements(VertexPositionColorTexture::getVertexDeclarationStatic(), 24,
                                {VertexElement(0, VertexElementFormat::Vector3,
                                               VertexElementUsage::Position, 0),
                                 VertexElement(12, VertexElementFormat::Color,
                                               VertexElementUsage::Color, 0),
                                 VertexElement(16, VertexElementFormat::Vector2,
                                               VertexElementUsage::TextureCoordinate, 0)}) &&
                    HasElements(VertexPositionNormalTexture::getVertexDeclarationStatic(), 32,
                                {VertexElement(0, VertexElementFormat::Vector3,
                                               VertexElementUsage::Position, 0),
                                 VertexElement(12, VertexElementFormat::Vector3,
                                               VertexElementUsage::Normal, 0),
                                 VertexElement(24, VertexElementFormat::Vector2,
                                               VertexElementUsage::TextureCoordinate, 0)}) &&
                    HasElements(VertexPositionTexture::getVertexDeclarationStatic(), 20,
                                {VertexElement(0, VertexElementFormat::Vector3,
                                               VertexElementUsage::Position, 0),
                                 VertexElement(12, VertexElementFormat::Vector2,
                                               VertexElementUsage::TextureCoordinate, 0)}),
                "the four static declarations expose their exact packed stream elements");

            const Vector3 position(1.0f, 2.0f, 3.0f);
            const Vector3 normal(0.0f, 1.0f, 0.0f);
            const Vector2 textureCoordinate(4.0f, 5.0f);
            const Color color(10, 20, 30, 40);
            const VertexPositionColor pcValue(position, color);
            const VertexPositionColorTexture pctValue(position, color, textureCoordinate);
            const VertexPositionNormalTexture pntValue(position, normal, textureCoordinate);
            const VertexPositionTexture ptValue(position, textureCoordinate);
            ctx.Check(
                pcValue.Position == position && pcValue.Color == color &&
                    pctValue.Position == position && pctValue.Color == color &&
                    pctValue.TextureCoordinate == textureCoordinate &&
                    pntValue.Position == position && pntValue.Normal == normal &&
                    pntValue.TextureCoordinate == textureCoordinate &&
                    ptValue.Position == position && ptValue.TextureCoordinate == textureCoordinate,
                "the stock constructors expose every authored public field");

            VertexPositionColor pcDifferent = pcValue;
            VertexPositionColorTexture pctDifferent = pctValue;
            VertexPositionNormalTexture pntDifferent = pntValue;
            VertexPositionTexture ptDifferent = ptValue;
            pcDifferent.Color = Color::White;
            pctDifferent.TextureCoordinate = Vector2::Zero;
            pntDifferent.Normal = Vector3::UnitZ;
            ptDifferent.Position = Vector3::Zero;
            ctx.Check(
                pcValue == VertexPositionColor(pcValue) && pcValue.Equals(VertexPositionColor(pcValue)) &&
                    pcValue.Equals(std::any(VertexPositionColor(pcValue))) && pcValue != pcDifferent &&
                    pctValue == VertexPositionColorTexture(pctValue) &&
                    pctValue.Equals(VertexPositionColorTexture(pctValue)) &&
                    pctValue.Equals(std::any(VertexPositionColorTexture(pctValue))) &&
                    pctValue != pctDifferent &&
                    pntValue == VertexPositionNormalTexture(pntValue) &&
                    pntValue.Equals(VertexPositionNormalTexture(pntValue)) &&
                    pntValue.Equals(std::any(VertexPositionNormalTexture(pntValue))) &&
                    pntValue != pntDifferent && ptValue == VertexPositionTexture(ptValue) &&
                    ptValue.Equals(VertexPositionTexture(ptValue)) &&
                    ptValue.Equals(std::any(VertexPositionTexture(ptValue))) &&
                    ptValue != ptDifferent && !pcValue.Equals(std::any(ptValue)),
                "equality observes every field and boxed equality rejects another stock type");
            ctx.Check(
                pcValue.GetHashCode() == VertexPositionColor(pcValue).GetHashCode() &&
                    pctValue.GetHashCode() == VertexPositionColorTexture(pctValue).GetHashCode() &&
                    pntValue.GetHashCode() == VertexPositionNormalTexture(pntValue).GetHashCode() &&
                    ptValue.GetHashCode() == VertexPositionTexture(ptValue).GetHashCode(),
                "equal stock vertex values have equal hashes");
            ctx.Check(
                pcValue.ToString() ==
                        "{Position:{X:1 Y:2 Z:3} Color:{R:10 G:20 B:30 A:40}}" &&
                    pctValue.ToString() ==
                        "{Position:{X:1 Y:2 Z:3} Color:{R:10 G:20 B:30 A:40} "
                        "TextureCoordinate:{X:4 Y:5}}" &&
                    pntValue.ToString() ==
                        "{Position:{X:1 Y:2 Z:3} Normal:{X:0 Y:1 Z:0} "
                        "TextureCoordinate:{X:4 Y:5}}" &&
                    ptValue.ToString() ==
                        "{Position:{X:1 Y:2 Z:3} TextureCoordinate:{X:4 Y:5}}",
                "all four stock values use XNA's exact field-labelled text");

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
    .apis = {"IVertexType", "IVertexType::VertexDeclaration",
             "VertexPositionColor::VertexPositionColor", "VertexPositionColor::Position",
             "VertexPositionColor::Color", "VertexPositionColor::VertexDeclaration",
             "VertexPositionColor::Equals", "VertexPositionColor::op_Equality",
             "VertexPositionColor::op_Inequality", "VertexPositionColor::GetHashCode",
             "VertexPositionColor::ToString",
             "VertexPositionColorTexture::VertexPositionColorTexture",
             "VertexPositionColorTexture::Position", "VertexPositionColorTexture::Color",
             "VertexPositionColorTexture::TextureCoordinate",
             "VertexPositionColorTexture::VertexDeclaration", "VertexPositionColorTexture::Equals",
             "VertexPositionColorTexture::op_Equality", "VertexPositionColorTexture::op_Inequality",
             "VertexPositionColorTexture::GetHashCode", "VertexPositionColorTexture::ToString",
             "VertexPositionNormalTexture::VertexPositionNormalTexture",
             "VertexPositionNormalTexture::Position", "VertexPositionNormalTexture::Normal",
             "VertexPositionNormalTexture::TextureCoordinate",
             "VertexPositionNormalTexture::VertexDeclaration", "VertexPositionNormalTexture::Equals",
             "VertexPositionNormalTexture::op_Equality", "VertexPositionNormalTexture::op_Inequality",
             "VertexPositionNormalTexture::GetHashCode", "VertexPositionNormalTexture::ToString",
             "VertexPositionTexture::VertexPositionTexture", "VertexPositionTexture::Position",
             "VertexPositionTexture::TextureCoordinate", "VertexPositionTexture::VertexDeclaration",
             "VertexPositionTexture::Equals", "VertexPositionTexture::op_Equality",
             "VertexPositionTexture::op_Inequality", "VertexPositionTexture::GetHashCode",
             "VertexPositionTexture::ToString", "VertexDeclaration::VertexStride",
             "VertexDeclaration::GetVertexElements", "GraphicsDevice::DrawUserPrimitives",
             "PrimitiveType::TriangleStrip"},
    .tags = {"3d", "vertex", "declaration", "texture"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
