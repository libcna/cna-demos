// SPDX-License-Identifier: MS-PL
// Select an owned portable shader package, then execute its selected pair.
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Graphics/ShaderPackageEXT.hpp"
#include "CNA/ShaderDiagnosticEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::Graphics::ShaderCodeEXT;
using CNA::Graphics::ShaderPackageEXT;
using CNA::ShaderLanguageEXT;
using CNA::ShaderStageEXT;

namespace
{
    constexpr const char* Vertex = R"(
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
uniform mat4 projection;
out vec2 uv;
out vec4 tint;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    uv = aTexCoord;
    tint = aColor;
}
)";
    constexpr const char* Fragment = R"(
in vec2 uv;
in vec4 tint;
uniform sampler2D texture1;
out vec4 FragColor;
void main() {
    vec4 color = texture(texture1, uv) * tint;
    FragColor = vec4(color.b, color.r, color.g, color.a);
}
)";

    class PortablePackageDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<ShaderCodeEXT> variants;
            for (const auto language : {ShaderLanguageEXT::GlslEs, ShaderLanguageEXT::GlslDesktop})
            {
                const std::string header = language == ShaderLanguageEXT::GlslEs
                    ? "#version 300 es\nprecision mediump float;\n" : "#version 330 core\n";
                std::string vertexSource = header + Vertex, fragmentSource = header + Fragment;
                variants.emplace_back(language, ShaderStageEXT::Vertex, "main", "channel-cycle.vert", vertexSource);
                variants.emplace_back(language, ShaderStageEXT::Fragment, "main", "channel-cycle.frag", fragmentSource);
                // Code values own the source; overwriting the author's buffers cannot alter them.
                vertexSource.assign("overwritten"); fragmentSource.assign("overwritten");
            }
            const ShaderPackageEXT package(variants, {ShaderStageEXT::Vertex, ShaderStageEXT::Fragment},
                {{"texture1", 0, CNA::Graphics::ShaderBindingTypeEXT::SampledTexture2D, ShaderStageEXT::Fragment}});
            selection_.emplace(package.selectFor(device));
            Context().Print("Package selection: {}", selection_->getDiagnostic());
            if (!selection_->isUsable()) Context().Skip("the live renderer cannot execute either supplied GLSL language");
            const auto* vertex = selection_->findStage(ShaderStageEXT::Vertex);
            const auto* fragment = selection_->findStage(ShaderStageEXT::Fragment);
            Context().Check(selection_->getCode().size() == 2 && vertex && fragment &&
                            vertex->getLanguage() == fragment->getLanguage() &&
                            vertex->getText().find("gl_Position") != std::string::npos &&
                            fragment->getSourceLabel() == "channel-cycle.frag",
                            "selection owns one labeled stage pair in a single supported language after caller buffers change");
            if (!vertex || !fragment) return;
            std::reverse(variants.begin(), variants.end());
            const auto reordered = ShaderPackageEXT(variants, package.getRequiredStages(),
                                                    package.getBindingRequirements()).selectFor(device);
            Context().Check(reordered.isUsable() && reordered.getLanguage() == selection_->getLanguage() &&
                            reordered.findStage(ShaderStageEXT::Vertex)->getText() == vertex->getText(),
                            "variant declaration order does not change live-device selection");

            // A selected-language vertex plus the other language's fragment has no complete pair.
            const auto otherFragment = std::find_if(variants.begin(), variants.end(), [&](const ShaderCodeEXT& code) {
                return code.getStage() == ShaderStageEXT::Fragment && code.getLanguage() != selection_->getLanguage();
            });
            const ShaderPackageEXT incompletePackage({*vertex, *otherFragment}, package.getRequiredStages());
            const auto incomplete = incompletePackage.selectFor(device);
            Context().Check(!incomplete.isUsable() && incomplete.getCode().empty() &&
                            incomplete.getLanguage() == ShaderLanguageEXT::Unknown && !incomplete.getDiagnostic().empty(),
                            "incomplete per-language pairs cannot be mixed into a usable program");
            std::vector<CNA::ShaderDiagnosticEXT> refusal;
            try { ShaderEffect unusable(device, incompletePackage); }
            catch (const CNA::ShaderCompilationExceptionEXT& error) { refusal = error.getDiagnostics(); }
            Context().Check(refusal.size() == 2 &&
                            refusal[0].getStage() == ShaderStageEXT::Vertex &&
                            refusal[1].getStage() == ShaderStageEXT::Fragment &&
                            refusal[0].getSourceLabel() == "channel-cycle.vert" &&
                            refusal[1].getSourceLabel() == "channel-cycle.frag" &&
                            std::all_of(refusal.begin(), refusal.end(), [](const auto& diagnostic) {
                                return diagnostic.getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Error &&
                                       !diagnostic.getMessage().empty();
                            }),
                            "effect construction refuses the incomplete package with owned labeled stage diagnostics");
            const auto ambiguous = ShaderPackageEXT({*vertex, *fragment, *vertex}, package.getRequiredStages()).selectFor(device);
            Context().Check(!ambiguous.isUsable() && !ambiguous.getDiagnostic().empty(),
                            "duplicate candidates for the same language and stage are refused as ambiguous");
            effect_ = std::make_unique<ShaderEffect>(device, package);
            Context().Check(effect_->IsEffectValid() && effect_->HasRenderer() &&
                            effect_->GetSelectedShaderLanguageEXT() == selection_->getLanguage() &&
                            effect_->GetCompileErrorEXT().empty() && effect_->GetShaderDiagnosticsEXT().empty(),
                            "the package constructor compiles the selected pair and retains its explicit language");
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            white_.emplace(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            const std::array<Color, 3> colors{Color::Red, Color::Lime, Color::Blue};
            for (int row = 0; row < 2; ++row)
            {
                batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                              nullptr, nullptr, row == 0 ? nullptr : effect_.get());
                for (int column = 0; column < 3; ++column)
                    batch_->Draw(*white_, Rectangle(80 + 240 * column, 80 + 200 * row, 160, 120), colors[column]);
                batch_->End();
            }
            batch_->Begin();
            batch_->DrawString(*font_, "Original RGB", Vector2(80, 45), Color::White);
            batch_->DrawString(*font_, "Selected shader output", Vector2(80, 245), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(CnaDemos::Near(pixels.At(160, 140), Color::Red, 0) &&
                                CnaDemos::Near(pixels.At(400, 140), Color::Lime, 0) &&
                                CnaDemos::Near(pixels.At(640, 140), Color::Blue, 0),
                                "stock shader control renders the authored red green blue row");
                Context().Check(CnaDemos::Near(pixels.At(160, 340), Color::Lime, 0) &&
                                CnaDemos::Near(pixels.At(400, 340), Color::Blue, 0) &&
                                CnaDemos::Near(pixels.At(640, 340), Color::Red, 0),
                                "the selected package executes its channel-cycle fragment program");
            }
            DemoGame::Draw(time);
        }
        std::optional<CNA::Graphics::ShaderPackageSelectionEXT> selection_;
        std::unique_ptr<ShaderEffect> effect_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<Texture2D> white_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PortablePackageDemo, {
    .id = "graphics.shader_effect.portable_package",
    .title = "Portable shader package selection",
    .description = "Selects an owned GLSL stage pair for the live renderer, rejects incomplete or ambiguous variants, "
                   "and draws a color-channel comparison through the package-created ShaderEffect.",
    .apis = {"CNA::Graphics::ShaderCodeEXT", "CNA::Graphics::ShaderPackageEXT::selectFor",
             "CNA::Graphics::ShaderPackageSelectionEXT", "ShaderEffect::ShaderEffect",
             "ShaderEffect::GetSelectedShaderLanguageEXT", "ShaderEffect::GetShaderDiagnosticsEXT",
             "CNA::ShaderCompilationExceptionEXT::getDiagnostics"},
    .tags = {"shader", "package", "glsl", "selection", "cnaext"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Supplies GLSL ES and desktop GLSL only; no SPIR-V, DXIL, WGSL or compiler-error-path coverage.",
})
