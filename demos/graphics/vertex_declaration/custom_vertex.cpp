// SPDX-License-Identifier: MS-PL
// A vertex structure of your own: a plain struct described by a VertexDeclaration.
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // The vertex as the GPU will read it: position, a value the effect does not use, then colour.
    // In C++ the struct *is* the vertex stream, so it must hold plain data only. CNA's Color
    // implements IPackedVector with virtual functions (16 bytes, not XNA's 4), so the colour is
    // stored as its 32-bit packed value -- the exact bytes an XNA Color holds
    // (CNA-DEMOS-ISSUE-005). CNA's own VertexPositionColor is converted for you.
    struct GlowVertex
    {
        Vector3 Position;          // offset 0
        float Glow;                // offset 12 -- extra per-vertex data a custom shader could read
        std::uint32_t PackedTint;  // offset 16 -- Color::getPackedValueProperty()
    };
    static_assert(std::is_trivially_copyable_v<GlowVertex> && sizeof(GlowVertex) == 20,
                  "a plain 20-byte vertex: 12 + 4 + 4 bytes, no padding");

    // One VertexElement per field: byte offset, format, what it means (usage) and usage index.
    const VertexDeclaration& GlowVertexDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(GlowVertex, Position)), VertexElementFormat::Vector3,
                          VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(GlowVertex, Glow)), VertexElementFormat::Single,
                          VertexElementUsage::TextureCoordinate, 1),
            VertexElement(static_cast<int>(offsetof(GlowVertex, PackedTint)), VertexElementFormat::Color,
                          VertexElementUsage::Color, 0),
        };
        return declaration;
    }

    class CustomVertexDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));

            // A fan of triangles around the centre, as a triangle list.
            constexpr int Slices = 12;
            for (int i = 0; i < Slices; ++i)
            {
                const auto rim = [](int k) {
                    const float angle = MathHelper::TwoPi * static_cast<float>(k) / Slices;
                    return Vector3(400.0f + 200.0f * std::cos(angle), 240.0f + 200.0f * std::sin(angle), 0.0f);
                };
                const Color tint = i % 2 == 0 ? Color::Teal : Color::Coral;
                vertices_.push_back({Vector3(400.0f, 240.0f, 0.0f), 1.0f, Color::White.getPackedValueProperty()});
                vertices_.push_back({rim(i), 0.0f, tint.getPackedValueProperty()});
                vertices_.push_back({rim(i + 1), 0.0f, tint.getPackedValueProperty()});
            }

            const VertexDeclaration& declaration = GlowVertexDeclaration();
            Context().Print("VertexStride {} bytes, {} elements", declaration.getVertexStrideProperty(),
                            declaration.GetVertexElements().size());
            Context().Check(declaration.getVertexStrideProperty() == static_cast<int>(sizeof(GlowVertex)),
                            "the declaration's stride is the struct's size");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // The declaration tells the device how to read the structure. BasicEffect uses
                // Position and Color and ignores the extra element.
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices_, 0,
                                          static_cast<int>(vertices_.size()) / 3, GlowVertexDeclaration());
            }
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::vector<GlowVertex> vertices_;
    };
}

CNA_DEMOS_GAME(CustomVertexDemo, {
    .id = "graphics.vertex_declaration.custom_vertex",
    .title = "Custom vertex format",
    .description = "Defines a plain vertex struct with an extra field, describes it with VertexElements in a "
                   "VertexDeclaration and draws it with DrawUserPrimitives.",
    .apis = {"VertexDeclaration::VertexDeclaration", "VertexDeclaration::VertexStride",
             "VertexDeclaration::GetVertexElements", "VertexElement::VertexElement", "VertexElementFormat::Vector3",
             "VertexElementFormat::Single", "VertexElementFormat::Color", "VertexElementUsage::Position",
             "VertexElementUsage::Color", "VertexElementUsage::TextureCoordinate", "GraphicsDevice::DrawUserPrimitives"},
    .tags = {"3d", "vertex", "layout", "declaration"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "CNA-DEMOS-ISSUE-005: CNA's Color is not a 4-byte struct, so the vertex stores its packed value.",
})
