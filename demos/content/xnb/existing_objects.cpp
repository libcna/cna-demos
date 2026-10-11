// SPDX-License-Identifier: MS-PL
// A loadout reader fills existing slots while preserving their runtime-only identity and state.
#include <any>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"

using namespace Microsoft::Xna::Framework::Content;

namespace
{
    constexpr const char* WeaponReaderName = "CnaDemos.LoadoutWeaponReader";
    constexpr const char* WeaponTypeName = "CnaDemos.LoadoutWeapon";
    struct Weapon { std::string name = "Default"; int ammunition = 1; std::string runtimeSlot; };
    using WeaponPtr = std::shared_ptr<Weapon>;
    struct Loadout
    {
        WeaponPtr primary, secondary, fallback;
        std::any tag, absentTag;
        bool primaryRetained = false, secondaryRetained = false, fallbackRetained = false;
        int endMarker = 0;
    };
    struct Observation
    {
        int constructed = 0, rootInitializations = 0, weaponInitializations = 0;
        int existingReads = 0, freshReads = 0;
        bool tableReadyBeforeInitialize = false, readerContract = false, typeLookupResolved = false;
    } observed;

    class WeaponReader final : public ContentTypeReader<WeaponPtr>
    {
    public:
        WeaponReader() : ContentTypeReader<WeaponPtr>(WeaponTypeName) { ++observed.constructed; }
        bool getCanDeserializeIntoExistingObjectProperty() const override { return true; }
        void Initialize(ContentTypeReaderManager&) override { ++observed.weaponInitializations; }
    protected:
        WeaponPtr Read(ContentReader& input, std::optional<WeaponPtr> existing) override
        {
            auto weapon = existing && *existing ? *existing : std::make_shared<Weapon>();
            if (existing && *existing) ++observed.existingReads; else ++observed.freshReads;
            // The content schema owns these fields. Runtime slot state belongs to the caller.
            weapon->name = input.ReadString();
            weapon->ammunition = input.ReadInt32();
            return weapon;
        }
    };

    class LoadoutReader final : public ContentTypeReader<Loadout>
    {
    public:
        LoadoutReader() : ContentTypeReader<Loadout>("CnaDemos.Loadout") { ++observed.constructed; }
        void Initialize(ContentTypeReaderManager& manager) override
        {
            ++observed.rootInitializations;
            // The root occurs BEFORE WeaponReader in the table, but both have been constructed.
            observed.tableReadyBeforeInitialize = observed.constructed == 2;
            weaponReader_ = manager.GetTypeReader(System::Type::From<WeaponPtr>());
            observed.typeLookupResolved = weaponReader_ != nullptr;
            // ISSUE-043: keep the failed type-lookup check visible below. A named public factory lets the
            // remaining existing-instance/raw-payload example run without changing CNA.
            if (!weaponReader_) weaponReader_ = ContentTypeReaderManager::CreateReader(WeaponReaderName);
            if (!weaponReader_) throw ContentLoadException("Loadout requires its weapon reader");
            observed.readerContract = weaponReader_->getCanDeserializeIntoExistingObjectProperty() &&
                                      weaponReader_->getTargetTypeNameProperty() == WeaponTypeName;
            // CNA returns a fresh reader here, not the table's instance. This dependency's Read
            // is stateless; it requires no separate Initialize call or per-file version state.
        }
    protected:
        Loadout Read(ContentReader& input, std::optional<Loadout>) override
        {
            Loadout loadout;
            auto primary = std::make_shared<Weapon>(); primary->runtimeSlot = "primary";
            auto secondary = std::make_shared<Weapon>(); secondary->runtimeSlot = "secondary";
            auto fallback = std::make_shared<Weapon>(); fallback->runtimeSlot = "fallback";
            loadout.primary = input.ReadObject<WeaponPtr>(primary); // consumes the reader-table index
            loadout.secondary = input.ReadRawObject<WeaponPtr>(*weaponReader_, secondary); // payload only
            loadout.fallback = input.ReadObject<WeaponPtr>(fallback); // authored zero: retain this default
            loadout.tag = input.ReadObject(); // std::any preserves the actual produced type
            loadout.absentTag = input.ReadObject(); // authored zero: empty std::any
            loadout.endMarker = input.ReadInt32();
            loadout.primaryRetained = loadout.primary == primary;
            loadout.secondaryRetained = loadout.secondary == secondary;
            loadout.fallbackRetained = loadout.fallback == fallback;
            return loadout;
        }
    private:
        std::unique_ptr<ContentTypeReaderBase> weaponReader_;
    };

    void ExistingObjects(CnaDemos::DemoContext& ctx)
    {
        ContentTypeReaderManager::AddTypeCreator("CnaDemos.LoadoutReader", [] { return std::make_unique<LoadoutReader>(); });
        ContentTypeReaderManager::AddTypeCreator(WeaponReaderName, [] { return std::make_unique<WeaponReader>(); });
        const auto root = std::filesystem::path(CNA::Platform::GetCurrentPlatform().GetFileSystem()->GetBasePath()) / "assets";
        ContentManager content(nullptr, root.string());
        observed = {};
        const auto loadout = content.Load<Loadout>("loadouts/existing_slots");
        ctx.Check(observed.tableReadyBeforeInitialize && observed.rootInitializations == 1 && observed.weaponInitializations == 1,
                  "all table readers are constructed before their one-time Initialize calls");
        ctx.Check(observed.typeLookupResolved,
                  "GetTypeReader resolves a target type whose canonical name differs from its reader factory name (CNA-DEMOS-ISSUE-043)");
        ctx.Check(observed.readerContract && observed.constructed == 3,
                  "the dependent reader retains the correct target type and advertises existing-object support");
        if (!ctx.Check(loadout.primary && loadout.secondary && loadout.fallback, "all three loadout slots remain populated")) return;
        ctx.Check(loadout.primaryRetained && loadout.primary->name == "Sword" && loadout.primary->ammunition == 12,
                  "indexed ReadObject fills the existing primary object without replacing its identity");
        ctx.Check(loadout.secondaryRetained && loadout.secondary->name == "Bow" && loadout.secondary->ammunition == 6,
                  "ReadRawObject fills the existing secondary directly without consuming a reader index");
        ctx.Check(loadout.primary->runtimeSlot == "primary" && loadout.secondary->runtimeSlot == "secondary",
                  "fields outside the content schema keep their caller-owned runtime state");
        ctx.Check(loadout.fallbackRetained && loadout.fallback->name == "Default" && loadout.fallback->ammunition == 1 &&
                  loadout.fallback->runtimeSlot == "fallback", "a zero object reference preserves the supplied fallback instance");
        const auto* tagged = std::any_cast<WeaponPtr>(&loadout.tag); // safe type probe, no unchecked cast
        ctx.Check(tagged && *tagged && *tagged != loadout.primary && (*tagged)->name == "Sword" &&
                  (*tagged)->ammunition == 12 && (*tagged)->runtimeSlot.empty(),
                  "the type-erased tag contains a fresh weapon with equal content and independent runtime state");
        ctx.Check(std::any_cast<int>(&loadout.tag) == nullptr && !loadout.absentTag.has_value(),
                  "an incompatible tag probe returns null and a zero object becomes empty std::any");
        ctx.Check(observed.existingReads == 2 && observed.freshReads == 1,
                  "only populated payloads invoke the weapon reader, with the correct existing-instance optional");
        ctx.Check(loadout.endMarker == 0x12345678, "mixed indexed and raw payloads leave the stream aligned at the trailer");
        ctx.Print("{}: {} rounds; {}: {} rounds; default retained; independent typed tag",
                  loadout.primary->name, loadout.primary->ammunition, loadout.secondary->name, loadout.secondary->ammunition);
    }
}

CNA_DEMOS_CONSOLE(ExistingObjects, {
    .id = "content.xnb.existing_objects",
    .title = "Fill existing XNB loadout slots",
    .description = "Reads indexed and raw weapon payloads into existing objects, preserves runtime slot state, "
                   "resolves a dependent reader during initialization, and safely inspects type-erased and null tags.",
    .apis = {"ContentReader::ReadObject", "ContentReader::ReadRawObject", "ContentTypeReader::Read",
             "ContentTypeReader::Initialize", "ContentTypeReader::CanDeserializeIntoExistingObject", "ContentTypeReader::TargetType",
             "ContentTypeReaderManager::GetTypeReader", "ContentTypeReaderManager::AddTypeCreator", "ContentManager::Load"},
    .tags = {"content", "xnb", "reader", "existing-instance", "ownership"},
    .assets = {"loadouts/existing_slots.xnb"},
    .knownIssue = "CNA-DEMOS-ISSUE-043",
})
