// SPDX-License-Identifier: MS-PL
// ISSUE-011: resources nested below non-IDisposable content roots.
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "System/ObjectDisposedException.hpp"

using Microsoft::Xna::Framework::Content::ContentManager;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ContentNestedOwnershipTest : public ::testing::Test
    {
    protected:
        GraphicsDevice device;
        ContentManager manager{nullptr, "tests/assets/xnb/monogame/windows/uncompressed"};
        void SetUp() override { manager.setGraphicsDevice(device); }
    };
}

TEST_F(ContentNestedOwnershipTest, UnloadDisposesSpriteFontAtlasAndReleasesRenderer)
{
    auto font = manager.Load<SpriteFont>("Default");
    const auto& atlas = font.getTextureEXT();
    ASSERT_FALSE(atlas.getIsDisposedProperty());
    const auto renderer = atlas.GetRendererWeak();
    ASSERT_FALSE(renderer.expired());
    const auto width = static_cast<std::size_t>(atlas.getWidthProperty());
    const auto height = static_cast<std::size_t>(atlas.getHeightProperty());
    ASSERT_TRUE(atlas.getFormatProperty() == SurfaceFormat::Dxt3 ||
                atlas.getFormatProperty() == SurfaceFormat::Color);
    const auto byteCount = atlas.getFormatProperty() == SurfaceFormat::Dxt3
        ? ((width + 3) / 4) * ((height + 3) / 4) * 16 : width * height * 4;
    std::vector<std::uint8_t> data(byteCount);
    ASSERT_NO_THROW(atlas.GetData(data.data(), static_cast<int>(data.size())));
    manager.Unload();
    EXPECT_TRUE(atlas.getIsDisposedProperty());
    EXPECT_TRUE(renderer.expired());
    EXPECT_THROW(atlas.GetData(data.data(), static_cast<int>(data.size())), System::ObjectDisposedException);
}

TEST_F(ContentNestedOwnershipTest, UnloadDisposesModelSharedBuffersAndEffect)
{
    auto model = manager.Load<Model>("BlenderDefaultCube");
    ASSERT_GT(model.getMeshesProperty().getCountProperty(), 0);
    auto* mesh = model.getMeshesProperty()[0];
    ASSERT_NE(mesh, nullptr);
    ASSERT_GT(mesh->getMeshPartsProperty().getCountProperty(), 0);
    auto* part = mesh->getMeshPartsProperty()[0];
    ASSERT_NE(part, nullptr);
    auto* vertices = part->getVertexBufferProperty();
    auto* indices = part->getIndexBufferProperty();
    auto* effect = part->getEffectProperty();
    ASSERT_NE(vertices, nullptr);
    ASSERT_NE(indices, nullptr);
    ASSERT_NE(effect, nullptr);
    ASSERT_FALSE(vertices->getIsDisposedProperty());
    ASSERT_FALSE(indices->getIsDisposedProperty());
    ASSERT_FALSE(effect->getIsDisposedProperty());
    manager.Unload();
    EXPECT_TRUE(vertices->getIsDisposedProperty());
    EXPECT_TRUE(indices->getIsDisposedProperty());
    EXPECT_TRUE(effect->getIsDisposedProperty());
}

TEST_F(ContentNestedOwnershipTest, UnloadDisposesLooseTextureAndReleasesRenderer)
{
    manager.setRootDirectoryProperty(std::string("tests/assets/xna40/texture"));
    auto texture = manager.Load<Texture2D>("probe_3x2.png");
    ASSERT_FALSE(texture.getIsDisposedProperty());
    std::vector<Microsoft::Xna::Framework::Color> pixels(
        texture.getWidthProperty() * texture.getHeightProperty());
    ASSERT_NO_THROW(texture.GetData(pixels.data(), static_cast<int>(pixels.size())));
    const auto renderer = texture.GetRendererWeak();
    ASSERT_FALSE(renderer.expired());
    manager.Unload();
    EXPECT_TRUE(texture.getIsDisposedProperty());
    EXPECT_TRUE(renderer.expired());
    EXPECT_THROW(texture.GetData(pixels.data(), static_cast<int>(pixels.size())),
                 System::ObjectDisposedException);
}

TEST(ContentAudioOwnershipTest, UnloadDisposesMovedSoundOwnerAndItsInstance)
{
#ifndef SOUND_ENABLED
    GTEST_SKIP() << "No audio implementation in this build.";
#else
    using Microsoft::Xna::Framework::Audio::SoundEffect;
    ContentManager manager(nullptr, "tests/assets/xnb/monogame/windows/uncompressed/audio");
    auto original = manager.Load<SoundEffect>("tone_mono_44khz_16bit");
    ASSERT_FALSE(original.getIsDisposedProperty());
    auto instance = original.CreateInstance();
    ASSERT_FALSE(instance.getIsDisposedProperty());
    SoundEffect moved(std::move(original));
    ASSERT_FALSE(moved.getIsDisposedProperty());
    manager.Unload();
    EXPECT_TRUE(moved.getIsDisposedProperty());
    EXPECT_TRUE(instance.getIsDisposedProperty());
    EXPECT_THROW((void)moved.CreateInstance(), System::ObjectDisposedException);
#endif
}
