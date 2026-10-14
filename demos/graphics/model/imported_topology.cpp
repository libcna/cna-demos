// SPDX-License-Identifier: MS-PL
// glTF has loops and triangle runs; CNA normalizes them into drawable XNA primitive topologies.
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GltfImportReportEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ImportedTopologyDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            model_.emplace(getContentProperty().Load<Model>("models/loose_topology.gltf"));
            auto& ctx = Context();
            if (!ctx.Check(model_->getMeshesProperty().getCountProperty() == 7,
                           "all seven authored glTF topology modes import as separate meshes")) return;
            const std::array<std::vector<std::uint32_t>, 7> expected = {{
                {0, 1, 2, 3, 0}, {0, 1, 2, 2, 1, 3}, {0, 1, 2, 0, 2, 3, 0, 3, 4},
                {0, 1, 2}, {0, 1, 2}, {0, 1}, {0, 1, 2}
            }};
            const std::array<int, 7> vertices = {4, 4, 5, 3, 3, 3, 4};
            const std::array<int, 7> primitives = {4, 2, 3, 3, 2, 1, 1};
            const std::array<PrimitiveType, 7> topologies = {
                PrimitiveType::LineStrip, PrimitiveType::TriangleList, PrimitiveType::TriangleList,
                PrimitiveType::PointListEXT, PrimitiveType::LineStrip, PrimitiveType::LineList,
                PrimitiveType::TriangleList
            };
            for (int i = 0; i < 7; ++i)
            {
                auto* mesh = model_->getMeshesProperty()[i];
                if (!ctx.Check(mesh && mesh->getMeshPartsProperty().getCountProperty() == 1, "each topology mesh keeps one material part")) return;
                auto* part = mesh->getMeshPartsProperty()[0];
                if (!ctx.Check(part->getIndexBufferProperty() && part->getPrimitiveTypeEXTProperty() == topologies[i] &&
                               part->getNumVerticesProperty() == vertices[i] && part->getPrimitiveCountProperty() == primitives[i],
                               "imported part topology and counts match the normalized primitive run")) return;
                auto& buffer = *part->getIndexBufferProperty();
                const int count = static_cast<int>(expected[i].size());
                std::vector<std::uint32_t> actual(count);
                // Inspect only this part's index range. Index count is topology-dependent.
                if (buffer.getIndexElementSizeProperty() == IndexElementSize::SixteenBits)
                {
                    std::vector<std::uint16_t> narrow(count);
                    buffer.GetData(part->getStartIndexProperty() * 2, narrow.data(), 0, count);
                    actual.assign(narrow.begin(), narrow.end());
                }
                else buffer.GetData(part->getStartIndexProperty() * 4, actual.data(), 0, count);
                static constexpr std::array<const char*, 7> indexChecks = {
                    "LINE_LOOP appends its first index to close the line strip",
                    "TRIANGLE_STRIP alternates triangle corner order to preserve winding",
                    "TRIANGLE_FAN expands consecutive triangles around its first vertex",
                    "POINTS preserves every authored point index",
                    "LINE_STRIP preserves its open authored run",
                    "LINES drops its dangling singleton index",
                    "TRIANGLES drops its dangling singleton index"
                };
                ctx.Check(actual == expected[i], indexChecks[i]);
                auto* effect = dynamic_cast<BasicEffect*>(part->getEffectProperty());
                if (!ctx.Check(effect && !effect->getLightingEnabledProperty() && effect->getVertexColorEnabledProperty(),
                               "unlit vertex colors remain visible through the imported BasicEffect")) return;
            }

            const auto& report = model_->getGltfImportReportEXTProperty();
            std::size_t incompleteDiagnostics = 0;
            std::size_t incompleteIndices = 0;
            bool lineNamed = false;
            bool triangleNamed = false;
            for (const auto& diagnostic : report.Diagnostics)
            {
                if (diagnostic.Code != "incomplete-indices-dropped") continue;
                ++incompleteDiagnostics;
                incompleteIndices += diagnostic.Count;
                lineNamed |= diagnostic.Subject.find("IncompleteLines") != std::string::npos;
                triangleNamed |= diagnostic.Subject.find("IncompleteTriangles") != std::string::npos;
            }
            ctx.Check(incompleteDiagnostics == 2 && incompleteIndices == 2 && lineNamed && triangleNamed &&
                      report.AnythingLost() && report.getDroppedFeatureCountProperty() >= 2,
                      "the import report identifies both dropped singleton tails as authored data loss");
            ready_ = true;
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            if (ready_)
            {
                device.setBlendStateProperty(BlendState::Opaque);
                device.setDepthStencilStateProperty(DepthStencilState::None);
                device.setRasterizerStateProperty(RasterizerState::CullNone);
                const Matrix identity = Matrix::getIdentityProperty();
                model_->Draw(identity, identity, identity);
                if (FramesDrawn() == 0)
                {
                    const CnaDemos::BackBufferPixels pixels(device);
                    const auto at = [&](float x, float y) {
                        return pixels.At(static_cast<int>((x + 1) * .5f * pixels.Width()),
                                         static_cast<int>((1 - y) * .5f * pixels.Height()));
                    };
                    bool closure = true;
                    const int edgeX = static_cast<int>(.05f * pixels.Width());
                    for (float y : {-.2f, 0.f, .2f})
                    {
                        const int row = static_cast<int>((1 - y) * .5f * pixels.Height());
                        bool found = false;
                        for (int x = edgeX - 2; x <= edgeX + 2; ++x)
                            found |= CnaDemos::Near(pixels.At(x, row), Color::Gold, 2);
                        closure &= found;
                    }
                    Context().Check(closure, "the loop's added closing segment appears at three interior rows");
                    Context().Check(CnaDemos::Near(at(-.7f, 0), Color::DarkSlateGray, 2), "the loop interior stays unfilled");
                    Context().Check(CnaDemos::Near(at(-.15f, -.2f), Color::Red, 2) && CnaDemos::Near(at(.15f, .2f), Color::Red, 2),
                                    "both normalized strip triangles fill the quad");
                    Context().Check(CnaDemos::Near(at(.7f, -.2f), Color::Lime, 2) && CnaDemos::Near(at(.7f, .2f), Color::Lime, 2),
                                    "the fan's lower region and upper tip are filled");
                    Context().Check(CnaDemos::Near(at(.35f, 0), Color::DarkSlateGray, 2), "separate primitive runs leave their gap clear");

                    const auto colorVisible = [&](float x, float y, const Color& color) {
                        const int px = static_cast<int>((x + 1) * .5f * pixels.Width());
                        const int py = static_cast<int>((1 - y) * .5f * pixels.Height());
                        bool found = false;
                        for (int yy = py - 1; yy <= py + 1; ++yy)
                            for (int xx = px - 1; xx <= px + 1; ++xx)
                                found |= CnaDemos::Near(pixels.At(xx, yy), color, 2);
                        return found;
                    };
                    Context().Check(colorVisible(-.8f, .65f, Color::Cyan) &&
                                    colorVisible(-.65f, .55f, Color::Cyan) &&
                                    colorVisible(-.5f, .7f, Color::Cyan),
                                    "all three imported POINTS primitives reach the framebuffer");
                    Context().Check(colorVisible(.295f, .6f, Color::Blue) &&
                                    colorVisible(.385f, .6f, Color::Blue) &&
                                    CnaDemos::Near(at(.34f, .45f), Color::DarkSlateGray, 2),
                                    "LINE_STRIP draws both authored segments without closing its endpoints");
                    Context().Check(CnaDemos::Near(at(0, .55f), Color::Magenta, 2) &&
                                    CnaDemos::Near(at(.2f, .68f), Color::DarkSlateGray, 2),
                                    "the complete line draws while its dangling endpoint creates no segment");
                    Context().Check(CnaDemos::Near(at(.675f, .55f), Color(255, 128, 0), 2) &&
                                    CnaDemos::Near(at(.675f, .7f), Color(255, 128, 0), 2),
                                    "the complete triangle draws after its dangling index is removed");
                }
            }
            DemoGame::Draw(time);
        }
    private:
        std::optional<Model> model_;
        bool ready_ = false;
    };
}

CNA_DEMOS_GAME(ImportedTopologyDemo, {
    .id = "graphics.model.imported_topology",
    .title = "Import glTF primitive topologies and incomplete tails",
    .description = "Inspects normalized public mesh-part topologies, primitive counts and index buffers; "
                   "draws loops, strips, fans and points, and checks diagnostics for incomplete line and triangle tails.",
    .apis = {"ContentManager::Load", "Model::Draw", "ModelMeshPart::IndexBuffer", "ModelMeshPart::PrimitiveCount",
             "ModelMeshPart::NumVertices", "ModelMeshPart::StartIndex", "IndexBuffer::GetData",
             "Microsoft::Xna::Framework::Graphics::ModelMeshPart::PrimitiveTypeEXT",
             "Model::GltfImportReportEXT", "GltfImportReportEXT::AnythingLost"},
    .tags = {"model", "gltf", "topology", "indices", "points", "diagnostics", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_topology.gltf"},
})
