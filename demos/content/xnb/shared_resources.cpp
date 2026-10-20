// SPDX-License-Identifier: MS-PL
// XNB shared resources: deferred callbacks rebuild aliases, a forward edge and a weak back edge.
#include <algorithm>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"
#include "System/IDisposable.hpp"

using namespace Microsoft::Xna::Framework::Content;

namespace
{
    struct Item final : System::IDisposable
    {
        std::string name;
        int quantity = 0;
        std::shared_ptr<Item> next;
        std::weak_ptr<Item> previous; // the graph has a back edge without a strong ownership cycle
        bool disposed = false;
        void Dispose() override { disposed = true; }
    };
    struct Party
    {
        std::shared_ptr<Item> primary, alias, absent, inlineCopy;
    };
    struct Observation
    {
        int roots = 0, items = 0, rootFixups = 0, nextFixups = 0, previousFixups = 0, zeroFixups = 0;
        bool rootWasDeferred = false, allItemsReadBeforeFixups = true;
    } observed; // factories and observations safely outlive process-wide reader registration

    struct Label { std::string text; }; // a valid object with the wrong shared-resource type
    class LabelReader final : public ContentTypeReader<Label>
    {
    public:
        LabelReader() : ContentTypeReader<Label>("CnaDemos.RuntimeGraphLabel") {}
    protected:
        Label Read(ContentReader& input, std::optional<Label>) override { return {input.ReadString()}; }
    };

    class ItemReader final : public ContentTypeReader<std::shared_ptr<Item>>
    {
    public:
        ItemReader() : ContentTypeReader<std::shared_ptr<Item>>("CnaDemos.RuntimeSharedItem") {}
    protected:
        std::shared_ptr<Item> Read(ContentReader& input, std::optional<std::shared_ptr<Item>>) override
        {
            ++observed.items;
            auto item = std::make_shared<Item>();
            item->name = input.ReadString();
            item->quantity = input.ReadInt32();
            // Capture a stable object, not a reference to a local shared_ptr variable.
            input.ReadSharedResource<std::shared_ptr<Item>>([item](std::shared_ptr<Item> next) {
                ++observed.nextFixups;
                observed.allItemsReadBeforeFixups &= observed.items == 3;
                item->next = std::move(next);
            });
            input.ReadSharedResource<std::shared_ptr<Item>>([item](std::shared_ptr<Item> previous) {
                ++observed.previousFixups;
                observed.allItemsReadBeforeFixups &= observed.items == 3;
                item->previous = previous;
            });
            return item;
        }
    };

    class PartyReader final : public ContentTypeReader<std::shared_ptr<Party>>
    {
    public:
        PartyReader() : ContentTypeReader<std::shared_ptr<Party>>("CnaDemos.RuntimeSharedParty") {}
    protected:
        std::shared_ptr<Party> Read(ContentReader& input, std::optional<std::shared_ptr<Party>>) override
        {
            ++observed.roots;
            auto party = std::make_shared<Party>();
            input.ReadSharedResource<std::shared_ptr<Item>>([party](std::shared_ptr<Item> value) {
                ++observed.rootFixups;
                observed.allItemsReadBeforeFixups &= observed.items == 3;
                party->primary = std::move(value);
            });
            input.ReadSharedResource<std::shared_ptr<Item>>([party](std::shared_ptr<Item> value) {
                ++observed.rootFixups;
                observed.allItemsReadBeforeFixups &= observed.items == 3;
                party->alias = std::move(value);
            });
            input.ReadSharedResource<std::shared_ptr<Item>>([party](std::shared_ptr<Item> value) {
                ++observed.zeroFixups; // the authored index is zero: this must never execute
                party->absent = std::move(value);
            });
            party->inlineCopy = input.ReadObject<std::shared_ptr<Item>>(); // ordinary reader-table dispatch, not a shared index
            observed.rootWasDeferred = !party->primary && !party->alias && !party->absent &&
                                       observed.items == 1 && observed.rootFixups == 0;
            // ContentManager finishes ReadSharedResources AFTER this root reader returns.
            return party;
        }
    };

    class RecordingManager final : public ContentManager
    {
    public:
        using ContentManager::ContentManager;
        std::shared_ptr<Party> ReadOwned(
            const std::string& name,
            std::function<void(std::shared_ptr<System::IDisposable>)> record)
        {
            return ReadAsset<std::shared_ptr<Party>>(name, std::move(record));
        }
    };

    void SharedResources(CnaDemos::DemoContext& ctx)
    {
        ContentTypeReaderManager::AddTypeCreator("CnaDemos.RuntimeGraphReader", [] { return std::make_unique<PartyReader>(); });
        ContentTypeReaderManager::AddTypeCreator("CnaDemos.RuntimeItemReader", [] { return std::make_unique<ItemReader>(); });
        // Register every reader this console asset needs: built-ins are registered by Game startup.
        ContentTypeReaderManager::AddTypeCreator("CnaDemos.RuntimeLabelReader", [] { return std::make_unique<LabelReader>(); });
        // A console demo has no Game establishing its content root. Use the public platform path
        // explicitly, avoiding the pre-platform working-directory ambiguity in ISSUE-014.
        const auto root = std::filesystem::path(CNA::Platform::GetCurrentPlatform().GetFileSystem()->GetBasePath()) / "assets";
        std::weak_ptr<Item> lifetime;
        {
            ContentManager content(nullptr, root.string());
            observed = {};
            auto party = content.Load<std::shared_ptr<Party>>("graphs/shared_party");
            if (!ctx.Check(party != nullptr, "the root reader returns a stable heap-owned graph")) return;
            ctx.Check(observed.rootWasDeferred, "shared references are still unset when the root reader returns");
            ctx.Check(observed.roots == 1 && observed.items == 3 && observed.rootFixups == 2 &&
                      observed.nextFixups == 1 && observed.previousFixups == 1 && observed.allItemsReadBeforeFixups,
                      "the inline item and both shared objects are read before any of the four non-null fixup callbacks");
            if (!ctx.Check(party->primary && party->primary == party->alias,
                           "two root references resolve to the same shared item")) return;
            const auto& sword = party->primary;
            if (!ctx.Check(sword->name == "Sword" && sword->quantity == 7 && sword->next &&
                           sword->next->name == "Gem" && sword->next->quantity == 11,
                           "the forward shared reference reaches the second fully decoded item")) return;
            ctx.Check(sword->next->previous.lock() == sword && !sword->next->next && sword->previous.expired(),
                      "the weak backward reference reconnects the graph without owning its predecessor");
            ctx.Check(!party->absent && observed.zeroFixups == 0, "a zero reference leaves the field empty and never invokes its callback");
            ctx.Check(party->inlineCopy && party->inlineCopy != sword && party->inlineCopy->name == sword->name &&
                      party->inlineCopy->quantity == sword->quantity,
                      "ReadObject creates a separate inline item even when its values equal a shared resource");
            ++sword->quantity;
            ctx.Check(party->alias->quantity == 8 && sword->next->quantity == 11 && party->inlineCopy && party->inlineCopy->quantity == 7,
                      "mutating an alias changes only the shared item, leaving the inline copy and neighbor intact");
            auto cached = content.Load<std::shared_ptr<Party>>("graphs/shared_party");
            ctx.Check(cached == party && observed.roots == 1 && observed.items == 3,
                      "a cached Load returns the same graph without rerunning readers or fixups");
            observed = {};
            ContentManager another(nullptr, root.string());
            auto independent = another.Load<std::shared_ptr<Party>>("graphs/shared_party");
            ctx.Check(independent && independent != party && independent->primary != sword &&
                      independent->primary && independent->primary->quantity == 7 && independent->primary == independent->alias,
                      "another manager reconstructs an independent graph while preserving its internal alias");
            for (const char* bad : {"graphs/bad_shared_index", "graphs/bad_shared_type"})
            {
                observed = {};
                bool refused = false;
                try { (void)content.Load<std::shared_ptr<Party>>(bad); }
                catch (const ContentLoadException& error)
                {
                    refused = true;
                    ctx.Print("{} refused: {}", bad, error.what());
                }
                ctx.Check(refused && observed.rootFixups == 0,
                          std::string(bad) + " is refused before any incompatible root fixup executes");
            }
            ctx.Check(content.Load<std::shared_ptr<Party>>("graphs/shared_party") == party,
                      "failed graph loads leave the already-cached valid graph intact");
            lifetime = sword;
            ctx.Print("Sword aliases share quantity {}; Sword -> Gem -> weak Sword", party->alias->quantity);
        }
        ctx.Check(lifetime.expired(), "the weak back edge lets the graph die after its managers and owners are gone");

        // ReadAsset's supplied recorder must see IDisposable values regardless of whether they
        // arrived through typed ReadObject dispatch or the type-erased shared-resource table.
        RecordingManager recording(nullptr, root.string());
        observed = {};
        std::vector<std::shared_ptr<System::IDisposable>> recorded;
        auto recordedParty = recording.ReadOwned(
            "graphs/shared_party",
            [&recorded](std::shared_ptr<System::IDisposable> value) {
                recorded.push_back(std::move(value));
            });
        const auto wasRecorded = [&recorded](const std::shared_ptr<Item>& item) {
            return std::find(recorded.begin(), recorded.end(), item) != recorded.end();
        };
        ctx.Check(recordedParty && recordedParty->inlineCopy && wasRecorded(recordedParty->inlineCopy),
                  "the supplied ReadAsset recorder receives the typed inline disposable");
        ctx.Check(recordedParty && recordedParty->primary && recordedParty->primary->next &&
                  recorded.size() == 3 && wasRecorded(recordedParty->primary) &&
                  wasRecorded(recordedParty->primary->next),
                  "the recorder also receives both IDisposable shared-table resources (CNA-DEMOS-ISSUE-087)");
        for (const auto& value : recorded) value->Dispose();
        if (recordedParty && recordedParty->primary)
        {
            if (!recordedParty->primary->disposed) recordedParty->primary->Dispose();
            if (recordedParty->primary->next && !recordedParty->primary->next->disposed)
                recordedParty->primary->next->Dispose();
        }
    }
}

CNA_DEMOS_CONSOLE(SharedResources, {
    .id = "content.xnb.shared_resources",
    .title = "Resolve an XNB shared object graph",
    .description = "Uses deferred public ContentReader callbacks to rebuild shared aliases, forward and weak backward references, "
                   "checks graph lifetime and failures, and audits ReadAsset disposal recording.",
    .apis = {"ContentReader::ReadSharedResource", "ContentReader::ReadObject", "ContentReader::ReadString", "ContentReader::ReadInt32",
             "ContentTypeReader::Read", "ContentTypeReaderManager::AddTypeCreator", "ContentManager::Load", "ContentManager::ReadAsset"},
    .tags = {"content", "xnb", "reader", "shared-resources", "ownership"},
    .assets = {"graphs/shared_party.xnb", "graphs/bad_shared_index.xnb", "graphs/bad_shared_type.xnb"},
})
