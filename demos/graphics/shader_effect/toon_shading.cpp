// SPDX-License-Identifier: MS-PL
// CNAEXT ShaderEffect: your own GLSL vertex and fragment shaders -- here cel ("toon") shading.
#include <memory>
#include <set>
#include <string>

#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::ShaderDialectEXT;

namespace
{
    // The vertex attributes CNA's GL renderers bind for VertexPositionNormalTexture, and the
    // World/View/Projection uniforms ShaderEffect fills from its IEffectMatrices properties.
    constexpr const char* VertexBody = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
uniform mat4 World;
uniform mat4 View;
uniform mat4 Projection;
out vec3 vNormal;
void main()
{
    gl_Position = Projection * View * World * vec4(aPosition, 1.0);
    vNormal = mat3(World) * aNormal;
}
)";

    // Cel shading: the lighting is quantised into four flat bands instead of a smooth gradient.
    constexpr const char* FragmentBody = R"(
in vec3 vNormal;
out vec4 FragColor;
uniform vec3 uLightDirection;   // the direction the light travels
uniform vec3 uColor;
void main()
{
    float d = max(dot(normalize(vNormal), -uLightDirection), 0.0);
    float band = d > 0.8 ? 1.0 : d > 0.45 ? 0.7 : d > 0.15 ? 0.45 : 0.2;
    FragColor = vec4(uColor * band, 1.0);
}
)";

    class ToonShadingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // A ShaderEffect is renderer-specific source text: ask which dialect this renderer wants.
            const ShaderDialectEXT dialect = device.GetShaderDialectEXT();
            std::string header;
            if (dialect == ShaderDialectEXT::GlslEs)
                header = "#version 300 es\nprecision mediump float;\n";
            else if (dialect == ShaderDialectEXT::GlslDesktop)
                header = "#version 330 core\n";
            else
                Context().Skip("this demo carries GLSL; the renderer wants another shading language");
            // Some renderers accept any source and draw with their own fixed path.
            if (!device.ExecutesShaderEffectSourceEXT())
                Context().Skip("this renderer accepts shader source but does not execute it");

            effect_ = std::make_unique<ShaderEffect>(device, header + VertexBody, header + FragmentBody);
            Context().Check(effect_->IsEffectValid(), "the GLSL compiled and linked");
            if (!effect_->IsEffectValid())
                Context().Print("compiler log:\n{}", effect_->GetCompileErrorEXT());

            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 4.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            const CnaDemos::Mesh sphere = CnaDemos::MakeSphere(1.2f);
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionNormalTexture::getVertexDeclarationStatic(),
                                                       sphere.VertexCount(), BufferUsage::WriteOnly);
            vertices_->SetData(sphere.vertices.data(), sphere.VertexCount());
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits,
                                                     static_cast<int>(sphere.indices.size()), BufferUsage::WriteOnly);
            indices_->SetData(sphere.indices.data(), static_cast<int>(sphere.indices.size()));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            // A convex sphere needs no depth buffer, only back-face culling.
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // Apply binds the program; uniforms are set on it afterwards.
            effect_->Apply();
            const Vector3 light = Vector3::Normalize(Vector3(-0.6f, -0.5f, -0.8f));
            effect_->SetUniformVec3("uLightDirection", light.X, light.Y, light.Z);
            effect_->SetUniformVec3("uColor", 1.0f, 0.55f, 0.1f);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, vertices_->getVertexCountProperty(), 0,
                                         indices_->getIndexCountProperty() / 3);

            if (FramesDrawn() == 0)
            {
                // A smooth shader gives hundreds of shades; the toon shader exactly four.
                const CnaDemos::BackBufferPixels p(device);
                std::set<std::uint32_t> shades;
                for (int y = 0; y < p.Height(); y += 2)
                    for (int x = 0; x < p.Width(); x += 2)
                        if (p.At(x, y) != Color::Black)
                            shades.insert(p.At(x, y).getPackedValueProperty());
                Context().Print("distinct colours on the sphere: {}", shades.size());
                Context().Check(shades.size() == 4, "four flat bands of shading");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<ShaderEffect> effect_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
    };
}

CNA_DEMOS_GAME(ToonShadingDemo, {
    .id = "graphics.shader_effect.toon_shading",
    .title = "Custom GLSL shader (toon shading)",
    .description = "Writes a vertex and fragment shader in GLSL for CNA's ShaderEffect (CNAEXT): the sphere's "
                   "lighting is quantised into four flat bands. The GLSL version line follows the renderer's dialect.",
    .apis = {"ShaderEffect::ShaderEffect", "ShaderEffect::SetUniformVec3", "ShaderEffect::IsEffectValid",
             "ShaderEffect::GetCompileErrorEXT", "GraphicsDevice::GetShaderDialectEXT",
             "GraphicsDevice::ExecutesShaderEffectSourceEXT", "Effect::Apply"},
    .tags = {"3d", "shader", "glsl", "toon", "cel", "effect", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 3,
    .deterministic = true,
    .restrictions = "GLSL only: renderers that take SPIR-V, HLSL, MSL or WGSL skip it.",
})
