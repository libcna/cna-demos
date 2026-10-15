// SPDX-License-Identifier: MS-PL
// Rigid glTF mesh instances: scene selection, composed placement and actionable loss diagnostics.
#include <array>
#include <cmath>
#include <optional>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GltfImportReportEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    bool Near(const Vector3& actual, const Vector3& expected, float tolerance = 0.0001f)
    {
        return Vector3::Distance(actual, expected) <= tolerance;
    }

    Vector3 Point(const Matrix& transform, const Vector3& local = Vector3::Zero)
    {
        return Vector3::Transform(local, transform);
    }

    const GltfImportDiagnosticEXT* FindDiagnostic(
        const GltfImportReportEXT& report, std::string_view code)
    {
        for (const GltfImportDiagnosticEXT& diagnostic : report.Diagnostics)
        {
            if (diagnostic.Code == code) return &diagnostic;
        }
        return nullptr;
    }

    struct Placement
    {
        ModelMesh* Mesh = nullptr;
        Matrix World;
        Vector3 Center;
        bool Mirrored = false;
    };

    class NodeInstancesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            model_.emplace(getContentProperty().Load<Model>("models/loose_node_instances"));
            const auto& bones = model_->getBonesProperty();
            const auto& meshes = model_->getMeshesProperty();

            ctx.Check(bones.getCountProperty() == 9 && meshes.getCountProperty() == 5,
                      "the selected scene becomes eight bones plus the synthetic root and five placements");

            ModelBone* trsParent = nullptr;
            ModelBone* trs = nullptr;
            ModelBone* matrix = nullptr;
            ModelBone* mirrorParent = nullptr;
            ModelBone* mirrored = nullptr;
            ModelBone* doubleMirrorParent = nullptr;
            ModelBone* evenMirror = nullptr;
            ModelBone* gpu = nullptr;
            ModelBone* offScene = nullptr;
            const bool namedBones =
                bones.TryGetValue("TrsParent", trsParent) &&
                bones.TryGetValue("TrsInstance", trs) &&
                bones.TryGetValue("MatrixInstance", matrix) &&
                bones.TryGetValue("MirrorParent", mirrorParent) &&
                bones.TryGetValue("MirroredInstance", mirrored) &&
                bones.TryGetValue("DoubleMirrorParent", doubleMirrorParent) &&
                bones.TryGetValue("EvenMirrorInstance", evenMirror) &&
                bones.TryGetValue("GpuInstance", gpu) &&
                !bones.TryGetValue("OffScene", offScene) && offScene == nullptr;
            if (!ctx.Check(namedBones,
                           "all selected-scene nodes are named bones while the decoy scene is absent"))
                return;
            ctx.Check(trs->getParentProperty() == trsParent &&
                          mirrored->getParentProperty() == mirrorParent &&
                          evenMirror->getParentProperty() == doubleMirrorParent,
                      "transform-only parents retain the authored hierarchy above their mesh nodes");

            absolute_.resize(static_cast<std::size_t>(bones.getCountProperty()));
            model_->CopyAbsoluteBoneTransformsTo(absolute_);
            const auto worldOf = [&](const ModelBone* bone) -> const Matrix& {
                return absolute_[static_cast<std::size_t>(bone->getIndexProperty())];
            };

            const Matrix& trsWorld = worldOf(trs);
            const Vector3 trsCenter = Point(trsWorld);
            ctx.Check(Near(trsCenter, Vector3(-.4f, .35f, 0)) &&
                          Near(Point(trsWorld, Vector3::UnitX) - trsCenter, Vector3(0, .6f, 0)) &&
                          Near(Point(trsWorld, Vector3::UnitY) - trsCenter, Vector3(-1.2f, 0, 0)),
                      "TRS scale and quarter-turn compose once below the translated parent");

            const Matrix& matrixWorld = worldOf(matrix);
            const Vector3 matrixCenter = Point(matrixWorld);
            ctx.Check(Near(matrixCenter, Vector3(.4f, .35f, 0)) &&
                          Near(Point(matrixWorld, Vector3::UnitX) - matrixCenter, Vector3(0, -.6f, 0)) &&
                          Near(Point(matrixWorld, Vector3::UnitY) - matrixCenter, Vector3(1.2f, 0, 0)),
                      "an explicit glTF matrix preserves its translation, nonuniform scale and opposite turn");

            const Matrix& mirroredWorld = worldOf(mirrored);
            const Matrix& evenWorld = worldOf(evenMirror);
            ctx.Check(Near(Point(mirroredWorld), Vector3(-.4f, -.35f, 0)) &&
                          mirroredWorld.Determinant() < 0 &&
                          Near(Point(evenWorld), Vector3(.4f, -.35f, 0)) &&
                          evenWorld.Determinant() > 0,
                      "one mirroring ancestor flips the composed placement while two restore handedness");
            ctx.Check(Near(Point(worldOf(gpu)), Vector3(0, -.65f, 0)),
                      "the unsupported GPU-instancing node retains its own single placement");

            const std::array<std::pair<std::string_view, ModelBone*>, 5> expectedMeshes{{
                {"SharedMarker_TrsInstance", trs},
                {"SharedMarker_MatrixInstance", matrix},
                {"SharedMarker_MirroredInstance", mirrored},
                {"SharedMarker_EvenMirrorInstance", evenMirror},
                {"SharedMarker_GpuInstance", gpu},
            }};
            bool placementLinks = true;
            placements_.reserve(expectedMeshes.size());
            for (const auto& [name, bone] : expectedMeshes)
            {
                ModelMesh* mesh = nullptr;
                placementLinks = placementLinks && meshes.TryGetValue(std::string(name), mesh) &&
                                 mesh != nullptr && mesh->getParentBoneProperty() == bone &&
                                 mesh->getMeshPartsProperty().getCountProperty() == 1;
                if (mesh != nullptr)
                {
                    const Matrix world = worldOf(bone);
                    placements_.push_back({mesh, world, Point(world), world.Determinant() < 0});
                }
            }
            ModelMesh* decoy = nullptr;
            placementLinks = placementLinks &&
                             !meshes.TryGetValue("SharedMarker_OffScene", decoy) && decoy == nullptr;
            if (!ctx.Check(placementLinks && placements_.size() == expectedMeshes.size(),
                           "each shared-mesh placement has its own named mesh and exact parent bone"))
                return;

            const GltfImportReportEXT& report = model_->getGltfImportReportEXTProperty();
            ctx.Check(report.NodeCount == 8 && report.MeshInstanceCount == 5 &&
                          report.DistinctMeshCount == 1 && report.SharedMeshCount == 1 &&
                          report.MaxNodeDepth == 2 && report.PrimitiveCount == 5 &&
                          report.CameraNodeCount == 0 && report.LightNodeCount == 0 &&
                          report.SkinCount == 0 && report.AnimationCount == 0,
                      "the import report counts the selected hierarchy and repeated source mesh exactly");

            const auto* mirroredDiagnostic = FindDiagnostic(report, "mirrored-winding-unapplied");
            const auto* gpuDiagnostic = FindDiagnostic(report, "gpu-instancing-dropped");
            const auto* extensionDiagnostic = FindDiagnostic(report, "ignored-extension");
            ctx.Check(mirroredDiagnostic &&
                          mirroredDiagnostic->Kind == GltfImportDiagnosticKindEXT::DroppedData &&
                          mirroredDiagnostic->Subject == "MirroredInstance" &&
                          mirroredDiagnostic->Count == 1 &&
                          gpuDiagnostic && gpuDiagnostic->Kind == GltfImportDiagnosticKindEXT::DroppedData &&
                          gpuDiagnostic->Count == 1 && gpuDiagnostic->Subject.empty() &&
                          extensionDiagnostic &&
                          extensionDiagnostic->Kind == GltfImportDiagnosticKindEXT::UnsupportedFeature &&
                          extensionDiagnostic->Message.find("EXT_mesh_gpu_instancing") != std::string::npos,
                      "mirrored winding and optional GPU instancing produce distinct actionable diagnostics");
            ctx.Check(report.Diagnostics.size() == 3 && report.getWarningCountProperty() == 3 &&
                          report.getDroppedFeatureCountProperty() == 3 &&
                          report.getApproximationCountProperty() == 0 && report.AnythingLost(),
                      "the three warnings aggregate three dropped occurrences without inventing an approximation");

            const Model implicit =
                getContentProperty().Load<Model>("models/loose_scene_fallback_implicit");
            ModelBone* implicitFirst = nullptr;
            ModelBone* implicitSecond = nullptr;
            ModelMesh* implicitMesh = nullptr;
            const auto& implicitBones = implicit.getBonesProperty();
            const auto& implicitMeshes = implicit.getMeshesProperty();
            ctx.Check(implicitBones.getCountProperty() == 2 &&
                          implicitMeshes.getCountProperty() == 1 &&
                          implicitBones.TryGetValue("ImplicitFirst", implicitFirst) &&
                          !implicitBones.TryGetValue("ImplicitSecond", implicitSecond) &&
                          implicitSecond == nullptr &&
                          implicitMeshes.TryGetValue("ImplicitShape", implicitMesh) &&
                          implicitMesh != nullptr &&
                          implicitMesh->getParentBoneProperty() == implicitFirst,
                      "omitting the default-scene index selects the first scene and de-duplicates its repeated root");
            const GltfImportReportEXT& implicitReport =
                implicit.getGltfImportReportEXTProperty();
            ctx.Check(implicitReport.NodeCount == 1 && implicitReport.MeshInstanceCount == 1 &&
                          implicitReport.DistinctMeshCount == 1 &&
                          implicitReport.SharedMeshCount == 0 && implicitReport.MaxNodeDepth == 1 &&
                          implicitReport.Diagnostics.empty(),
                      "the implicit first scene has one node, one placement and no invented warning");

            const Model rooted =
                getContentProperty().Load<Model>("models/loose_scene_fallback_nodes");
            ModelBone* fallbackRoot = nullptr;
            ModelBone* fallbackChild = nullptr;
            ModelBone* fallbackPeer = nullptr;
            ModelMesh* fallbackChildMesh = nullptr;
            ModelMesh* fallbackPeerMesh = nullptr;
            const auto& rootedBones = rooted.getBonesProperty();
            const auto& rootedMeshes = rooted.getMeshesProperty();
            std::vector<Matrix> rootedAbsolute(
                static_cast<std::size_t>(rootedBones.getCountProperty()));
            rooted.CopyAbsoluteBoneTransformsTo(rootedAbsolute);
            const bool rootedNames =
                rootedBones.TryGetValue("FallbackRoot", fallbackRoot) &&
                rootedBones.TryGetValue("FallbackChild", fallbackChild) &&
                rootedBones.TryGetValue("FallbackPeer", fallbackPeer) &&
                rootedMeshes.TryGetValue("FallbackShape_FallbackChild", fallbackChildMesh) &&
                rootedMeshes.TryGetValue("FallbackShape_FallbackPeer", fallbackPeerMesh);
            ctx.Check(rootedNames && rootedBones.getCountProperty() == 4 &&
                          rootedMeshes.getCountProperty() == 2 &&
                          fallbackChild->getParentProperty() == fallbackRoot &&
                          fallbackChildMesh->getParentBoneProperty() == fallbackChild &&
                          fallbackPeerMesh->getParentBoneProperty() == fallbackPeer &&
                          Near(Point(rootedAbsolute[static_cast<std::size_t>(
                                         fallbackChild->getIndexProperty())]),
                               Vector3(.1f, .3f, 0)) &&
                          Near(Point(rootedAbsolute[static_cast<std::size_t>(
                                         fallbackPeer->getIndexProperty())]),
                               Vector3(.5f, -.2f, 0)),
                      "a file with no scenes imports every root hierarchy with composed transforms");
            const GltfImportReportEXT& rootedReport = rooted.getGltfImportReportEXTProperty();
            ctx.Check(rootedReport.NodeCount == 3 && rootedReport.MeshInstanceCount == 2 &&
                          rootedReport.DistinctMeshCount == 1 && rootedReport.SharedMeshCount == 1 &&
                          rootedReport.MaxNodeDepth == 2 && rootedReport.Diagnostics.empty(),
                      "the no-scenes root fallback reports both placements of the shared mesh");

            const Model bare =
                getContentProperty().Load<Model>("models/loose_scene_fallback_meshes");
            ModelMesh* bareA = nullptr;
            ModelMesh* bareB = nullptr;
            const auto& bareBones = bare.getBonesProperty();
            const auto& bareMeshes = bare.getMeshesProperty();
            ModelBone* bareRoot = bare.getRootProperty();
            ctx.Check(bareBones.getCountProperty() == 1 && bareMeshes.getCountProperty() == 2 &&
                          bareMeshes.TryGetValue("BareA", bareA) &&
                          bareMeshes.TryGetValue("BareB", bareB) && bareRoot != nullptr &&
                          bareA->getParentBoneProperty() == bareRoot &&
                          bareB->getParentBoneProperty() == bareRoot,
                      "a file with neither scenes nor nodes imports every bare mesh at the identity root");
            const GltfImportReportEXT& bareReport = bare.getGltfImportReportEXTProperty();
            ctx.Check(bareReport.NodeCount == 0 && bareReport.MeshInstanceCount == 2 &&
                          bareReport.DistinctMeshCount == 2 && bareReport.SharedMeshCount == 0 &&
                          bareReport.MaxNodeDepth == 0 && bareReport.Diagnostics.empty(),
                      "the bare-mesh fallback reports no source nodes and two distinct placements");

            bool emptySceneRefused = false;
            bool exactEmptySceneLeak = false;
            try
            {
                const Model empty =
                    getContentProperty().Load<Model>("models/loose_scene_fallback_empty");
                ModelMesh* excludedMesh = nullptr;
                ModelBone* excludedBone = nullptr;
                const GltfImportReportEXT& emptyReport =
                    empty.getGltfImportReportEXTProperty();
                exactEmptySceneLeak =
                    empty.getBonesProperty().getCountProperty() == 1 &&
                    empty.getMeshesProperty().getCountProperty() == 1 &&
                    empty.getMeshesProperty().TryGetValue("ExcludedShape", excludedMesh) &&
                    excludedMesh != nullptr &&
                    excludedMesh->getParentBoneProperty() == empty.getRootProperty() &&
                    !empty.getBonesProperty().TryGetValue("ExcludedNode", excludedBone) &&
                    excludedBone == nullptr && emptyReport.NodeCount == 0 &&
                    emptyReport.MeshInstanceCount == 1 && emptyReport.DistinctMeshCount == 1;
            }
            catch (const Content::ContentLoadException&)
            {
                emptySceneRefused = true;
            }
            ctx.Check(emptySceneRefused,
                      "an explicitly selected empty scene does not resurrect a mesh from another scene "
                      "(CNA-DEMOS-ISSUE-092)");
            ctx.Check(emptySceneRefused || exactEmptySceneLeak,
                      "the known empty-scene leak is isolated to one root-placed mesh with no imported source node");

            bool cycleRefused = false;
            std::string cycleMessage;
            try
            {
                (void)getContentProperty().Load<Model>("models/loose_scene_fallback_cycle");
            }
            catch (const Content::ContentLoadException& error)
            {
                cycleRefused = true;
                cycleMessage = error.what();
            }
            ctx.Check(cycleRefused && cycleMessage.find("structural validation") != std::string::npos,
                      "a cyclic hierarchy outside the selected scene is still rejected structurally");

            ready_ = true;
        }

        void Draw(const GameTime& gameTime) override
        {
            auto& device = getGraphicsDeviceProperty();
            const Color clear = Color::DarkSlateGray;
            device.Clear(clear);
            if (ready_)
            {
                device.setBlendStateProperty(BlendState::Opaque);
                device.setDepthStencilStateProperty(DepthStencilState::None);

                // glTF's counter-clockwise front uses the opposite fixed cull preset from XNA's
                // default. That works for ordinary and double-mirrored placements, but not an odd
                // mirror: the public diagnostic tells the application to reverse this per draw.
                device.setRasterizerStateProperty(RasterizerState::CullClockwise);
                model_->Draw(Matrix::getIdentityProperty(), Matrix::getIdentityProperty(),
                             Matrix::getIdentityProperty());
                if (!checked_)
                {
                    const CnaDemos::BackBufferPixels fixed(device);
                    Context().Check(IsOrange(At(fixed, Vector3(-.4f, .35f, 0))) &&
                                        CnaDemos::Near(At(fixed, Vector3(-.4f, -.35f, 0)), clear, 2),
                                    "one fixed glTF cull choice draws an ordinary placement but culls the odd mirror");
                }

                device.Clear(clear);
                for (const Placement& placement : placements_)
                {
                    device.setRasterizerStateProperty(placement.Mirrored
                        ? RasterizerState::CullCounterClockwise
                        : RasterizerState::CullClockwise);
                    for (Effect* effect : placement.Mesh->getEffectsProperty())
                    {
                        auto* basic = dynamic_cast<BasicEffect*>(effect);
                        if (basic == nullptr) continue;
                        basic->setWorldProperty(placement.World);
                        basic->setViewProperty(Matrix::getIdentityProperty());
                        basic->setProjectionProperty(Matrix::getIdentityProperty());
                    }
                    placement.Mesh->Draw();
                }

                if (!checked_)
                {
                    const CnaDemos::BackBufferPixels corrected(device);
                    bool allPlacements = true;
                    for (const Placement& placement : placements_)
                        allPlacements = allPlacements && IsOrange(At(corrected, placement.Center));
                    Context().Check(allPlacements,
                                    "per-placement determinant-aware culling draws all five imported placements");
                    Context().Check(CnaDemos::Near(At(corrected, Vector3::Zero), clear, 2),
                                    "the non-default scene's center decoy is absent");
                    Context().Check(CnaDemos::Near(At(corrected, Vector3(-.28f, -.65f, 0)), clear, 2) &&
                                        CnaDemos::Near(At(corrected, Vector3(.28f, -.65f, 0)), clear, 2),
                                    "unsupported GPU instance transforms do not masquerade as imported copies");
                    checked_ = true;
                }
            }
            Game::Draw(gameTime);
        }

    private:
        static Color At(const CnaDemos::BackBufferPixels& pixels, const Vector3& point)
        {
            return pixels.At(static_cast<int>((point.X + 1) * .5f * pixels.Width()),
                             static_cast<int>((1 - point.Y) * .5f * pixels.Height()));
        }

        static bool IsOrange(const Color& color)
        {
            return color.getRProperty() > 180 && color.getGProperty() > 70 &&
                   color.getGProperty() < 190 && color.getBProperty() < 40;
        }

        std::optional<Model> model_;
        std::vector<Matrix> absolute_;
        std::vector<Placement> placements_;
        bool ready_ = false;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(NodeInstancesDemo, {
    .id = "graphics.model.node_instances",
    .title = "Rigid glTF node instances",
    .description = "Imports one mesh through hierarchical TRS, an explicit matrix and odd/even mirrored placements; "
                   "demonstrates explicit, implicit-first, empty and no-scenes selection, reports unsupported GPU "
                   "instancing and applies the mirrored-winding diagnostic through per-placement culling.",
    .apis = {"ContentManager::Load", "Model::Bones", "Model::Meshes", "Model::Draw",
             "Model::CopyAbsoluteBoneTransformsTo", "Model::GltfImportReportEXT", "ModelBone::Transform",
             "ModelBone::Parent", "ModelMesh::ParentBone", "ModelMesh::Draw", "BasicEffect::World",
             "GltfImportReportEXT::NodeCount", "GltfImportReportEXT::MeshInstanceCount",
             "GltfImportReportEXT::DistinctMeshCount", "GltfImportReportEXT::SharedMeshCount",
             "GltfImportReportEXT::MaxNodeDepth", "GltfImportReportEXT::Diagnostics",
             "GltfImportDiagnosticEXT::Code", "GltfImportDiagnosticEXT::Subject",
             "RasterizerState::CullClockwise", "RasterizerState::CullCounterClockwise"},
    .tags = {"model", "gltf", "instances", "hierarchy", "transforms", "diagnostics", "culling"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_node_instances.gltf", "models/loose_scene_fallback_implicit.gltf",
               "models/loose_scene_fallback_nodes.gltf", "models/loose_scene_fallback_meshes.gltf",
               "models/loose_scene_fallback_empty.gltf", "models/loose_scene_fallback_cycle.gltf"},
    .restrictions = "EXT_mesh_gpu_instancing remains unsupported; CNA-DEMOS-ISSUE-092 records that an explicitly "
                    "selected empty scene incorrectly revives an unselected mesh at the identity root.",
    .knownIssue = "CNA-DEMOS-ISSUE-092",
})
