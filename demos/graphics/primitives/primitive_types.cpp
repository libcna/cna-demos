// SPDX-License-Identifier: MS-PL
// PrimitiveType: the same six vertices read as a triangle list, triangle strip, line list and line strip.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct Panel
    {
        const char* name;
        PrimitiveType type;
        int primitiveCount;  // how many primitives six vertices make for this type
    };

    constexpr std::array<Panel, 4> Panels = {{
        {"TriangleList\n2 triangles", PrimitiveType::TriangleList, 2},
        {"TriangleStrip\n4 triangles", PrimitiveType::TriangleStrip, 4},
        {"LineList\n3 lines", PrimitiveType::LineList, 3},
        {"LineStrip\n5 lines", PrimitiveType::LineStrip, 5},
    }};

    class PrimitiveTypesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // An orthographic projection that maps world units to back-buffer pixels (y down).
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            // Triangles of a strip alternate their winding; draw both sides.
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            for (std::size_t p = 0; p < Panels.size(); ++p)
            {
                // Six vertices on a zigzag: top, bottom, top, bottom, ...
                const float left = 30.0f + static_cast<float>(p) * 195.0f;
                std::array<VertexPositionColor, 6> zigzag{};
                for (int i = 0; i < 6; ++i)
                {
                    const Color color = i % 2 == 0 ? Color::Orange : Color::DeepSkyBlue;
                    zigzag[static_cast<std::size_t>(i)] = VertexPositionColor(
                        Vector3(left + static_cast<float>(i) * 32.0f, i % 2 == 0 ? 150.0f : 330.0f, 0.0f), color);
                }

                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(Panels[p].type, zigzag.data(), 0, Panels[p].primitiveCount);
                }
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            for (std::size_t p = 0; p < Panels.size(); ++p)
                spriteBatch_->DrawString(*font_, Panels[p].name,
                                         Vector2(30.0f + static_cast<float>(p) * 195.0f, 360.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(PrimitiveTypesDemo, {
    .id = "graphics.primitives.primitive_types",
    .title = "Primitive types",
    .description = "Draws the same six zigzag vertices as PrimitiveType::TriangleList, TriangleStrip, LineList and "
                   "LineStrip, with the primitive count each type makes of them.",
    .apis = {"PrimitiveType::TriangleList", "PrimitiveType::TriangleStrip", "PrimitiveType::LineList",
             "PrimitiveType::LineStrip", "GraphicsDevice::DrawUserPrimitives", "Matrix::CreateOrthographicOffCenter"},
    .tags = {"3d", "primitives", "lines", "strip"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
