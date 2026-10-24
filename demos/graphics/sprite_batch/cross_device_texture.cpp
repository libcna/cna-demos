// SPDX-License-Identifier: MS-PL
// A SpriteBatch may draw only textures created by its own GraphicsDevice.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentInterval.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Exception.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    PresentationParameters SmallPresentation()
    {
        PresentationParameters parameters;
        parameters.setBackBufferWidthProperty(32);
        parameters.setBackBufferHeightProperty(32);
        parameters.setIsFullScreenProperty(false);
        parameters.setDepthStencilFormatProperty(DepthFormat::None);
        parameters.setPresentationIntervalProperty(PresentInterval::Immediate);
        return parameters;
    }

    void CrossDeviceTexture(CnaDemos::DemoContext& ctx)
    {
        PresentationParameters firstParameters = SmallPresentation();
        PresentationParameters secondParameters = SmallPresentation();
        GraphicsDevice first(GraphicsAdapter::getDefaultAdapterProperty(),
                             GraphicsProfile::HiDef, firstParameters);
        GraphicsDevice second(GraphicsAdapter::getDefaultAdapterProperty(),
                              GraphicsProfile::HiDef, secondParameters);
        Texture2D ownTexture(first, 1, 1);
        Texture2D foreignTexture(second, 1, 1);
        SpriteBatch batch(first);

        batch.Begin();
        bool rejectedAtDraw = false;
        std::string refusal;
        try
        {
            batch.Draw(foreignTexture, Vector2::Zero, Color::White);
        }
        catch (const System::InvalidOperationException& error)
        {
            rejectedAtDraw = true;
            refusal = error.what();
        }
        ctx.Check(rejectedAtDraw && refusal.find("different GraphicsDevice") != std::string::npos,
                  "Draw rejects a texture owned by another GraphicsDevice");

        bool batchStillUsable = true;
        try
        {
            batch.Draw(ownTexture, Vector2::Zero, Color::White);
            batch.End();
            batch.Begin();
            batch.End();
        }
        catch (const System::Exception& error)
        {
            batchStillUsable = false;
            ctx.Print("same-device recovery failed: {}", error.what());
        }
        ctx.Check(batchStillUsable,
                  "the refusal queues nothing and the same-device batch can End and begin again");

        second.Dispose();
        first.Dispose();
    }
}

CNA_DEMOS_CONSOLE(CrossDeviceTexture, {
    .id = "graphics.sprite_batch.cross_device_texture",
    .title = "Reject a texture from another device",
    .description = "Creates two graphics devices, rejects the second device's texture at Draw, then proves the first "
                   "device's SpriteBatch remains usable with its own texture.",
    .apis = {"GraphicsDevice::GraphicsDevice", "Texture2D::Texture2D", "SpriteBatch::SpriteBatch",
             "SpriteBatch::Begin", "SpriteBatch::Draw", "SpriteBatch::End"},
    .tags = {"sprite", "device", "ownership", "errors", "lifetime"},
    .restrictions = "Validates public refusal and batch recovery; it does not compare pixels or exercise device loss.",
})
