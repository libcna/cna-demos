// SPDX-License-Identifier: MS-PL
// A complete public-C++ Model graph built without the content pipeline.
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct ProceduralVertex
    {
        Vector3 position;
        std::uint32_t color;
    };

    struct LifetimeSentinel
    {
        bool* graphDestroyed = nullptr;
        ~LifetimeSentinel()
        {
            if (graphDestroyed != nullptr)
                *graphDestroyed = true;
        }
    };

    // Model, ModelMesh, ModelMeshPart and ModelBone retain raw non-owning pointers. This owner is
    // deliberately declared from longest-lived dependencies to shortest-lived graph roots. C++
    // destroys members in reverse declaration order: Model first, then mesh/bones/parts/effects,
    // and the GPU buffers last. Moving any owning object after publishing its address would break
    // the graph, so every pointee is separately heap allocated and the owner itself is non-moving.
    class OwnedProceduralModel final
    {
    public:
        explicit OwnedProceduralModel(GraphicsDevice& device, bool* graphDestroyed = nullptr)
            : sentinel_{graphDestroyed}
            , declaration_(
                  static_cast<int>(sizeof(ProceduralVertex)),
                  {VertexElement(static_cast<int>(offsetof(ProceduralVertex, position)),
                                 VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
                   VertexElement(static_cast<int>(offsetof(ProceduralVertex, color)),
                                 VertexElementFormat::Color, VertexElementUsage::Color, 0)})
        {
            static_assert(sizeof(ProceduralVertex) == 16);

            constexpr std::array<float, 3> centers = {-0.65f, 0.0f, 0.65f};
            const std::array<Color, 3> colors = {Color::Red, Color::Lime, Color::Blue};
            constexpr std::array<std::uint16_t, 6> indices = {0, 1, 2, 0, 2, 3};

            vertexBuffers_.reserve(3);
            indexBuffers_.reserve(3);
            parts_.reserve(3);
            effects_.reserve(2);
            bones_.reserve(2);

            for (std::size_t i = 0; i < centers.size(); ++i)
            {
                constexpr float half = 0.22f;
                const float x = centers[i];
                const std::uint32_t packed = colors[i].getPackedValueProperty();
                const std::array<ProceduralVertex, 4> vertices = {
                    ProceduralVertex{Vector3(x - half, -half, 0.0f), packed},
                    ProceduralVertex{Vector3(x + half, -half, 0.0f), packed},
                    ProceduralVertex{Vector3(x + half, half, 0.0f), packed},
                    ProceduralVertex{Vector3(x - half, half, 0.0f), packed},
                };

                auto verticesBuffer = std::make_unique<VertexBuffer>(
                    device, declaration_, static_cast<int>(vertices.size()), BufferUsage::WriteOnly);
                verticesBuffer->SetData(vertices.data(), static_cast<int>(vertices.size()));
                vertexBuffers_.push_back(std::move(verticesBuffer));

                auto indexBuffer = std::make_unique<IndexBuffer>(
                    device, IndexElementSize::SixteenBits, static_cast<int>(indices.size()),
                    BufferUsage::WriteOnly);
                indexBuffer->SetData(indices.data(), static_cast<int>(indices.size()));
                indexBuffers_.push_back(std::move(indexBuffer));
            }

            effects_.push_back(std::make_unique<BasicEffect>(device));
            effects_.push_back(std::make_unique<BasicEffect>(device));
            for (const auto& effect : effects_)
                effect->setVertexColorEnabledProperty(true);

            for (std::size_t i = 0; i < vertexBuffers_.size(); ++i)
            {
                auto part = std::make_unique<ModelMeshPart>(
                    vertexBuffers_[i].get(), indexBuffers_[i].get(), 4, 2, 0, 0);
                // Parts zero and one deliberately share an Effect. ModelMesh must register that
                // pointer once, while the third part contributes the second distinct Effect.
                part->setEffectProperty(effects_[i < 2 ? 0 : 1].get());
                parts_.push_back(std::move(part));
            }

            std::vector<ModelMeshPart*> partPointers;
            for (const auto& part : parts_)
                partPointers.push_back(part.get());
            mesh_ = std::make_unique<ModelMesh>(&device, "Three procedural panels", partPointers);

            // Do not call mesh_->getEffectsPropertyMutable().Add here. The public ModelMesh
            // constructor registers each distinct part Effect. Add() is intentionally a raw
            // collection operation and adding either pointer again would create a duplicate,
            // causing Model::Draw to update the same effect more than once.

            bones_.push_back(std::make_unique<ModelBone>(0, "Root"));
            bones_.push_back(std::make_unique<ModelBone>(1, "Panel placement"));
            bones_[0]->AddChild(bones_[1].get());
            bones_[1]->setTransformProperty(Matrix::CreateTranslation(0.0f, 0.10f, 0.0f));

            model_ = std::make_unique<Model>(
                &device,
                std::vector<ModelBone*>{bones_[0].get(), bones_[1].get()},
                std::vector<ModelMesh*>{mesh_.get()},
                std::vector<ModelBone*>{bones_[1].get()}, 0);
        }

        OwnedProceduralModel(const OwnedProceduralModel&) = delete;
        OwnedProceduralModel& operator=(const OwnedProceduralModel&) = delete;
        OwnedProceduralModel(OwnedProceduralModel&&) = delete;
        OwnedProceduralModel& operator=(OwnedProceduralModel&&) = delete;

        [[nodiscard]] Model& ModelValue() const { return *model_; }
        [[nodiscard]] ModelMesh& Mesh() const { return *mesh_; }
        [[nodiscard]] ModelBone& Root() const { return *bones_[0]; }
        [[nodiscard]] ModelBone& PlacementBone() const { return *bones_[1]; }
        [[nodiscard]] const VertexDeclaration& Declaration() const { return declaration_; }
        [[nodiscard]] VertexBuffer& VertexBufferAt(std::size_t i) const { return *vertexBuffers_[i]; }
        [[nodiscard]] IndexBuffer& IndexBufferAt(std::size_t i) const { return *indexBuffers_[i]; }

    private:
        LifetimeSentinel sentinel_;
        VertexDeclaration declaration_;
        std::vector<std::unique_ptr<VertexBuffer>> vertexBuffers_;
        std::vector<std::unique_ptr<IndexBuffer>> indexBuffers_;
        std::vector<std::unique_ptr<BasicEffect>> effects_;
        std::vector<std::unique_ptr<ModelMeshPart>> parts_;
        std::vector<std::unique_ptr<ModelBone>> bones_;
        std::unique_ptr<ModelMesh> mesh_;
        std::unique_ptr<Model> model_;
    };

    class ProceduralConstructionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // A second, short-lived graph is the lifetime regression. The sentinel is the first
            // member and therefore dies last, after every object that participates in raw links.
            bool probeDestroyed = false;
            {
                OwnedProceduralModel probe(device, &probeDestroyed);
                Context().Check(probe.Mesh().getMeshPartsProperty().getCountProperty() == 3 &&
                                    probe.Mesh().getEffectsProperty().getCountProperty() == 2,
                                "a temporary graph deduplicates one shared effect across three parts");
            }
            Context().Check(probeDestroyed,
                            "the separate lifetime probe destroys the complete non-owning graph in dependency order");

            graph_ = std::make_unique<OwnedProceduralModel>(device, &graphDestroyed_);
            Context().Check(graph_->Declaration().getVertexStrideProperty() ==
                                    static_cast<int>(sizeof(ProceduralVertex)) &&
                                graph_->VertexBufferAt(0).getVertexCountProperty() == 4 &&
                                graph_->IndexBufferAt(0).getIndexCountProperty() == 6,
                            "the explicit position/color declaration backs four vertices and six indices per part");
            Context().Check(graph_->Mesh().getMeshPartsProperty().getCountProperty() == 3 &&
                                graph_->Mesh().getEffectsProperty().getCountProperty() == 2,
                            "ModelMesh retains three parts and exactly two distinct automatically registered effects");
            Context().Check(graph_->ModelValue().getRootProperty() == &graph_->Root() &&
                                graph_->PlacementBone().getParentProperty() == &graph_->Root() &&
                                graph_->Root().getChildrenProperty().getCountProperty() == 1 &&
                                graph_->Mesh().getParentBoneProperty() == &graph_->PlacementBone(),
                            "the Model root, child hierarchy and mesh parent-bone association are coherent");
        }

        void UnloadContent() override
        {
            graph_.reset();
            Context().Check(graphDestroyed_,
                            "Model and every raw-linked dependency are released once without an escaping pointer");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            graph_->ModelValue().Draw(Matrix::getIdentityProperty(), Matrix::getIdentityProperty(),
                                      Matrix::getIdentityProperty());

            if (!pixelsChecked_)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const int y = static_cast<int>(0.45f * pixels.Height());
                Context().Check(CnaDemos::Near(pixels.At(static_cast<int>(0.175f * pixels.Width()), y),
                                               Color::Red, 2),
                                "the first ModelMeshPart renders red through Model::Draw");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 2, y), Color::Lime, 2),
                                "the shared-effect second ModelMeshPart renders green");
                Context().Check(CnaDemos::Near(pixels.At(static_cast<int>(0.825f * pixels.Width()), y),
                                               Color::Blue, 2),
                                "the distinct-effect third ModelMeshPart renders blue");
                pixelsChecked_ = true;
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<OwnedProceduralModel> graph_;
        bool graphDestroyed_ = false;
        bool pixelsChecked_ = false;
    };
}

CNA_DEMOS_GAME(ProceduralConstructionDemo, {
    .id = "graphics.model.procedural_construction",
    .title = "Construct a Model by hand",
    .description = "Builds explicit vertex declarations, vertex/index buffers, effects, three ModelMeshParts, one "
                   "ModelMesh and a two-bone hierarchy, assembles them into a public C++ Model and pixel-checks "
                   "Model::Draw; a separate RAII probe verifies the raw-pointer lifetime order.",
    .apis = {"VertexDeclaration::VertexDeclaration", "VertexBuffer::VertexBuffer", "VertexBuffer::SetData",
             "IndexBuffer::IndexBuffer", "IndexBuffer::SetData", "BasicEffect::BasicEffect",
             "ModelMeshPart::ModelMeshPart", "ModelMeshPart::Effect", "ModelMesh::ModelMesh",
             "ModelMesh::MeshParts", "ModelMesh::Effects", "ModelMesh::ParentBone", "ModelBone::ModelBone",
             "ModelBone::Children", "ModelBone::Parent", "ModelBone::Transform", "Model::Model",
             "Model::Root", "Model::Bones", "Model::Meshes", "Model::Draw"},
    .tags = {"model", "procedural", "mesh", "mesh-part", "bone", "effect", "ownership", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
