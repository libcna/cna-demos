// SPDX-License-Identifier: MS-PL
// ISSUE-011: retain CNA's independent wrappers while enforcing manager ownership.
#include <gtest/gtest.h>
#include <utility>
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/ObjectDisposedException.hpp"

using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::Content::ContentManager;
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
using Microsoft::Xna::Framework::Graphics::Texture2D;

namespace
{
    class TextureOwnershipManager final : public ContentManager
    {
    public:
        using ContentManager::Dispose;
        explicit TextureOwnershipManager(GraphicsDevice& device)
            : ContentManager(nullptr, "tests/assets/xnb/monogame/windows/uncompressed")
        {
            setGraphicsDevice(device);
        }
    };

    class ContentTextureOwnershipTest : public ::testing::Test
    {
    protected:
        GraphicsDevice device;

        static void ExpectWhite(Texture2D& texture)
        {
            ASSERT_FALSE(texture.getIsDisposedProperty());
            Color pixel;
            ASSERT_NO_THROW(texture.GetData(&pixel, 1));
            EXPECT_EQ(pixel, Color::White);
        }

        static void ExpectDisposed(Texture2D& texture)
        {
            EXPECT_TRUE(texture.getIsDisposedProperty());
            Color pixel;
            EXPECT_THROW(texture.GetData(&pixel, 1), System::ObjectDisposedException);
        }
    };
}

TEST_F(ContentTextureOwnershipTest, UnloadDisposesBothCachedResultsAndReleasesTheirRenderer)
{
    TextureOwnershipManager manager(device);
    auto first = manager.Load<Texture2D>("white-1");
    auto second = manager.Load<Texture2D>("white-1");
    ExpectWhite(first);
    ExpectWhite(second);
    const auto renderer = first.GetRendererWeak();
    ASSERT_FALSE(renderer.expired());
    EXPECT_EQ(renderer.lock(), second.GetRendererWeak().lock());

    manager.Unload();
    ExpectDisposed(first);
    ExpectDisposed(second);
    EXPECT_TRUE(renderer.expired());

    auto reloaded = manager.Load<Texture2D>("white-1");
    ExpectWhite(reloaded);
    manager.Unload();
    ExpectDisposed(reloaded);
}

TEST_F(ContentTextureOwnershipTest, UnloadingOneManagerDoesNotDisposeAnotherManagersTexture)
{
    TextureOwnershipManager firstManager(device);
    TextureOwnershipManager secondManager(device);
    auto first = firstManager.Load<Texture2D>("white-1");
    auto second = secondManager.Load<Texture2D>("white-1");
    EXPECT_NE(first.GetRendererWeak().lock(), second.GetRendererWeak().lock());

    firstManager.Unload();
    ExpectDisposed(first);
    ExpectWhite(second);
    secondManager.Unload();
    ExpectDisposed(second);
}

TEST_F(ContentTextureOwnershipTest, WrapperCopyAndMovePreserveTheManagersScopeNotExplicitDisposal)
{
    TextureOwnershipManager manager(device);
    auto original = manager.Load<Texture2D>("white-1");
    Texture2D copied(original);
    Texture2D moved(std::move(copied));
    Texture2D assigned;
    assigned = moved;
    Texture2D moveAssigned;
    moveAssigned = std::move(assigned);

    // CNA's documented value-copy extension: manually disposing one wrapper does
    // not dispose the others. Manager unload is the distinct owning operation.
    original.Dispose();
    ExpectDisposed(original);
    ExpectWhite(moved);
    ExpectWhite(moveAssigned);
    manager.Unload();
    ExpectDisposed(moved);
    ExpectDisposed(moveAssigned);
}

TEST_F(ContentTextureOwnershipTest, NonManagedDisposeDropsOwnershipWithoutDisposingTheTexture)
{
    TextureOwnershipManager manager(device);
    auto texture = manager.Load<Texture2D>("white-1");
    manager.Dispose(false);
    ExpectWhite(texture);
    manager.Dispose();
    ExpectWhite(texture);
}
