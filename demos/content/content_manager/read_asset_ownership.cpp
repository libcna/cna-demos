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
    struct Level { std::string name; TilesPtr tiles, alias, absent; };
    class TilesReader final : public ContentTypeReader<TilesPtr>
    {
    public:
        TilesReader() : ContentTypeReader<TilesPtr>("CnaDemos.OwnedTiles") {}
    protected:
        TilesPtr Read(ContentReader& input, std::optional<TilesPtr>) override
        {
            const int count = input.ReadInt32();
            if (count < 0 || count > 1024) throw ContentLoadException("Invalid tile byte count");
            auto bytes = input.ReadBytes(count);
            if (bytes.size() != static_cast<std::size_t>(count)) throw ContentLoadException("Truncated tile data");
            if (auto tiles = canonical_.lock(); tiles && tiles->bytes == bytes) return tiles;
            auto tiles = std::make_shared<Tiles>();
            tiles->bytes = std::move(bytes);
            canonical_ = tiles;
            return tiles;
        }
    private:
        std::weak_ptr<Tiles> canonical_;
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
            level.alias = input.ReadObject<TilesPtr>();
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
        using ContentManager::Dispose;

        explicit PackManager(std::vector<std::uint8_t> bytes) : bytes_(std::move(bytes)) {}
        StreamObservations streams;
        int disposeHookCalls = 0;
        bool lastDisposing = false;
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
        void Dispose(bool disposing) override
        {
            ++disposeHookCalls;
            lastDisposing = disposing;
            ContentManager::Dispose(disposing);
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
        // The callback is per typed read occurrence. This caller wants exactly-once disposal, so
        // its ownership list explicitly deduplicates repeated identities.
        std::vector<std::shared_ptr<System::IDisposable>> owned;
        int recordCalls = 0;
        const auto record = [&](std::shared_ptr<System::IDisposable> resource) {
            ++recordCalls;
            if (std::find(owned.begin(), owned.end(), resource) == owned.end())
                owned.push_back(std::move(resource));
        };
        auto first = pack.ReadOwned("level", record);
        const std::vector<std::uint8_t> expected{1, 1, 0, 2, 0, 3, 3, 0};
        if (!ctx.Check(first.name == "Courtyard" && first.tiles && first.alias == first.tiles &&
                       first.tiles->bytes == expected && !first.absent,
                       "the packed level restores aliased tile blocks and its null optional resource")) return;
        ctx.Check(recordCalls == 2 && owned.size() == 1 && owned.front() == first.tiles,
                  "the callback receives each typed occurrence and the caller deduplicates their shared identity");
        ctx.Check(pack.streams.opened == 1 && pack.streams.destroyed == 1 && pack.streams.bytes == sourceSize && pack.streams.reads > 1,
                  "ReadAsset consumes short stream reads and destroys its owned source stream before returning");
        auto second = pack.ReadOwned("level", record);
        if (!ctx.Check(second.tiles && second.alias == second.tiles && second.tiles != first.tiles &&
                       second.tiles->bytes == expected && pack.streams.opened == 2,
                       "reading the same name again constructs a fresh resource instead of reusing the first asset")) return;
        ctx.Check(recordCalls == 4 && owned.size() == 2 && owned[1] == second.tiles &&
                  pack.streams.destroyed == 2 && pack.streams.bytes == 2 * sourceSize,
                  "each read independently records its nested resource and releases its source stream");
        first.tiles->bytes[0] = 9;
        ctx.Check(first.alias->bytes[0] == 9 && second.tiles->bytes[0] == 1,
                  "aliases share one read's mutation while independently read resources do not");
        pack.Unload();
        ctx.Check(!first.tiles->disposed && !second.tiles->disposed && first.tiles->bytes[0] == 9 && second.tiles->bytes == expected,
                  "manager Unload leaves the explicitly delegated resources under the caller's control");
        bool emptyRefused = false;
        try { (void)pack.ReadOwned("", record); }
        catch (const System::ArgumentNullException&) { emptyRefused = true; }
        ctx.Check(emptyRefused && pack.streams.opened == 2 && owned.size() == 2,
                  "an empty asset name is refused before opening a stream or recording resources");
        System::IDisposable& disposableManager = pack;
        disposableManager.Dispose();
        ctx.Check(pack.disposeHookCalls == 1 && pack.lastDisposing,
                  "IDisposable::Dispose dispatches through the derived protected Dispose(true) hook");
        ctx.Check(!first.tiles->disposed && !second.tiles->disposed, "manager disposal also preserves explicitly delegated resources");
        pack.Dispose();
        ctx.Check(pack.disposeHookCalls == 2 && pack.lastDisposing,
                  "repeated public disposal reaches the override while base cleanup remains idempotent");
        bool disposedRefused = false;
        try { (void)pack.ReadOwned("level", record); }
        catch (const System::ObjectDisposedException&) { disposedRefused = true; }
        ctx.Check(disposedRefused && pack.streams.opened == 2 && owned.size() == 2,
                  "a disposed manager refuses another read before stream creation or callback invocation");
        for (const auto& resource : owned) resource->Dispose();
        ctx.Check(first.tiles->disposed && second.tiles->disposed && first.tiles->bytes.empty() && second.tiles->bytes.empty(),
                  "the caller disposes the recorded resources through IDisposable and releases their tile buffers");
        const std::weak_ptr<Tiles> firstLifetime = first.tiles, secondLifetime = second.tiles;
        first.tiles.reset(); first.alias.reset();
        second.tiles.reset(); second.alias.reset();
        ctx.Check(!firstLifetime.expired() && !secondLifetime.expired(), "the callback's list owns the resources after asset values release them");
        owned.clear();
        ctx.Check(firstLifetime.expired() && secondLifetime.expired(), "releasing the caller's list destroys both resources without hidden manager ownership");

        // A later parse failure does not revoke ownership already handed to the callback. Remove
        // only the final null-reference byte and repair the XNB's declared length so deserialization
        // reaches both aliased tiles before it fails on the absent field.
        auto partialBytes = bytes;
        partialBytes.pop_back();
        const std::uint32_t partialSize = static_cast<std::uint32_t>(partialBytes.size());
        for (int i = 0; i < 4; ++i)
            partialBytes[6 + i] = static_cast<std::uint8_t>((partialSize >> (8 * i)) & 0xffu);
        PackManager partial(std::move(partialBytes));
        std::vector<std::shared_ptr<System::IDisposable>> partialOwned;
        int partialRecordCalls = 0;
        bool partialFailed = false;
        try
        {
            (void)partial.ReadOwned("level", [&](std::shared_ptr<System::IDisposable> resource) {
                ++partialRecordCalls;
                if (std::find(partialOwned.begin(), partialOwned.end(), resource) == partialOwned.end())
                    partialOwned.push_back(std::move(resource));
            });
        }
        catch (const std::exception& error)
        {
            partialFailed = true;
            ctx.Print("Partial level refused: {}", error.what());
        }
        if (!ctx.Check(partialFailed && partialRecordCalls == 2 && partialOwned.size() == 1 &&
                       !static_cast<Tiles*>(partialOwned.front().get())->disposed &&
                       partial.streams.opened == 1 && partial.streams.destroyed == 1,
                       "a later read failure preserves the one resource already delegated to the caller")) return;
        const std::weak_ptr<System::IDisposable> partialLifetime = partialOwned.front();
        partialOwned.front()->Dispose();
        partialOwned.clear();
        ctx.Check(partialLifetime.expired(),
                  "the caller can dispose and release a delegated resource after its parent read fails");

        struct RecorderFailure : std::runtime_error { using std::runtime_error::runtime_error; };
        PackManager throwing(bytes);
        std::weak_ptr<System::IDisposable> rejectedLifetime;
        bool recorderFailurePropagated = false;
        try
        {
            (void)throwing.ReadOwned("level", [&](std::shared_ptr<System::IDisposable> resource) {
                rejectedLifetime = resource;
                throw RecorderFailure("caller recorder stopped the read");
            });
        }
        catch (const RecorderFailure&) { recorderFailurePropagated = true; }
        ctx.Check(recorderFailurePropagated && rejectedLifetime.expired() &&
                  throwing.streams.opened == 1 && throwing.streams.destroyed == 1,
                  "a recorder exception propagates while the stream and unclaimed resource unwind cleanly");

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
                   "deduplicates aliased ownership, and checks failure cleanup against the default recorder.",
    .apis = {"ContentManager::ReadAsset", "ContentManager::OpenStream", "ContentManager::Unload", "ContentManager::Dispose",
             "ContentReader::ReadObject", "ContentTypeReader::Read", "ContentTypeReaderManager::AddTypeCreator"},
    .tags = {"content", "xnb", "pack-file", "lifetime", "dispose", "callback"},
    .assets = {"ownership/level.xnb"},
    .restrictions = "Supplied and absent callbacks with typed shared_ptr IDisposable children; default tracking reproduces ISSUE-011. Raw/untyped and shared-resource disposal discovery remain separate contracts.",
    .knownIssue = "CNA-DEMOS-ISSUE-011",
})
