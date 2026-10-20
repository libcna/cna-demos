// SPDX-License-Identifier: MS-PL
// A ContentManager that serves assets from memory by overriding OpenStream -- XNA's extension
// point for pack files and encrypted content (CNA-DEMOS-ISSUE-012 in CNA).
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Content;

namespace
{
    // Assets kept in memory under short names -- in a real game, read from one pack file.
    class PackContentManager final : public ContentManager
    {
    public:
        explicit PackContentManager(System::IServiceProvider* services) : ContentManager(services, "pack")
        {
            const std::unique_ptr<System::IO::Stream> file =
                TitleContainer::OpenStream("assets/xnb/textures/colour_wheel.xnb");
            std::vector<std::uint8_t>& bytes = files_["wheel"];
            bytes.resize(static_cast<std::size_t>(file->getLengthProperty()));
            file->Read(bytes.data(), 0, static_cast<int>(bytes.size()));
        }

        int opened = 0;

        // ReadAsset is the protected step Load uses to read an asset through OpenStream.
        template <typename T>
        T ReadFromPack(const std::string& name)
        {
            return ReadAsset<T>(name, {});
        }

    protected:
        std::unique_ptr<System::IO::Stream> OpenStream(const std::string& assetName) override
        {
            ++opened;
            const auto found = files_.find(assetName);
            if (found == files_.end())
                throw ContentLoadException("not in the pack: " + assetName);
            return std::make_unique<System::IO::MemoryStream>(
                reinterpret_cast<const SharpRuntime::bytecs*>(found->second.data()),
                static_cast<int>(found->second.size()), false);
        }

    private:
        std::map<std::string, std::vector<std::uint8_t>> files_;
    };

    class OpenStreamDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            PackContentManager pack(&getServicesProperty());
            Context().Check(pack.getServiceProviderProperty() == &getServicesProperty(),
                            "the manager retains the exact service provider supplied to its constructor");

            // ReadAsset reaches OpenStream: the texture comes out of memory.
            const Texture2D direct = pack.ReadFromPack<Texture2D>("wheel");
            Context().Print("ReadAsset: {}x{}, OpenStream called {} time(s)", direct.getWidthProperty(),
                            direct.getHeightProperty(), pack.opened);
            Context().Check(direct.getWidthProperty() == 128 && pack.opened == 1, "ReadAsset reads through OpenStream");

            // In XNA, Load<T> -- what the rest of the game calls -- also reads through OpenStream,
            // so a derived manager changes where every asset comes from.
            bool loaded = false;
            try
            {
                loaded = pack.Load<Texture2D>("wheel").getWidthProperty() == 128;
            }
            catch (const std::exception& e)
            {
                Context().Print("Load: {}", e.what());
            }
            Context().Print("OpenStream called {} time(s) in total", pack.opened);
            Context().Check(loaded && pack.opened == 2, "Load<T> reads through the OpenStream override");

            const int openedBeforeMissing = pack.opened;
            bool missing = false;
            try
            {
                (void)pack.ReadFromPack<Texture2D>("missing");
            }
            catch (const ContentLoadException& error)
            {
                missing = std::string(error.what()) == "not in the pack: missing";
            }
            Context().Check(missing && pack.opened == openedBeforeMissing + 1,
                            "the override reports a missing packed asset with ContentLoadException");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            Game::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(OpenStreamDemo, {
    .id = "content.content_manager.open_stream_override",
    .title = "Assets from memory (OpenStream)",
    .description = "Derives a ContentManager whose OpenStream serves XNB assets from memory, reads one with "
                   "ReadAsset, and checks XNA's rule that Load<T> goes through the override too.",
    .apis = {"ContentManager::OpenStream", "ContentManager::ReadAsset", "ContentManager::Load",
             "ContentManager::ServiceProvider", "ContentLoadException::ContentLoadException"},
    .tags = {"content", "pack-file", "stream", "custom-manager", "xnb"},
    .frames = 1,
    .assets = {"xnb/textures/colour_wheel.xnb"},
})
