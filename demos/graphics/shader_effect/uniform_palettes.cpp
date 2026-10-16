// SPDX-License-Identifier: MS-PL
// Two custom transform/color palette entries, then updated uniforms on the same program.
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct PaletteVertex { float x, y, z, palette; };
    static_assert(std::is_standard_layout_v<PaletteVertex> && std::is_trivially_copyable_v<PaletteVertex>);
    constexpr const char* Vertex = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aPalette;
uniform mat4 transforms[2];
uniform mat4 sceneTransform;
uniform vec3 paletteColors[2];
out vec3 tint;
void main() {
    int slot = int(aPalette + 0.5);
    gl_Position = sceneTransform * transforms[slot] * vec4(aPosition, 1.0);
    tint = paletteColors[slot];
}
)";
    constexpr const char* Fragment = R"(
in vec3 tint;
uniform vec4 sceneTint;
out vec4 FragColor;
void main() { FragColor = vec4(tint * sceneTint.rgb, 1.0) * sceneTint.a; }
)";
    class UniformPaletteDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto dialect = device.GetShaderDialectEXT();
            const bool es = dialect == CNA::ShaderDialectEXT::GlslEs;
            if ((!es && dialect != CNA::ShaderDialectEXT::GlslDesktop) || !device.ExecutesShaderEffectSourceEXT())
                Context().Skip("the palette demo needs executable GLSL source");
            const std::string header = es ? "#version 300 es\nprecision highp float;\n" : "#version 330 core\n";
            shader_ = std::make_unique<ShaderEffect>(device, header + Vertex, header + Fragment);
            if (!Context().Check(shader_->IsEffectValid(), "the custom transform and color array shader compiles"))
                throw std::runtime_error(shader_->GetCompileErrorEXT());
            declaration_.emplace(static_cast<int>(sizeof(PaletteVertex)), std::vector<VertexElement>{
                VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
                VertexElement(static_cast<int>(offsetof(PaletteVertex, palette)), VertexElementFormat::Single,
                              VertexElementUsage::TextureCoordinate, 0)});
            const std::array<PaletteVertex, 6> vertices{{
                {-.3f, -.6f, 0, 0}, {.3f, -.6f, 0, 0}, {0, .6f, 0, 0},
                {-.3f, -.6f, 0, 1}, {.3f, -.6f, 0, 1}, {0, .6f, 0, 1}}};
            vertices_ = std::make_unique<VertexBuffer>(device, *declaration_, 6, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 6);
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto full = device.getViewportProperty();
            const int width = full.getWidthProperty(), rowHeight = full.getHeightProperty() / 3;
            const Matrix left = Matrix::CreateTranslation(-.5f, 0, 0);
            const Matrix right = Matrix::CreateTranslation(.5f, 0, 0);
            // Each group is M11..M44, matching SetUniformMat4's public payload convention.
            // Do not assume two C++ Matrix objects have a contiguous 32-float representation.
            const std::array<float, 32> transforms{
                left.M11,left.M12,left.M13,left.M14, left.M21,left.M22,left.M23,left.M24,
                left.M31,left.M32,left.M33,left.M34, left.M41,left.M42,left.M43,left.M44,
                right.M11,right.M12,right.M13,right.M14, right.M21,right.M22,right.M23,right.M24,
                right.M31,right.M32,right.M33,right.M34, right.M41,right.M42,right.M43,right.M44};
            device.Clear(Color::Black);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            for (int row = 0; row < 3; ++row)
            {
                auto matrices = transforms;
                std::array<float, 6> colors{1, 0, 0, 0, 1, 0};
                if (row == 1)
                {
                    matrices[12] = .5f; matrices[28] = -.5f; // swap the two M41 translations
                    colors = {0, 1, 1, 1, 0, 1}; // cyan slot0, magenta slot1
                }
                // The scene transform is a parent of both per-object palette transforms.
                // Row-vector XNA composition scales first, then translates; upload M11..M44 explicitly.
                const Matrix scene = row == 2 ? Matrix::CreateScale(.5f, .5f, 1) *
                                                Matrix::CreateTranslation(0, .2f, 0) : Matrix::getIdentityProperty();
                std::array<float, 16> sceneValues{
                    scene.M11,scene.M12,scene.M13,scene.M14, scene.M21,scene.M22,scene.M23,scene.M24,
                    scene.M31,scene.M32,scene.M33,scene.M34, scene.M41,scene.M42,scene.M43,scene.M44};
                if (row == 2) colors = {0, 1, 1, 1, 0, 1};
                device.setBlendStateProperty(row == 2 ? BlendState::AlphaBlend : BlendState::Opaque);
                device.setViewportProperty(Viewport(0, row * rowHeight, width, rowHeight));
                // The first panel deliberately uploads before Apply. Each public setter must make
                // this effect's program current itself; otherwise the values are lost or written
                // to whichever program a prior draw happened to leave bound.
                if (row != 0) shader_->Apply();
                shader_->SetUniformMat4("sceneTransform", sceneValues.data()); // one parent matrix
                if (row == 2) shader_->SetUniformVec4("sceneTint", .5f, .25f, 1, .5f);
                else shader_->SetUniformVec4("sceneTint", 1, 1, 1, 1);
                shader_->SetUniformMat4Array("transforms", matrices.data(), 2); // two matrices, not 32 floats
                shader_->SetUniformVec3Array("paletteColors", colors.data(), 2); // two vectors, not 6 floats
                if (row == 0)
                {
                    const std::array<float, 32> ignoredMatrices{};
                    const std::array<float, 6> ignoredColors{};
                    shader_->SetUniformMat4Array("transforms", ignoredMatrices.data(), 0);
                    shader_->SetUniformVec3Array("paletteColors", ignoredColors.data(), 0);
                    shader_->SetUniformVec4("missingSceneTint", 0, 0, 0, 0);
                }
                sceneValues.fill(0); matrices.fill(0); colors.fill(0); // caller storage can change after the upload
                if (row == 0) shader_->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
            }
            device.setViewportProperty(full);
            batch_->Begin();
            batch_->DrawString(*font_, "Initial transform and color palettes", Vector2(40, 12), Color::White);
            batch_->DrawString(*font_, "Swapped transforms and updated colors", Vector2(40, static_cast<float>(rowHeight + 12)), Color::White);
            batch_->DrawString(*font_, "Shared scene transform and premultiplied tint",
                               Vector2(40, static_cast<float>(rowHeight * 2 + 12)), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels screen(device);
                Context().Check(CnaDemos::Near(screen.At(width / 4, rowHeight / 2), Color::Red, 0) &&
                                CnaDemos::Near(screen.At(width * 3 / 4, rowHeight / 2), Color::Lime, 0),
                                "pre-Apply values survive caller-buffer changes, zero-count updates and a missing uniform name");
                Context().Check(CnaDemos::Near(screen.At(width / 4, rowHeight * 3 / 2), Color::Magenta, 0) &&
                                CnaDemos::Near(screen.At(width * 3 / 4, rowHeight * 3 / 2), Color::Cyan, 0),
                                "updating the same program swaps the two matrix translations and replaces both color entries");
                Context().Check(CnaDemos::Near(screen.At(width / 2, rowHeight / 2), Color::Black, 0) &&
                                CnaDemos::Near(screen.At(width / 2, rowHeight * 3 / 2), Color::Black, 0),
                                "the translated triangle pairs leave the center gap unchanged in both viewports");
                const int sceneY = rowHeight * 2 + rowHeight * 2 / 5;
                // Parent scaling moves centers from +/-0.5 to +/-0.25; +0.2 Y moves them upwards.
                // Cyan/magenta times RGB(.5,.25,1), then premultiplied alpha .5 over black.
                Context().Check(CnaDemos::Near(screen.At(width * 3 / 8, sceneY), Color(0, 32, 128), 1) &&
                                CnaDemos::Near(screen.At(width * 5 / 8, sceneY), Color(64, 0, 128), 1),
                                "the shared scalar matrix composes after both palette transforms and all tint channels affect the blended colors");
                Context().Check(CnaDemos::Near(screen.At(width / 4, sceneY), Color::Black, 0) &&
                                CnaDemos::Near(screen.At(width * 3 / 4, sceneY), Color::Black, 0) &&
                                CnaDemos::Near(screen.At(width / 2, sceneY), Color::Black, 0),
                                "the scene transform vacates both original object centers and retains their new center gap");
            }
            DemoGame::Draw(time);
        }
        std::unique_ptr<ShaderEffect> shader_;
        std::optional<VertexDeclaration> declaration_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(UniformPaletteDemo, {
    .id = "graphics.shader_effect.uniform_palettes",
    .title = "Custom shader transform and color arrays",
    .description = "Uploads scalar and array uniforms before the first Apply, changes caller storage, then updates the "
                   "same program. A shared scalar matrix and premultiplied tint compose over both palette entries.",
    .apis = {"ShaderEffect::SetUniformMat4Array", "ShaderEffect::SetUniformVec3Array",
             "ShaderEffect::SetUniformMat4", "ShaderEffect::SetUniformVec4", "Effect::Apply",
             "Matrix::CreateTranslation", "GraphicsDevice::DrawPrimitives"},
    .tags = {"shader", "uniform", "array", "matrix", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL only. Vec3/mat4 zero-count updates and one missing scalar name are ignored; negative "
                    "C++ pointer counts, uniform blocks and other renderer dialects are not inferred.",
})
