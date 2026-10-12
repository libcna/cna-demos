// SPDX-License-Identifier: MS-PL
// Keep a usable shader while reporting an invalid replacement's owned diagnostics.
#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "CNA/Graphics/ShaderCodeEXT.hpp"
#include "CNA/ShaderDiagnosticEXT.hpp"
#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr const char* Vertex = R"(
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); }
)";
    constexpr const char* Fragment = R"(
out vec4 FragColor;
void main() { FragColor = vec4(0.0, 1.0, 1.0, 1.0); }
)";
    constexpr const char* InvalidFragment = R"(
out vec4 FragColor;
void main() { FragColor = vec4(0.0, 1.0, 1.0, 1.0) }
)"; // Deliberately missing the assignment's semicolon, not a package-selection failure.

    class CompilerDiagnosticsDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto dialect = device.GetShaderDialectEXT();
            const bool es = dialect == CNA::ShaderDialectEXT::GlslEs;
            if ((!es && dialect != CNA::ShaderDialectEXT::GlslDesktop) || !device.ExecutesShaderEffectSourceEXT())
                Context().Skip("the demo needs a renderer that compiles GLSL source");
            const auto language = es ? CNA::ShaderLanguageEXT::GlslEs : CNA::ShaderLanguageEXT::GlslDesktop;
            const std::string header = es ? "#version 300 es\nprecision mediump float;\n" : "#version 330 core\n";
            const CNA::Graphics::ShaderCodeEXT vertex(language, CNA::ShaderStageEXT::Vertex,
                                                     "main", "retained.vert", header + Vertex);
            const CNA::Graphics::ShaderCodeEXT fragment(language, CNA::ShaderStageEXT::Fragment,
                                                       "main", "retained.frag", header + Fragment);
            good_ = std::make_unique<ShaderEffect>(device, vertex, fragment);
            if (!Context().Check(good_->IsEffectValid() && good_->GetCompileErrorEXT().empty() &&
                                 good_->GetShaderDiagnosticsEXT().empty(),
                                 "the retained valid shader has no compiler log or structured diagnostics"))
                throw std::runtime_error(good_->GetCompileErrorEXT());
            std::vector<CNA::ShaderDiagnosticEXT> diagnostics;
            std::string rawLog, firstMessage;
            {
                const CNA::Graphics::ShaderCodeEXT invalid(language, CNA::ShaderStageEXT::Fragment,
                                                          "main", "replacement.frag", header + InvalidFragment);
                // A usable explicit pair delegates actual compilation to the nonthrowing compiler.
                // Package-selection exceptions are demonstrated separately by portable_package.
                ShaderEffect replacement(device, vertex, invalid);
                rawLog = replacement.GetCompileErrorEXT();
                diagnostics = replacement.GetShaderDiagnosticsEXT();
                Context().Check(!replacement.IsEffectValid() && replacement.GetSelectedShaderLanguageEXT() == language,
                                "a syntactically invalid selected pair returns an invalid effect in the explicit language");
                Context().Check(!rawLog.empty() && !diagnostics.empty() &&
                                std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& item) {
                                    return item.getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Error;
                                }), "the failed compile exposes raw text and at least one structured error");
                // Stage and location may be unavailable; do not invent them or match vendor text.
                Context().Check(std::all_of(diagnostics.begin(), diagnostics.end(), [](const auto& item) {
                    return !item.getMessage().empty() && item.getLine() >= 0 && item.getColumn() >= 0 &&
                           (item.getLine() != 0 || item.getColumn() == 0) &&
                           (item.getStage() != CNA::ShaderStageEXT::Fragment || item.getSourceLabel() == "replacement.frag") &&
                           (item.getStage() != CNA::ShaderStageEXT::Vertex || item.getSourceLabel() == "retained.vert");
                }), "reported stages use the caller labels and available locations remain consistent");
                if (!diagnostics.empty()) firstMessage = diagnostics.front().getMessage();
            }
            Context().Check(!diagnostics.empty() && !firstMessage.empty() &&
                            diagnostics.front().getMessage() == firstMessage && !rawLog.empty(),
                            "owned compiler diagnostics remain readable after the rejected effect is destroyed");
            Context().Print("Rejected replacement compiler log:\n{}", rawLog);
            for (const auto& item : diagnostics)
                Context().Print("Diagnostic: stage={} label={} line={} column={} text={}",
                                static_cast<int>(item.getStage()), item.getSourceLabel(),
                                item.getLine(), item.getColumn(), item.getMessage());
            Context().Check(good_->IsEffectValid() && good_->GetCompileErrorEXT().empty() &&
                            good_->GetShaderDiagnosticsEXT().empty(),
                            "a rejected replacement leaves the retained shader valid and its diagnostics empty");
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
            batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                          nullptr, nullptr, good_.get());
            batch_->Draw(*white_, Rectangle(240, 140, 320, 200), Color::White);
            batch_->End();
            batch_->Begin();
            batch_->DrawString(*font_, "Retained shader after rejected replacement", Vector2(50, 60), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(CnaDemos::Near(pixels.At(400, 240), Color::Cyan, 0) &&
                                CnaDemos::Near(pixels.At(180, 240), Color::Black, 0),
                                "the retained shader still renders its cyan card after replacement compilation fails");
            }
            DemoGame::Draw(time);
        }
        std::unique_ptr<ShaderEffect> good_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::optional<Texture2D> white_;
    };
}

CNA_DEMOS_GAME(CompilerDiagnosticsDemo, {
    .id = "graphics.shader_effect.compiler_diagnostics",
    .title = "Shader compiler diagnostics and retained program",
    .description = "Rejects a GLSL fragment with a missing semicolon, retains owned compiler diagnostics after destruction, "
                   "and draws with the still-valid original shader.",
    .apis = {"ShaderEffect::ShaderEffect", "ShaderEffect::IsEffectValid", "ShaderEffect::GetCompileErrorEXT",
             "ShaderEffect::GetShaderDiagnosticsEXT", "ShaderEffect::GetSelectedShaderLanguageEXT",
             "CNA::ShaderDiagnosticEXT::getSeverity", "CNA::ShaderDiagnosticEXT::getStage",
             "CNA::ShaderDiagnosticEXT::getSourceLabel", "CNA::ShaderDiagnosticEXT::getLine",
             "CNA::ShaderDiagnosticEXT::getColumn", "CNA::ShaderDiagnosticEXT::getMessage"},
    .tags = {"shader", "diagnostics", "ownership", "cnaext"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL source compilation only. The deliberate compiler error is logged to stderr. "
                    "Vendor wording and optional stage/location fields are not prescribed; this is application-owned replacement policy, not a hot-reload API.",
})
