// SPDX-License-Identifier: MS-PL
// Compiled versus loose glTF morph-weight carriers and their public import report.
#include <cmath>
#include <cstdint>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GltfImportReportEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/MorphTargetEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const GltfImportDiagnosticEXT* FindDiagnostic(
        const GltfImportReportEXT& report, std::string_view code)
    {
        for (const GltfImportDiagnosticEXT& diagnostic : report.Diagnostics)
        {
            if (diagnostic.Code == code) return &diagnostic;
        }
        return nullptr;
    }

    ModelMeshPart* SolePart(Model& model)
    {
        if (model.getMeshesProperty().getCountProperty() != 1) return nullptr;
        ModelMesh* mesh = model.getMeshesProperty()[0];
        return mesh->getMeshPartsProperty().getCountProperty() == 1
            ? mesh->getMeshPartsProperty()[0]
            : nullptr;
    }

    class MorphAnimationImportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            Model compiled = getContentProperty().Load<Model>("models/morph_flag");
            Model loose = getContentProperty().Load<Model>("models/loose_morph_flag");
            ModelMeshPart* compiledPart = SolePart(compiled);
            ModelMeshPart* loosePart = SolePart(loose);
            auto* compiledMorph = compiledPart
                ? dynamic_cast<MorphTargetDataEXT*>(compiledPart->getTagProperty()) : nullptr;
            auto* looseMorph = loosePart
                ? dynamic_cast<MorphTargetDataEXT*>(loosePart->getTagProperty()) : nullptr;
            if (!ctx.Check(compiledMorph && looseMorph,
                           "compiled and loose model parts both carry MorphTargetDataEXT")) return;

            ctx.Check(compiledMorph->Stride == looseMorph->Stride &&
                          compiledMorph->BaseVertexBytes == looseMorph->BaseVertexBytes &&
                          compiledMorph->PositionDeltas.size() == 2 &&
                          looseMorph->PositionDeltas.size() == 2 &&
                          compiledMorph->Weights == std::vector<float>({0, 0}) &&
                          looseMorph->Weights == std::vector<float>({0, 0}) &&
                          compiledMorph->WeightTrack.Keys.size() == 4 &&
                          looseMorph->WeightTrack.Keys.size() == 4,
                      "both carriers preserve the same base layout, targets, defaults and four-key track");

            const std::vector<float> compiledWeights =
                EvaluateMorphWeightsEXT(compiledMorph->WeightTrack, 1.5);
            const std::vector<float> looseWeights =
                EvaluateMorphWeightsEXT(looseMorph->WeightTrack, 1.5);
            const std::vector<std::uint8_t> expected =
                BlendMorphTargetsEXT(*looseMorph, looseWeights);
            ctx.Check(compiledWeights == looseWeights && looseWeights.size() == 2 &&
                          std::abs(looseWeights[0] - 1) < 1e-4f &&
                          std::abs(looseWeights[1] - .5f) < 1e-4f &&
                          BlendMorphTargetsEXT(*compiledMorph, compiledWeights) == expected,
                      "compiled and loose tracks evaluate to the same weights and vertex bytes at 1.5 seconds");

            SetMorphWeightsEXT(*loosePart, looseWeights);
            std::vector<std::uint8_t> uploaded(expected.size());
            const int vertexCount = static_cast<int>(uploaded.size() /
                                                      static_cast<std::size_t>(looseMorph->Stride));
            loosePart->getVertexBufferProperty()->GetDataRawEXT(
                0, uploaded.data(), vertexCount, looseMorph->Stride);
            ctx.Check(looseMorph->Weights == looseWeights && uploaded == expected,
                      "SetMorphWeightsEXT stores the evaluated weights and uploads the exact blended bytes");

            const GltfImportReportEXT& compiledReport =
                compiled.getGltfImportReportEXTProperty();
            ctx.Check(compiledReport.Diagnostics.empty() &&
                          compiledReport.AnimationCount == 0 && compiledReport.ClipCount == 0,
                      "the compiled carrier has the format's ordinary empty import report");

            const GltfImportReportEXT& report = loose.getGltfImportReportEXTProperty();
            const auto* summary = FindDiagnostic(report, "morph-target-summary");
            const auto* empty = FindDiagnostic(report, "animations-empty");
            const auto* unsupported =
                FindDiagnostic(report, "animation-paths-unsupported");
            ctx.Check(unsupported == nullptr,
                      "a weights channel retained as a playable morph track is not reported as unsupported loss");
            // Until CNA-DEMOS-ISSUE-094 was fixed this pinned the defect's exact shape: one
            // animations-empty and one animation-paths-unsupported warning, both counted lost.
            ctx.Check(summary && summary->Count == 2 &&
                          summary->Kind == GltfImportDiagnosticKindEXT::Information &&
                          empty == nullptr && unsupported == nullptr &&
                          report.AnimationCount == 1 && report.ClipCount == 0 &&
                          report.getWarningCountProperty() == 0 &&
                          report.getDroppedFeatureCountProperty() == 0 &&
                          report.getApproximationCountProperty() == 0 && !report.AnythingLost(),
                      "the loose report keeps its morph summary and reports the retained weight animation as neither empty nor lost");
        }
    };
}

CNA_DEMOS_GAME(MorphAnimationImportDemo, {
    .id = "graphics.model.morph_animation_import",
    .title = "Morph animation import carriers",
    .description = "Compares the same generated morph animation through compiled and loose Model loading, "
                   "evaluates and uploads identical weight-driven vertex bytes, and checks that the "
                   "retained loose track is not reported lost (CNA-DEMOS-ISSUE-094).",
    .apis = {"MorphTargetDataEXT", "MorphWeightTrackEXT", "EvaluateMorphWeightsEXT",
             "BlendMorphTargetsEXT", "SetMorphWeightsEXT", "VertexBuffer::GetDataRawEXT",
             "ModelMeshPart::Tag", "Model::GltfImportReportEXT", "GltfImportReportEXT::Diagnostics"},
    .tags = {"model", "gltf", "morph", "animation", "content", "diagnostics", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .assets = {"models/morph_flag.cnb", "models/loose_morph_flag.gltf"},
})
