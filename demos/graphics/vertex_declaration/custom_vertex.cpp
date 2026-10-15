// SPDX-License-Identifier: MS-PL
// A vertex structure of your own: a plain struct described by a VertexDeclaration.
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "System/ObjectDisposedException.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
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

    struct FormatInventoryVertex
    {
        std::array<std::uint8_t, 48> Bytes;
        bool operator==(const FormatInventoryVertex&) const = default;
    };
    static_assert(std::is_trivially_copyable_v<FormatInventoryVertex> && sizeof(FormatInventoryVertex) == 48);

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
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
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
            const auto& elements = declaration.GetVertexElements();
            Context().Print("VertexStride {} bytes, {} elements", declaration.getVertexStrideProperty(),
                            elements.size());
            Context().Check(declaration.getVertexStrideProperty() == static_cast<int>(sizeof(GlowVertex)),
                            "the declaration's stride is the struct's size");

            Context().Check(elements.size() == 3 &&
                                elements[0].getOffsetProperty() == static_cast<int>(offsetof(GlowVertex, Position)) &&
                                elements[1].getOffsetProperty() == static_cast<int>(offsetof(GlowVertex, Glow)) &&
                                elements[2].getOffsetProperty() == static_cast<int>(offsetof(GlowVertex, PackedTint)),
                            "element offsets match the three struct fields");
            Context().Check(elements[0].getVertexElementFormatProperty() == VertexElementFormat::Vector3 &&
                                elements[1].getVertexElementFormatProperty() == VertexElementFormat::Single &&
                                elements[2].getVertexElementFormatProperty() == VertexElementFormat::Color,
                            "element formats match the bytes stored in each field");
            Context().Check(elements[0].getVertexElementUsageProperty() == VertexElementUsage::Position &&
                                elements[1].getVertexElementUsageProperty() == VertexElementUsage::TextureCoordinate &&
                                elements[2].getVertexElementUsageProperty() == VertexElementUsage::Color &&
                                elements[0].getUsageIndexProperty() == 0 &&
                                elements[1].getUsageIndexProperty() == 1 &&
                                elements[2].getUsageIndexProperty() == 0,
                            "usage and usage index identify Position0, TextureCoordinate1 and Color0");

            const std::vector<VertexElement> authored{
                VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
                VertexElement(12, VertexElementFormat::Single, VertexElementUsage::TextureCoordinate, 1),
                VertexElement(16, VertexElementFormat::Color, VertexElementUsage::Color, 0),
            };
            Context().Check(elements == authored && elements[1].Equals(authored[1]) &&
                                elements[1].GetHashCode() == authored[1].GetHashCode(),
                            "equal element values compare equal and have the same hash");

            VertexElement edited;
            edited.setOffsetProperty(12);
            edited.setVertexElementFormatProperty(VertexElementFormat::Single);
            edited.setVertexElementUsageProperty(VertexElementUsage::TextureCoordinate);
            edited.setUsageIndexProperty(1);
            Context().Check(edited == elements[1] && edited != elements[0] &&
                                edited.ToString() ==
                                    "{Offset:12 Format:Single Usage:TextureCoordinate UsageIndex:1}",
                            "a mutable element value round-trips all four fields and describes itself");

            const std::vector<VertexElement> formatInventory{
                VertexElement(0, VertexElementFormat::Byte4, VertexElementUsage::BlendIndices, 0),
                VertexElement(4, VertexElementFormat::HalfVector2, VertexElementUsage::Binormal, 0),
                VertexElement(8, VertexElementFormat::HalfVector4, VertexElementUsage::BlendWeight, 0),
                VertexElement(16, VertexElementFormat::NormalizedShort2, VertexElementUsage::Depth, 0),
                VertexElement(20, VertexElementFormat::NormalizedShort4, VertexElementUsage::Fog, 0),
                VertexElement(28, VertexElementFormat::Short2, VertexElementUsage::PointSize, 0),
                VertexElement(32, VertexElementFormat::Short4, VertexElementUsage::Sample, 0),
                VertexElement(40, VertexElementFormat::Vector2, VertexElementUsage::TessellateFactor, 0),
            };
            VertexDeclaration inventoryDeclaration(formatInventory);
            Context().Check(inventoryDeclaration.getVertexStrideProperty() == 48 &&
                                inventoryDeclaration.GetVertexElements() == formatInventory,
                            "all compact formats contribute their exact size and preserve every authored usage");

            FormatInventoryVertex input{};
            for (std::size_t i = 0; i < input.Bytes.size(); ++i)
                input.Bytes[i] = static_cast<std::uint8_t>(i * 5 + 3);
            VertexBuffer inventoryBuffer(device, inventoryDeclaration, 1, BufferUsage::None);
            inventoryBuffer.SetData(&input, 1);
            FormatInventoryVertex output{};
            inventoryBuffer.GetData(&output, 1);
            Context().Check(output == input &&
                                inventoryBuffer.getVertexDeclarationProperty().GetVertexElements() == formatInventory,
                            "a readable HiDef buffer accepts the complete compact declaration and preserves its bytes");

            VertexDeclaration disposed(authored);
            disposed.Dispose();
            disposed.Dispose();
            bool refused = false;
            try
            {
                VertexBuffer unused(device, disposed, 1, BufferUsage::None);
            }
            catch (const System::ObjectDisposedException&)
            {
                refused = true;
            }
            Context().Check(disposed.getIsDisposedProperty() && refused,
                            "disposed declarations are idempotently marked and cannot bind a vertex buffer");
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
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Color centre = pixels.At(400, 240);
                Context().Check(centre.getRProperty() > 240 && centre.getGProperty() > 240 &&
                                    centre.getBProperty() > 240 && pixels.At(10, 10) == Color::Black,
                                "the declared Position and Color fields draw the white centre on a black exterior");
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
             "VertexDeclaration::GetVertexElements", "VertexElement::VertexElement", "VertexElement::Offset",
             "VertexElement::VertexElementFormat", "VertexElement::VertexElementUsage", "VertexElement::UsageIndex",
             "VertexElement::Equals", "VertexElement::op_Equality", "VertexElement::op_Inequality",
             "VertexElement::GetHashCode", "VertexElement::ToString", "VertexElementFormat::Vector3",
             "VertexElementFormat::Single", "VertexElementFormat::Color", "VertexElementUsage::Position",
             "VertexElementUsage::Color", "VertexElementUsage::TextureCoordinate", "VertexDeclaration::Dispose",
             "GraphicsDevice::DrawUserPrimitives", "VertexElementFormat::HalfVector2",
             "VertexElementFormat::HalfVector4", "VertexElementFormat::NormalizedShort2",
             "VertexElementFormat::NormalizedShort4", "VertexElementFormat::Short2", "VertexElementFormat::Short4",
             "VertexElementUsage::Binormal", "VertexElementUsage::Depth", "VertexElementUsage::Fog",
             "VertexElementUsage::PointSize", "VertexElementUsage::Sample", "VertexElementUsage::TessellateFactor"},
    .tags = {"3d", "vertex", "layout", "declaration"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "CNA-DEMOS-ISSUE-005: CNA's Color is not a 4-byte struct, so the vertex stores its packed value.",
})
