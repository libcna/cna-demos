// SPDX-License-Identifier: MS-PL
// CNAEXT glTF import report: structural counts and typed diagnostic outcomes from real imports.
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GltfImportReportEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const GltfImportDiagnosticEXT* FindDiagnostic(
        const GltfImportReportEXT& report, std::string_view code,
        std::string_view subject = {})
    {
        for (const GltfImportDiagnosticEXT& diagnostic : report.Diagnostics)
        {
            if (diagnostic.Code == code &&
                (subject.empty() || diagnostic.Subject == subject))
                return &diagnostic;
        }
        return nullptr;
    }

    bool IsEmptyReport(const GltfImportReportEXT& report)
    {
        return report.NodeCount == 0 && report.MeshInstanceCount == 0 &&
               report.DistinctMeshCount == 0 && report.SharedMeshCount == 0 &&
               report.MaxNodeDepth == 0 && report.CameraNodeCount == 0 &&
               report.LightNodeCount == 0 && report.ImportedLightCount == 0 &&
               report.PrimitiveCount == 0 && report.SkinCount == 0 &&
               report.AnimationCount == 0 && report.ClipCount == 0 &&
               report.Diagnostics.empty() && report.getWarningCountProperty() == 0 &&
               report.getDroppedFeatureCountProperty() == 0 &&
               report.getApproximationCountProperty() == 0 && !report.AnythingLost();
    }

    class ImportReportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            auto& ctx = Context();

            // Structural counts come from the represented source scene. The camera scene covers
            // nodes, distinct placements, depth, cameras, primitives and retained animation.
            const Model cameras = getContentProperty().Load<Model>("models/loose_camera_rig");
            const GltfImportReportEXT& cameraReport = cameras.getGltfImportReportEXTProperty();
            Add("camera rig: 7 nodes, 4 meshes, 3 cameras, 1 clip");
            ctx.Check(cameraReport.NodeCount == 7 && cameraReport.MeshInstanceCount == 4 &&
                          cameraReport.DistinctMeshCount == 4 && cameraReport.SharedMeshCount == 0 &&
                          cameraReport.MaxNodeDepth == 1 && cameraReport.CameraNodeCount == 3 &&
                          cameraReport.LightNodeCount == 0 && cameraReport.ImportedLightCount == 0 &&
                          cameraReport.PrimitiveCount == 4 && cameraReport.SkinCount == 0 &&
                          cameraReport.AnimationCount == 1 && cameraReport.ClipCount == 1,
                      "the camera report exposes every structural count at its exact source-scene value");
            ctx.Check(cameraReport.getWarningCountProperty() == 0 &&
                          cameraReport.getDroppedFeatureCountProperty() == 0 &&
                          cameraReport.getApproximationCountProperty() == 0 &&
                          !cameraReport.AnythingLost(),
                      "informational camera diagnostics do not make the aggregate report lossy");

            // One authored mesh has two independent placements and skins. That distinguishes all
            // three mesh counters: two instances, one distinct mesh, one shared mesh.
            const Model skins = getContentProperty().Load<Model>("models/loose_multi_skin");
            const GltfImportReportEXT& skinReport = skins.getGltfImportReportEXTProperty();
            Add("shared skin mesh: 2 instances / 1 distinct / 1 shared");
            ctx.Check(skinReport.NodeCount == 6 && skinReport.MeshInstanceCount == 2 &&
                          skinReport.DistinctMeshCount == 1 && skinReport.SharedMeshCount == 1 &&
                          skinReport.MaxNodeDepth == 2 && skinReport.PrimitiveCount == 2 &&
                          skinReport.SkinCount == 2,
                      "shared authored geometry is counted separately from its two skinned placements");

            // This loose copy of the existing lit-sphere fixture deliberately carries a stale
            // accessor bound and one optional unknown extension. Its geometry and lights still
            // import, while both source problems become typed public diagnostics.
            const Model lights = getContentProperty().Load<Model>("models/loose_lit_sphere");
            const GltfImportReportEXT& lightReport = lights.getGltfImportReportEXTProperty();
            Add("lit sphere: 2 light nodes / 2 effect light slots");
            ctx.Check(lightReport.NodeCount == 3 && lightReport.MeshInstanceCount == 1 &&
                          lightReport.DistinctMeshCount == 1 && lightReport.SharedMeshCount == 0 &&
                          lightReport.MaxNodeDepth == 1 && lightReport.CameraNodeCount == 0 &&
                          lightReport.LightNodeCount == 2 && lightReport.ImportedLightCount == 2 &&
                          lightReport.PrimitiveCount == 1 && lightReport.SkinCount == 0 &&
                          lightReport.AnimationCount == 0 && lightReport.ClipCount == 0,
                      "light-node and imported-slot counts distinguish authored from retained lights");
            const auto* invalid = FindDiagnostic(lightReport, "accessor-bounds-mismatch");
            const auto* unsupported = FindDiagnostic(lightReport, "ignored-extension");
            ctx.Check(invalid && invalid->Severity == GltfImportDiagnosticSeverityEXT::Warning &&
                          invalid->Kind == GltfImportDiagnosticKindEXT::InvalidSourceData &&
                          invalid->Subject.empty() && invalid->Count == 1 &&
                          invalid->WorstMagnitude == 0.0 && invalid->Details.empty() &&
                          invalid->Message.find("Accessor 0 component 0") != std::string::npos,
                      "stale accessor bounds retain code, severity, kind, count and actionable message");
            ctx.Check(unsupported && unsupported->Severity == GltfImportDiagnosticSeverityEXT::Warning &&
                          unsupported->Kind == GltfImportDiagnosticKindEXT::UnsupportedFeature &&
                          unsupported->Subject.empty() && unsupported->Count == 1 &&
                          unsupported->WorstMagnitude == 0.0 && unsupported->Details.empty() &&
                          unsupported->Message.find("EXT_cna_demos_report") != std::string::npos &&
                          unsupported->Message.find("/assets/") == std::string::npos,
                      "an optional unknown extension is a portable unsupported-feature warning");
            ctx.Check(lightReport.Diagnostics.size() == 2 &&
                          lightReport.getWarningCountProperty() == 2 &&
                          lightReport.getDroppedFeatureCountProperty() == 1 &&
                          lightReport.getApproximationCountProperty() == 0 &&
                          lightReport.AnythingLost(),
                      "warning entries and dropped occurrences have distinct exact aggregates");

            // A harmless layout note supplies every diagnostic payload field, including Details,
            // without making the report lossy.
            const Model robot = getContentProperty().Load<Model>("models/loose_robot_arm");
            const GltfImportReportEXT& robotReport = robot.getGltfImportReportEXTProperty();
            const auto* information = FindDiagnostic(robotReport, "vertex-layout-limit", "BaseMesh");
            Add("information: vertex-layout-limit on BaseMesh");
            ctx.Check(robotReport.Diagnostics.size() == 3 && information &&
                          information->Severity == GltfImportDiagnosticSeverityEXT::Information &&
                          information->Kind == GltfImportDiagnosticKindEXT::Information &&
                          information->Subject == "BaseMesh" && information->Count == 1 &&
                          information->WorstMagnitude == 0.0 && information->Details.size() == 1 &&
                          information->Details[0].find("Normal stream") != std::string::npos &&
                          !information->Message.empty() && !robotReport.AnythingLost(),
                      "an information entry exposes code, subject, count, details and display message");

            // The material-variant fixture needs a generated normal basis only for its PBR
            // variant. GeneratedData remains informational even when Count is greater elsewhere.
            const Model variants = getContentProperty().Load<Model>("models/loose_paint_toy");
            const GltfImportReportEXT& variantReport = variants.getGltfImportReportEXTProperty();
            const auto* generated = FindDiagnostic(
                variantReport, "normals-generated", "BodyMesh variant 0");
            Add("generated: normals for BodyMesh variant 0");
            ctx.Check(generated && generated->Severity == GltfImportDiagnosticSeverityEXT::Information &&
                          generated->Kind == GltfImportDiagnosticKindEXT::GeneratedData &&
                          generated->Count == 1 && generated->WorstMagnitude == 0.0 &&
                          generated->Details.empty() && !generated->Message.empty() &&
                          variantReport.getWarningCountProperty() == 0 && !variantReport.AnythingLost(),
                      "generated data is recorded as a non-lossy informational diagnostic");

            // Existing topology and skin-weight fixtures provide actual dropped and approximated
            // authored data, including multi-occurrence aggregate semantics.
            const Model topology = getContentProperty().Load<Model>("models/loose_topology");
            const GltfImportReportEXT& topologyReport = topology.getGltfImportReportEXTProperty();
            const auto* conversion = FindDiagnostic(topologyReport, "topology-converted", "Loop");
            const auto* dropped = FindDiagnostic(
                topologyReport, "incomplete-indices-dropped", "IncompleteLines");
            Add("dropped: 2 incomplete indices; exact conversions stay info");
            ctx.Check(conversion && conversion->Severity == GltfImportDiagnosticSeverityEXT::Information &&
                          conversion->Kind == GltfImportDiagnosticKindEXT::Information &&
                          conversion->Count == 1 && conversion->Details ==
                              std::vector<std::string>{"LINE_LOOP", "LINE_STRIP"},
                      "an exact topology conversion names both source and retained forms in Details");
            ctx.Check(dropped && dropped->Severity == GltfImportDiagnosticSeverityEXT::Warning &&
                          dropped->Kind == GltfImportDiagnosticKindEXT::DroppedData &&
                          dropped->Subject == "IncompleteLines" && dropped->Count == 1 &&
                          dropped->WorstMagnitude == 0.0 && dropped->Details.empty() &&
                          topologyReport.getWarningCountProperty() == 2 &&
                          topologyReport.getDroppedFeatureCountProperty() == 2 &&
                          topologyReport.getApproximationCountProperty() == 0 &&
                          topologyReport.AnythingLost(),
                      "dropped tails count as two warning entries and two lost occurrences");

            const Model weights = getContentProperty().Load<Model>("models/loose_skin_weight_forms");
            const GltfImportReportEXT& weightReport = weights.getGltfImportReportEXTProperty();
            const auto* approximation = FindDiagnostic(
                weightReport, "weights-renormalised", "Renormalized");
            Add("approximated: 8 weights; dropped: 5 influence cases");
            ctx.Check(approximation &&
                          approximation->Severity == GltfImportDiagnosticSeverityEXT::Warning &&
                          approximation->Kind == GltfImportDiagnosticKindEXT::Approximation &&
                          approximation->Count == 4 &&
                          std::abs(approximation->WorstMagnitude - 0.4) < 1e-5 &&
                          approximation->Details.empty() && !approximation->Message.empty(),
                      "weight repair reports its occurrence count and worst measured deviation");
            ctx.Check(weightReport.getWarningCountProperty() == 4 &&
                          weightReport.getDroppedFeatureCountProperty() == 5 &&
                          weightReport.getApproximationCountProperty() == 8 &&
                          weightReport.AnythingLost(),
                      "report aggregates count entries for warnings but occurrences for loss kinds");

            // This project's ordinary compiled Model path does not serialize an import report, so
            // every field and aggregate is the documented empty default rather than stale data.
            const Model compiled = getContentProperty().Load<Model>("models/robot_arm");
            Add("compiled robot: all-zero empty report");
            ctx.Check(IsEmptyReport(compiled.getGltfImportReportEXTProperty()),
                      "a compiled model without serialized import metadata exposes the full empty default");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            float y = 20.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line.substr(0, 64), Vector2(15.0f, y), Color::White);
                y += 24.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void Add(const std::string& line)
        {
            Context().Print("{}", line);
            lines_.push_back(line);
        }

        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ImportReportDemo, {
    .id = "graphics.model.import_report",
    .title = "glTF import report (CNAEXT)",
    .description = "Loads focused glTF fixtures at run time and verifies every GltfImportReportEXT structural "
                   "count, all six typed diagnostic kinds and their complete payload, exact warning/drop/approximation "
                   "aggregates, AnythingLost, and the empty-report contract of an ordinary compiled model.",
    .apis = {"ContentManager::Load", "Model::GltfImportReportEXT", "GltfImportReportEXT",
             "GltfImportReportEXT::NodeCount", "GltfImportReportEXT::MeshInstanceCount",
             "GltfImportReportEXT::DistinctMeshCount", "GltfImportReportEXT::SharedMeshCount",
             "GltfImportReportEXT::MaxNodeDepth", "GltfImportReportEXT::CameraNodeCount",
             "GltfImportReportEXT::LightNodeCount", "GltfImportReportEXT::ImportedLightCount",
             "GltfImportReportEXT::PrimitiveCount", "GltfImportReportEXT::SkinCount",
             "GltfImportReportEXT::AnimationCount", "GltfImportReportEXT::ClipCount",
             "GltfImportReportEXT::Diagnostics", "GltfImportReportEXT::WarningCount",
             "GltfImportReportEXT::DroppedFeatureCount", "GltfImportReportEXT::ApproximationCount",
             "GltfImportReportEXT::AnythingLost", "GltfImportDiagnosticEXT",
             "GltfImportDiagnosticEXT::Code", "GltfImportDiagnosticEXT::Severity",
             "GltfImportDiagnosticEXT::Kind", "GltfImportDiagnosticEXT::Subject",
             "GltfImportDiagnosticEXT::Count", "GltfImportDiagnosticEXT::WorstMagnitude",
             "GltfImportDiagnosticEXT::Details", "GltfImportDiagnosticEXT::Message",
             "GltfImportDiagnosticSeverityEXT", "GltfImportDiagnosticSeverityEXT::Information",
             "GltfImportDiagnosticSeverityEXT::Warning", "GltfImportDiagnosticKindEXT",
             "GltfImportDiagnosticKindEXT::Information", "GltfImportDiagnosticKindEXT::GeneratedData",
             "GltfImportDiagnosticKindEXT::InvalidSourceData", "GltfImportDiagnosticKindEXT::Approximation",
             "GltfImportDiagnosticKindEXT::DroppedData", "GltfImportDiagnosticKindEXT::UnsupportedFeature"},
    .tags = {"model", "gltf", "import", "diagnostics", "runtime-loading", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "models/loose_camera_rig.gltf",
               "models/loose_multi_skin.gltf", "models/loose_lit_sphere.gltf",
               "models/loose_robot_arm.gltf", "models/loose_paint_toy.gltf",
               "models/loose_topology.gltf", "models/loose_skin_weight_forms.gltf",
               "models/robot_arm.cnb"},
})
