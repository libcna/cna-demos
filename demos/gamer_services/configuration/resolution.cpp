// SPDX-License-Identifier: MS-PL
// CNA GamerServices deployment configuration: defaults, sources, overrides and secure endpoints.
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "CNA/GamerServices/Configuration.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    void SetEnvironment(const char* name, const std::string& value)
    {
#ifdef _WIN32
        _putenv_s(name, value.c_str());
#else
        setenv(name, value.c_str(), 1);
#endif
    }

    class TemporaryManifest
    {
    public:
        TemporaryManifest()
            : directory_(std::filesystem::temp_directory_path() /
                         ("cna-demos-gamer-configuration-" + std::to_string(
                             std::chrono::steady_clock::now().time_since_epoch().count()))),
              path_(directory_ / "title.json")
        {
            std::filesystem::remove_all(directory_);
            std::filesystem::create_directories(directory_);
            std::ofstream output(path_);
            output << R"({
  "endpoint": "",
  "gameId": "manifest-title",
  "caBundle": "/manifest/trust.pem",
  "insecureLoopback": false,
  "avatarCatalogUpdates": false,
  "maxAvatarCatalogBytes": 3145728,
  "titleVersion": "2.4.1"
})";
            output.close();
            if (!output)
            {
                throw std::runtime_error("could not create the temporary GamerServices manifest");
            }
        }

        ~TemporaryManifest()
        {
            std::error_code ignored;
            std::filesystem::remove_all(directory_, ignored);
        }

        [[nodiscard]] const std::filesystem::path& Path() const { return path_; }
        [[nodiscard]] const std::filesystem::path& Directory() const { return directory_; }

    private:
        std::filesystem::path directory_;
        std::filesystem::path path_;
    };

    bool SameConfiguration(const CNA::GamerServices::Configuration& left,
                           const CNA::GamerServices::Configuration& right)
    {
        return left.endpoint == right.endpoint && left.gameId == right.gameId &&
               left.caBundle == right.caBundle && left.insecureLoopback == right.insecureLoopback &&
               left.avatarCatalogUpdates == right.avatarCatalogUpdates &&
               left.maxAvatarCatalogBytes == right.maxAvatarCatalogBytes &&
               left.titleVersion == right.titleVersion;
    }

    CNA::GamerServices::Configuration OnlineConfiguration(std::string endpoint, std::string gameId,
                                                            const bool insecureLoopback = false)
    {
        CNA::GamerServices::Configuration configuration;
        configuration.endpoint = std::move(endpoint);
        configuration.gameId = std::move(gameId);
        configuration.insecureLoopback = insecureLoopback;
        return configuration;
    }

    template <class Action>
    bool ThrowsConfigurationError(Action&& action)
    {
        try
        {
            action();
            return false;
        }
        catch (const std::runtime_error&)
        {
            return true;
        }
    }

    void ConfigurationResolution(CnaDemos::DemoContext& ctx)
    {
        using CNA::GamerServices::Configuration;
        using CNA::GamerServices::resolveConfiguration;
        using CNA::GamerServices::setConfigurationOverride;
        using CNA::GamerServices::validateConfiguration;

        const Configuration defaults;
        ctx.Check(defaults.endpoint.empty() && defaults.gameId.empty() && defaults.caBundle.empty() &&
                      !defaults.insecureLoopback && defaults.avatarCatalogUpdates &&
                      defaults.maxAvatarCatalogBytes == (std::uint64_t{64} << 20) &&
                      defaults.titleVersion.empty(),
                  "Configuration defaults to offline mode, catalog updates, and a 64 MiB pack limit");
        ctx.Check(!ThrowsConfigurationError([&] { validateConfiguration(defaults); }),
                  "the default offline configuration is valid without a service transport");

        Configuration overridden;
        overridden.gameId = "override-title";
        overridden.caBundle = "/override/trust.pem";
        overridden.insecureLoopback = true;
        overridden.avatarCatalogUpdates = false;
        overridden.maxAvatarCatalogBytes = 7u << 20;
        overridden.titleVersion = "1.2.10";
        setConfigurationOverride(overridden);
        ctx.Check(SameConfiguration(resolveConfiguration(), overridden),
                  "a programmatic override preserves every deployment field exactly");

        TemporaryManifest manifest;
        SetEnvironment("XDG_CONFIG_HOME", manifest.Directory().string());
        SetEnvironment("CNA_GAMER_SERVICES_MANIFEST", manifest.Path().string());
        SetEnvironment("CNA_GAMER_SERVICES_ENDPOINT", "");
        SetEnvironment("CNA_GAME_ID", "");
        SetEnvironment("CNA_GAMER_SERVICES_CA_BUNDLE", "");
        SetEnvironment("CNA_GAME_VERSION", "");
        SetEnvironment("CNA_GAMER_SERVICES_INSECURE_LOOPBACK", "");
        SetEnvironment("CNA_AVATAR_CATALOG_UPDATES", "");
        setConfigurationOverride(std::nullopt);

        const Configuration resolved = resolveConfiguration();
        ctx.Check(resolved.endpoint.empty() && resolved.gameId == "manifest-title" &&
                      resolved.caBundle == "/manifest/trust.pem" && !resolved.insecureLoopback &&
                      !resolved.avatarCatalogUpdates && resolved.maxAvatarCatalogBytes == 3145728u &&
                      resolved.titleVersion == "2.4.1",
                  "removing the override exposes all values from the selected title manifest");

        Configuration invalidVersion;
        invalidVersion.titleVersion = "v1.2";
        ctx.Check(ThrowsConfigurationError([&] { validateConfiguration(invalidVersion); }),
                  "title versions use the bounded numeric dotted form");

        const Configuration secure = OnlineConfiguration("https://service.example/cna/v1", "secure-title");
        bool serviceTransportAvailable = true;
        std::string unavailableReason;
        try
        {
            validateConfiguration(secure);
        }
        catch (const std::runtime_error& error)
        {
            serviceTransportAvailable = false;
            unavailableReason = error.what();
        }

        if (serviceTransportAvailable)
        {
            ctx.Check(!ThrowsConfigurationError([] {
                          validateConfiguration(
                              OnlineConfiguration("http://127.0.0.1/cna/v1", "loopback-title", true));
                      }),
                      "unencrypted service traffic requires explicit numeric-loopback opt-in");
            ctx.Check(ThrowsConfigurationError([] {
                          validateConfiguration(
                              OnlineConfiguration("http://service.example/cna/v1", "remote-title", true));
                      }) &&
                          ThrowsConfigurationError([] {
                              validateConfiguration(
                                  OnlineConfiguration("http://localhost/cna/v1", "loopback-title", true));
                          }),
                      "insecure remote and hostname-loopback endpoints are refused");
            ctx.Check(ThrowsConfigurationError([] {
                          validateConfiguration(OnlineConfiguration(
                              "https://user:secret@service.example/cna/v1", "secure-title"));
                      }) &&
                          ThrowsConfigurationError([] {
                              validateConfiguration(
                                  OnlineConfiguration("https://service.example/other", "secure-title"));
                          }) &&
                          ThrowsConfigurationError([] {
                              validateConfiguration(OnlineConfiguration(
                                  "https://service.example/cna/v1?token=secret", "secure-title"));
                          }),
                      "credentials, a different path, and query data are refused in service endpoints");
            ctx.Check(ThrowsConfigurationError([] {
                          validateConfiguration(OnlineConfiguration("https://service.example/cna/v1", ""));
                      }),
                      "an online configuration requires a stable game identifier");
        }
        else
        {
            ctx.Check(unavailableReason.find("SERVICE_TRANSPORT_UNAVAILABLE") != std::string::npos,
                      "targets without the service transport explicitly refuse online configuration");
        }

        // Other GamerServices demos in this process must never inherit an online configuration.
        setConfigurationOverride(Configuration{});
        ctx.Print("configuration source: manifest; service endpoint validation: {}",
                  serviceTransportAvailable ? "available" : "transport unavailable");
    }
}

CNA_DEMOS_CONSOLE(ConfigurationResolution, {
    .id = "gamer_services.configuration.resolution",
    .title = "Gamer Services configuration",
    .description = "Checks offline defaults, exact programmatic overrides, override removal and title-manifest "
                   "resolution, version validation, and the secure endpoint boundary without opening a connection.",
    .apis = {"CNA::GamerServices::Configuration", "CNA::GamerServices::Configuration::endpoint",
             "CNA::GamerServices::Configuration::gameId", "CNA::GamerServices::Configuration::caBundle",
             "CNA::GamerServices::Configuration::insecureLoopback",
             "CNA::GamerServices::Configuration::avatarCatalogUpdates",
             "CNA::GamerServices::Configuration::maxAvatarCatalogBytes",
             "CNA::GamerServices::Configuration::titleVersion",
             "CNA::GamerServices::setConfigurationOverride", "CNA::GamerServices::resolveConfiguration",
             "CNA::GamerServices::validateConfiguration"},
    .tags = {"gamer-services", "configuration", "manifest", "offline", "security", "cnaext"},
    .ownProcess = true,
})
