// SPDX-License-Identifier: MS-PL
// ReadAsset lets a level owner retain and dispose nested resources independently of its manager.
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/IDisposable.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework::Content;

namespace
{
    constexpr const char* LevelReaderName = "CnaDemos.OwnedLevelReader";
    constexpr const char* TilesReaderName = "CnaDemos.OwnedTilesReader";
    struct Tiles final : System::IDisposable
    {
        std::vector<std::uint8_t> bytes;
        bool disposed = false;
        void Dispose() override { std::vector<std::uint8_t>().swap(bytes); disposed = true; }
    };
    using TilesPtr = std::shared_ptr<Tiles>;
    struct Level { std::string name; TilesPtr tiles, absent; };
    class TilesReader final : public ContentTypeReader<TilesPtr>
    {
    public:
        TilesReader() : ContentTypeReader<TilesPtr>("CnaDemos.OwnedTiles") {}
    protected:
        TilesPtr Read(ContentReader& input, std::optional<TilesPtr>) override
        {
            const int count = input.ReadInt32();
            if (count < 0 || count > 1024) throw ContentLoadException("Invalid tile byte count");
            auto tiles = std::make_shared<Tiles>();
            tiles->bytes = input.ReadBytes(count);
            if (tiles->bytes.size() != static_cast<std::size_t>(count)) throw ContentLoadException("Truncated tile data");
            return tiles;
        }
    };
    class LevelReader final : public ContentTypeReader<Level>
    {
    public:
        LevelReader() : ContentTypeReader<Level>("CnaDemos.OwnedLevel") {}
    protected:
        Level Read(ContentReader& input, std::optional<Level>) override
        {
            Level level;
            level.name = input.ReadString();
            level.tiles = input.ReadObject<TilesPtr>();
            level.absent = input.ReadObject<TilesPtr>();
            return level;
        }
    };
    struct ScopedReaders
    {
        ~ScopedReaders()
        {
            ContentTypeReaderManager::RemoveTypeCreatorEXT(LevelReaderName);
            ContentTypeReaderManager::RemoveTypeCreatorEXT(TilesReaderName);
        }
    };
    struct StreamObservations { int opened = 0, destroyed = 0, reads = 0, bytes = 0; };
    class PackStream final : public System::IO::MemoryStream
    {
    public:
        PackStream(const std::vector<std::uint8_t>& bytes, StreamObservations& observed)
            : MemoryStream(bytes.data(), static_cast<int>(bytes.size()), false), observed_(observed) {}
        ~PackStream() override { ++observed_.destroyed; }
        int Read(std::uint8_t* buffer, int offset, int count) override
        {
            ++observed_.reads;
            const int read = MemoryStream::Read(buffer, offset, std::min(count, 7));
            observed_.bytes += read;
            return read;
        }
    private:
        StreamObservations& observed_;
    };
    class PackManager final : public ContentManager
    {
    public:
        explicit PackManager(std::vector<std::uint8_t> bytes) : bytes_(std::move(bytes)) {}
        StreamObservations streams;
        Level ReadOwned(const std::string& name, std::function<void(std::shared_ptr<System::IDisposable>)> record)
        {
            return ReadAsset<Level>(name, std::move(record));
        }
    protected:
        std::unique_ptr<System::IO::Stream> OpenStream(const std::string& name) override
        {
            if (name != "level") throw ContentLoadException("Unknown packed level");
            ++streams.opened;
            return std::make_unique<PackStream>(bytes_, streams);
        }
    private:
        std::vector<std::uint8_t> bytes_;
    };

    void ReadAssetOwnership(CnaDemos::DemoContext& ctx)
    {
        if (!ctx.Check(!ContentTypeReaderManager::IsRegistered(LevelReaderName) &&
                       !ContentTypeReaderManager::IsRegistered(TilesReaderName), "both demo reader keys start unregistered")) return;
        ScopedReaders cleanup;
        ContentTypeReaderManager::AddTypeCreator(LevelReaderName, [] { return std::make_unique<LevelReader>(); });
        ContentTypeReaderManager::AddTypeCreator(TilesReaderName, [] { return std::make_unique<TilesReader>(); });
        const auto file = Microsoft::Xna::Framework::TitleContainer::OpenStream("assets/ownership/level.xnb");
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(file->getLengthProperty()));
        int offset = 0;
        while (offset < static_cast<int>(bytes.size()))
        {
            const int read = file->Read(bytes.data(), offset, static_cast<int>(bytes.size()) - offset);
            if (read == 0) throw ContentLoadException("Truncated authored level fixture");
            offset += read;
        }
        const auto sourceSize = static_cast<int>(bytes.size());
        PackManager pack(bytes); // retain the same authored bytes for the default-recorder comparison
        // The caller's resource list owns the callback results after each ContentReader goes away.
        std::vector<std::shared_ptr<System::IDisposable>> owned;
        const auto record = [&](std::shared_ptr<System::IDisposable> resource) { owned.push_back(std::move(resource)); };
        auto first = pack.ReadOwned("level", record);
        const std::vector<std::uint8_t> expected{1, 1, 0, 2, 0, 3, 3, 0};
        if (!ctx.Check(first.name == "Courtyard" && first.tiles && first.tiles->bytes == expected && !first.absent,
                       "the packed level restores its authored tile bytes and null optional resource")) return;
        ctx.Check(owned.size() == 1 && owned.front() == first.tiles,
                  "the callback receives the actual nested disposable, excluding the value root and null child");
        ctx.Check(pack.streams.opened == 1 && pack.streams.destroyed == 1 && pack.streams.bytes == sourceSize && pack.streams.reads > 1,
                  "ReadAsset consumes short stream reads and destroys its owned source stream before returning");
        auto second = pack.ReadOwned("level", record);
        if (!ctx.Check(second.tiles && second.tiles != first.tiles && second.tiles->bytes == expected && pack.streams.opened == 2,
                       "reading the same name again constructs a fresh resource instead of reusing the first asset")) return;
        ctx.Check(owned.size() == 2 && owned[1] == second.tiles && pack.streams.destroyed == 2 && pack.streams.bytes == 2 * sourceSize,
                  "each read independently records its nested resource and releases its source stream");
        first.tiles->bytes[0] = 9;
        ctx.Check(second.tiles->bytes[0] == 1, "independently read resources do not share mutable tile storage");
        pack.Unload();
        ctx.Check(!first.tiles->disposed && !second.tiles->disposed && first.tiles->bytes[0] == 9 && second.tiles->bytes == expected,
                  "manager Unload leaves the explicitly delegated resources under the caller's control");
        bool emptyRefused = false;
        try { (void)pack.ReadOwned("", record); }
        catch (const System::ArgumentNullException&) { emptyRefused = true; }
        ctx.Check(emptyRefused && pack.streams.opened == 2 && owned.size() == 2,
                  "an empty asset name is refused before opening a stream or recording resources");
        pack.Dispose();
        ctx.Check(!first.tiles->disposed && !second.tiles->disposed, "manager disposal also preserves explicitly delegated resources");
        bool disposedRefused = false;
        try { (void)pack.ReadOwned("level", record); }
        catch (const System::ObjectDisposedException&) { disposedRefused = true; }
        ctx.Check(disposedRefused && pack.streams.opened == 2 && owned.size() == 2,
                  "a disposed manager refuses another read before stream creation or callback invocation");
        for (const auto& resource : owned) resource->Dispose();
        ctx.Check(first.tiles->disposed && second.tiles->disposed && first.tiles->bytes.empty() && second.tiles->bytes.empty(),
                  "the caller disposes the recorded resources through IDisposable and releases their tile buffers");
        const std::weak_ptr<Tiles> firstLifetime = first.tiles, secondLifetime = second.tiles;
        first.tiles.reset(); second.tiles.reset();
        ctx.Check(!firstLifetime.expired() && !secondLifetime.expired(), "the callback's list owns the resources after asset values release them");
        owned.clear();
        ctx.Check(firstLifetime.expired() && secondLifetime.expired(), "releasing the caller's list destroys both resources without hidden manager ownership");

        // An empty callback delegates disposal tracking back to ContentManager by its public
        // ReadAsset contract. Keep the loaded child alive so Unload's Dispose call is observable.
        PackManager automatic(std::move(bytes));
        auto managed = automatic.ReadOwned("level", {});
        if (!ctx.Check(managed.tiles && managed.tiles->bytes == expected && !managed.tiles->disposed,
                       "omitting the callback still decodes a fresh usable nested disposable")) return;
        automatic.Unload();
        ctx.Check(managed.tiles->disposed,
                  "ReadAsset without a callback records the disposable for manager Unload (CNA-DEMOS-ISSUE-011)");
        // Cleanup follows the assertion; it does not turn a missing manager Dispose into a pass.
        if (!managed.tiles->disposed) managed.tiles->Dispose();

    }
}

CNA_DEMOS_CONSOLE(ReadAssetOwnership, {
    .id = "content.content_manager.read_asset_ownership",
    .title = "Control packed asset lifetime",
    .description = "Reads a packed level repeatedly through ReadAsset, records its nested disposable tiles, "
                   "and compares caller-owned lifetime with the default recorder across Unload while releasing short-read source streams.",
    .apis = {"ContentManager::ReadAsset", "ContentManager::OpenStream", "ContentManager::Unload", "ContentManager::Dispose",
             "ContentReader::ReadObject", "ContentTypeReader::Read", "ContentTypeReaderManager::AddTypeCreator"},
    .tags = {"content", "xnb", "pack-file", "lifetime", "dispose", "callback"},
    .assets = {"ownership/level.xnb"},
    .restrictions = "Supplied and absent callbacks with typed shared_ptr IDisposable children; default tracking reproduces ISSUE-011. Raw/untyped reads and callback deduplication remain separate contracts.",
    .knownIssue = "CNA-DEMOS-ISSUE-011",
})
