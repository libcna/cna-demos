// SPDX-License-Identifier: MS-PL
// A custom ShaderEffect passed to SpriteBatch::Begin: sprites drawn through your own pixel shader.
#include <memory>
#include <string>

#include "CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::Internal::Renderers::ShaderDialectEXT;

namespace
{
    // SpriteBatch feeds a sprite shader position (2D), texture coordinate and tint, and sets the
    // projection that maps pixels to clip space; the sprite's texture is sampler texture1.
    constexpr const char* VertexBody = R"(
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
uniform mat4 projection;
out vec2 vTexCoord;
out vec4 vColor;
void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
)";

    // Desaturate by uAmount: 0 = original colours, 1 = grey (Rec. 601 luma weights).
    constexpr const char* FragmentBody = R"(
in vec2 vTexCoord;
in vec4 vColor;
out vec4 FragColor;
uniform sampler2D texture1;
uniform float uAmount;
void main()
{
    vec4 c = texture(texture1, vTexCoord) * vColor;
    float grey = dot(c.rgb, vec3(0.299, 0.587, 0.114));
    FragColor = vec4(mix(c.rgb, vec3(grey), uAmount), c.a);
}
)";

    class SpriteShaderDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const ShaderDialectEXT dialect = device.GetShaderDialectEXT();
            std::string header;
            if (dialect == ShaderDialectEXT::GlslEs)
                header = "#version 300 es\nprecision mediump float;\n";
            else if (dialect == ShaderDialectEXT::GlslDesktop)
                header = "#version 330 core\n";
            else
                Context().Skip("this demo carries GLSL; the renderer wants another shading language");
            if (!device.ExecutesShaderEffectSourceEXT())
                Context().Skip("this renderer accepts shader source but does not execute it");

            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            gradient_ = getContentProperty().Load<Texture2D>("textures/gradient");
            effect_ = std::make_unique<ShaderEffect>(device, header + VertexBody, header + FragmentBody);
            Context().Check(effect_->IsEffectValid(), "the GLSL compiled and linked");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // Left: the stock sprite shader.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::LinearClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(gradient_, Rectangle(40, 80, 220, 320), Color::White);
            spriteBatch_->End();

            // Middle and right: the custom shader, half and fully desaturated. Apply first, then
            // set the uniform on the bound program, then Begin with the effect.
            for (int i = 0; i < 2; ++i)
            {
                effect_->Apply();
                effect_->SetUniformFloat("uAmount", i == 0 ? 0.5f : 1.0f);
                spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::LinearClamp,
                                    nullptr, nullptr, effect_.get());
                spriteBatch_->Draw(gradient_, Rectangle(290 + 250 * i, 80, 220, 320), Color::White);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                const Color original = p.At(200, 120), half = p.At(450, 120), grey = p.At(700, 120);
                Context().Print("same texel: original {}, 50% {}, 100% {}", original.ToString(), half.ToString(),
                                grey.ToString());
                Context().Check(grey.getRProperty() == grey.getGProperty() && grey.getGProperty() == grey.getBProperty(),
                                "uAmount 1: every channel equal (grey)");
                Context().Check(original.getRProperty() != original.getBProperty(), "the stock shader keeps colour");
                const double luma = 0.299 * original.getRProperty() + 0.587 * original.getGProperty() +
                                    0.114 * original.getBProperty();
                Context().CheckNear(grey.getRProperty(), luma, 2.0, "the grey is the texel's luma");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D gradient_;
        std::unique_ptr<ShaderEffect> effect_;
    };
}

CNA_DEMOS_GAME(SpriteShaderDemo, {
    .id = "graphics.shader_effect.sprite_shader",
    .title = "Custom sprite shader",
    .description = "Passes a GLSL ShaderEffect (CNAEXT) to SpriteBatch::Begin to desaturate sprites, with the amount "
                   "set as a uniform: original, 50% and 100% grey.",
    .apis = {"SpriteBatch::Begin", "ShaderEffect::ShaderEffect", "ShaderEffect::SetUniformFloat", "Effect::Apply"},
    .tags = {"shader", "glsl", "sprite", "postprocess", "grayscale", "cnaext"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/gradient.png"},
    .restrictions = "GLSL only: renderers that take SPIR-V, HLSL, MSL or WGSL skip it.",
})
