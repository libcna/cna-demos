// SPDX-License-Identifier: MS-PL
// A game's JSON assets: named CNJ factories produce the same C++ enemy type.
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <nlohmann/json.hpp>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

using namespace Microsoft::Xna::Framework::Content;
using Microsoft::Xna::Framework::Graphics::Texture2D;

namespace
{
    struct Enemy { std::string name; int health; };
    struct Nameplate { std::string text; };

    struct TemporaryContent
    {
        std::filesystem::path path;
        ~TemporaryContent()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void WriteCnj(const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file << text;
        file.close();
        if (!file) throw std::runtime_error("Could not write a private CNJ fixture");
    }

    void CnjLoader(CnaDemos::DemoContext& ctx)
    {
        // CNJ registration belongs to this manager, unlike the shared CNB loader registry.
        // Captured counters outlive the manager that owns these callbacks.
        int ordinaryReads = 0, eliteReads = 0, nameplateReads = 0;
        ContentManager content;
        content.setRootDirectoryProperty("assets/custom");
        content.RegisterCnjLoader<Enemy>("CnaDemos.Enemy",
            [&ordinaryReads](const std::string& text, ContentManager&) {
                // CNA supplies raw JSON; a game chooses its own parser for the payload.
                // nlohmann-json is already a documented build prerequisite of this project.
                const auto data = nlohmann::json::parse(text).at("data");
                Enemy enemy{data.at("name").get<std::string>(), data.at("health").get<int>()};
                ++ordinaryReads;
                return enemy;
            });
        content.RegisterCnjLoader<Enemy>("CnaDemos.EliteEnemy",
            [&eliteReads](const std::string& text, ContentManager&) {
                const auto data = nlohmann::json::parse(text).at("data");
                // Elite assets author a base health; this game's elite rule doubles it.
                Enemy enemy{data.at("name").get<std::string>(), 2 * data.at("baseHealth").get<int>()};
                ++eliteReads;
                return enemy;
            });

        // MTH-CONF-004 (3671c7ee0): one logical asset name has one cached type per manager.
        const auto loadNameplate = [&nameplateReads](const std::string& text, ContentManager&) {
            const auto data = nlohmann::json::parse(text).at("data");
            ++nameplateReads;
            return Nameplate{data.at("name").get<std::string>() + " (" +
                             std::to_string(data.at("health").get<int>()) + " HP)"};
        };
        content.RegisterCnjLoader<Nameplate>("CnaDemos.Enemy", loadNameplate);
        ContentManager labels;
        labels.setRootDirectoryProperty("assets/custom");
        labels.RegisterCnjLoader<Nameplate>("CnaDemos.Enemy", loadNameplate);

        const Enemy guard = content.Load<Enemy>("guard");
        const Enemy elite = content.Load<Enemy>("elite");
        ctx.Print("{}: {} HP; {}: {} HP", guard.name, guard.health, elite.name, elite.health);
        ctx.Check(guard.name == "Mossling" && guard.health == 12,
                  "the ordinary factory reads the name and health from the CNJ payload");
        ctx.Check(elite.name == "Stone Guard" && elite.health == 100,
                  "a second CNJ type selects a different factory for the same C++ Enemy");
        (void)content.Load<Enemy>("guard");
        (void)content.Load<Enemy>("elite");
        ctx.Check(ordinaryReads == 1 && eliteReads == 1, "both named factories use the manager's asset cache");

        bool cachedTypeRefused = false;
        try { (void)content.Load<Nameplate>("guard"); }
        catch (const ContentLoadException&) { cachedTypeRefused = true; }
        ctx.Check(cachedTypeRefused && nameplateReads == 0,
                  "an existing cached type refuses another registered type without invoking its factory");
        const auto plate = labels.Load<Nameplate>("guard");
        ctx.Check(plate.text == "Mossling (12 HP)" && nameplateReads == 1 && ordinaryReads == 1,
                  "separate managers cache the same asset name as exact Enemy and Nameplate values");
        (void)labels.Load<Nameplate>("guard");
        (void)content.Load<Enemy>("guard");
        ctx.Check(nameplateReads == 1 && ordinaryReads == 1,
                  "repeating each manager's requested type reuses its existing cache entry");
        bool wrongTypeRefused = false;
        try { (void)content.Load<int>("guard"); }
        catch (const ContentLoadException&) { wrongTypeRefused = true; }
        ctx.Check(wrongTypeRefused && nameplateReads == 1 && ordinaryReads == 1,
                  "cached objects do not satisfy an unregistered requested C++ type");

        bool duplicateRejected = false;
        try
        {
            content.RegisterCnjLoader<Enemy>("CnaDemos.Enemy",
                [](const std::string&, ContentManager&) -> Enemy {
                    throw std::logic_error("Duplicate registration must never replace the original factory");
                });
        }
        catch (const std::logic_error&) { duplicateRejected = true; }
        ctx.Check(duplicateRejected, "duplicate named registration is rejected instead of replacing a factory");

        bool payloadRejected = false;
        try { (void)content.Load<Enemy>("missing_health"); }
        catch (const ContentLoadException& error)
        {
            payloadRejected = true;
            ctx.Print("Invalid enemy payload: {}", error.what());
        }
        ctx.Check(payloadRejected && ordinaryReads == 1,
                  "a missing required payload field reaches the caller as ContentLoadException");

        content.Unload();
        const Enemy reloaded = content.Load<Enemy>("guard");
        ctx.Check(reloaded.health == 12 && ordinaryReads == 2,
                  "Unload clears cached assets but retains this manager's factory registrations");
        labels.Unload();
        ctx.Check(labels.Load<Nameplate>("guard").text == plate.text && nameplateReads == 2,
                  "the label manager Unload clears its cached value while preserving its factory");

        // Failed envelope dispatch must not invoke the factory or publish a cache entry. Repair
        // each exact path and retry it through the same manager to prove both properties.
        const auto path = std::filesystem::temp_directory_path() /
            ("cna-demos-cnj-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(path))
            throw std::runtime_error("Could not create a private CNJ content directory");
        TemporaryContent temporary{path};
        int retryReads = 0;
        ContentManager guarded;
        guarded.setRootDirectoryProperty(path.string());

        bool emptyNameRefused = false;
        try
        {
            guarded.RegisterCnjLoader<Enemy>("",
                [](const std::string&, ContentManager&) { return Enemy{}; });
        }
        catch (const std::invalid_argument&) { emptyNameRefused = true; }
        ctx.Check(emptyNameRefused, "an empty CNJ type name is rejected as an invalid argument");

        bool emptyFactoryRefused = false;
        try
        {
            guarded.RegisterCnjLoader<Enemy>(
                "CnaDemos.Empty", ContentManager::CnjLoaderFn<Enemy>{});
        }
        catch (const std::invalid_argument&) { emptyFactoryRefused = true; }
        ctx.Check(emptyFactoryRefused, "an empty CNJ factory is rejected as an invalid argument");

        bool occupiedTypeRefused = false;
        bool unreachableFactoryInvoked = false;
        try
        {
            guarded.RegisterCnjLoader<Texture2D>("CnaDemos.Texture",
                [&unreachableFactoryInvoked](const std::string&, ContentManager&) -> Texture2D {
                    unreachableFactoryInvoked = true;
                    throw std::logic_error("An unreachable Texture2D factory was invoked");
                });
        }
        catch (const std::logic_error&) { occupiedTypeRefused = true; }
        ctx.Check(occupiedTypeRefused && !unreachableFactoryInvoked,
                  "a built-in C++ asset type refuses a shadow CNJ loader before invoking it");

        guarded.RegisterCnjLoader<Enemy>("CnaDemos.Enemy",
            [&retryReads](const std::string& text, ContentManager&) {
                const auto data = nlohmann::json::parse(text).at("data");
                ++retryReads;
                return Enemy{data.at("name").get<std::string>(), data.at("health").get<int>()};
            });

        WriteCnj(path / "unknown.cnj",
                 R"({"cnjVersion":1,"type":"CnaDemos.UnknownEnemy","data":{"name":"Bogus","health":1}})");
        bool unknownTypeRefused = false;
        try { (void)guarded.Load<Enemy>("unknown"); }
        catch (const ContentLoadException& error)
        {
            unknownTypeRefused = std::string(error.what()).find("CnaDemos.UnknownEnemy") != std::string::npos;
        }
        ctx.Check(unknownTypeRefused && retryReads == 0,
                  "an unknown envelope type is named and refused before any factory invocation");
        WriteCnj(path / "unknown.cnj",
                 R"({"cnjVersion":1,"type":"CnaDemos.Enemy","data":{"name":"Repaired","health":21}})");
        const Enemy repairedType = guarded.Load<Enemy>("unknown");
        ctx.Check(repairedType.name == "Repaired" && repairedType.health == 21 && retryReads == 1,
                  "repairing the same unknown-type path retries it instead of finding a failed cache entry");

        WriteCnj(path / "future.cnj",
                 R"({"cnjVersion":2,"type":"CnaDemos.Enemy","data":{"name":"Future","health":99}})");
        bool futureVersionRefused = false;
        try { (void)guarded.Load<Enemy>("future"); }
        catch (const ContentLoadException& error)
        {
            const std::string message = error.what();
            futureVersionRefused = message.find("cnjVersion") != std::string::npos &&
                                   message.find("2.000000") != std::string::npos;
        }
        ctx.Check(futureVersionRefused && retryReads == 1,
                  "a future envelope version is named and refused before any factory invocation");
        WriteCnj(path / "future.cnj",
                 R"({"cnjVersion":1,"type":"CnaDemos.Enemy","data":{"name":"Current","health":34}})");
        const Enemy repairedVersion = guarded.Load<Enemy>("future");
        ctx.Check(repairedVersion.name == "Current" && repairedVersion.health == 34 && retryReads == 2,
                  "repairing the same future-version path retries it without unloading the manager");

        ContentManager unregistered;
        unregistered.setRootDirectoryProperty("assets/custom");
        bool isolated = false;
        try { (void)unregistered.Load<Enemy>("guard"); }
        catch (const ContentLoadException&) { isolated = true; }
        ctx.Check(isolated, "a different manager needs its own CNJ registration");
    }
}

CNA_DEMOS_CONSOLE(CnjLoader, {
    .id = "content.custom.cnj_loader",
    .title = "Custom CNJ enemy definitions (CNAEXT)",
    .description = "Registers two named CNJ factories for one game-owned Enemy type, parses their JSON payloads, "
                   "checks typed caches and manager-local registration, and repairs rejected type/version envelopes.",
    .apis = {"ContentManager::RegisterCnjLoader", "ContentManager::Load", "ContentManager::Unload",
             "ContentLoadException"},
    .tags = {"content", "custom", "cnj", "json", "cnaext"},
    .assets = {"custom/guard.cnj", "custom/elite.cnj", "custom/missing_health.cnj"},
})
