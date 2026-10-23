// SPDX-License-Identifier: MS-PL
// Compare the unchanged native factory with the logical adapter in separate fresh processes.
#include <array>
#include <chrono>
#include <iostream>
#include <gtest/gtest.h>
#include "CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"

namespace {
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
void CheckCachedPrecision(bool logical)
{
    PresentationParameters pp;
    pp.setBackBufferWidthProperty(64); pp.setBackBufferHeightProperty(64);
    pp.setDepthStencilFormatProperty(DepthFormat::None);
    GraphicsDevice device(GraphicsAdapter::getDefaultAdapterProperty(), GraphicsProfile::HiDef, pp);
    const auto lease = device.GetRenderer().AcquireThreadContextLeaseEXT();
    auto query = logical ? device.GetRenderer().CreateLogicalOcclusionQueryEXT()
                         : device.GetRenderer().CreateOcclusionQuery();
    ASSERT_NE(query, nullptr);
    const bool cached = query->PixelCountIsPreciseEXT();
    BasicEffect effect(device); effect.setVertexColorEnabledProperty(true);
    device.setRasterizerStateProperty(RasterizerState::CullNone);
    device.setDepthStencilStateProperty(DepthStencilState::None);
    device.Clear(Color::Black);
    const std::array<VertexPositionColor, 3> vertices = {
        VertexPositionColor(Vector3(-0.5f,-0.5f,0.5f),Color::White),
        VertexPositionColor(Vector3(0,0.5f,0.5f),Color::White),
        VertexPositionColor(Vector3(0.5f,-0.5f,0.5f),Color::White)};
    query->Begin(); effect.Apply();
    device.DrawUserPrimitives(PrimitiveType::TriangleList,vertices.data(),0,1);
    query->End();
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while (!query->IsComplete() && std::chrono::steady_clock::now()<deadline) device.Present();
    ASSERT_TRUE(query->IsComplete());
    const int count=query->PixelCount();
    const bool current=query->PixelCountIsPreciseEXT();
    std::cout << "logical=" << logical << " cached=" << cached << " current=" << current << " count=" << count << '\n';
    EXPECT_TRUE(current ? count>1 : count==1);
    EXPECT_TRUE(cached ? count>1 : count==1) << "The original C demo cached precision before the first Begin";
}
TEST(PrecisionCacheObservation, RawNativeFactory) { CheckCachedPrecision(false); }
TEST(PrecisionCacheObservation, LogicalFactory) { CheckCachedPrecision(true); }
}
