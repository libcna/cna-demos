// SPDX-License-Identifier: MS-PL
// CNAEXT glTF import report: what CNA's importer found, converted and approximated in a model.
#include <memory>
#include <optional>
#include <string>
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
    class ImportReportDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // CNA also loads .gltf/.glb files at run time, without the content pipeline. Such a model
            // carries an import report: counts of what the file held, and diagnostics for
            // everything converted, generated, approximated or dropped -- e.g. why an imported
            // model renders black (no lights) or looks flat (no normals).
            const Model loose = getContentProperty().Load<Model>("models/loose_robot_arm");
            const GltfImportReportEXT& report = loose.getGltfImportReportEXTProperty();
            Add("models/loose_robot_arm.gltf, loaded at run time:");
            Add("  " + std::to_string(report.NodeCount) + " nodes, " + std::to_string(report.PrimitiveCount) +
                " primitives, " + std::to_string(report.SkinCount) + " skins, " +
                std::to_string(report.ImportedLightCount) + " lights, depth " + std::to_string(report.MaxNodeDepth));
            for (const GltfImportDiagnosticEXT& diagnostic : report.Diagnostics)
                Add("  " + diagnostic.Code + ": " + diagnostic.Message);
            Context().Check(report.NodeCount == 3 && report.PrimitiveCount == 3 && report.SkinCount == 0,
                            "the report counts the arm's 3 nodes and 3 primitives, no skin");
            Context().Check(loose.getMeshesProperty().getCountProperty() == 3, "the loose glTF loads as the same model");

            // A model compiled by the pipeline (models/robot_arm.cnb) carries no report: the
            // pipeline printed its diagnostics when it built the asset.
            const Model compiled = getContentProperty().Load<Model>("models/robot_arm");
            Add("models/robot_arm.cnb (compiled): " +
                std::to_string(compiled.getGltfImportReportEXTProperty().NodeCount) + " nodes in the report");
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
    .description = "Loads a glTF file at run time, without the content pipeline, and prints its "
                   "Model::GltfImportReportEXT: node, primitive, skin and light counts and each diagnostic of CNA's "
                   "importer; a pipeline-compiled model carries no report.",
    .apis = {"Model::GltfImportReportEXT", "GltfImportReportEXT", "GltfImportDiagnosticEXT"},
    .tags = {"model", "gltf", "import", "diagnostics", "runtime-loading", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb", "models/loose_robot_arm.gltf", "models/robot_arm.cnb"},
})
